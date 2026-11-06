#include "coffea_converter.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


void CoffeaConverter::add_mapping(std::string source, std::string dest) {
    var_mappings.emplace(source, dest);
}

static bool is_string(std::string &in) {
    static const std::regex reg_string("\"[^\"]*\"");
    return (std::regex_match(in, reg_string));
}

static bool is_number(std::string &in) {
    static const std::regex reg_number(R"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[Ee][-+]?\d+)?)");
    return (std::regex_match(in, reg_number));
}

std::string CoffeaConverter::get_mapping(std::string in) {
    if (is_string(in) || is_number(in)) return in;
    
    std::string mapped = "";

    if (var_mappings.contains(in)) {
        mapped = var_mappings[in];
    }
    
    if (mapped == in) return in;
    if (var_mappings.contains(mapped)) return get_mapping(mapped);

    return mapped;
}

std::string CoffeaConverter::get_mapped_source(const AnalysisCommand &command, size_t pos) {
    std::string orig_source = command.get_source_argument(pos);

    // Remove special characters from variable names
    std::regex e("[_\\->]");

    std::string escaped_source;

    if (is_string(orig_source) || is_number(orig_source)) {
        escaped_source = orig_source;
    } else {
        escaped_source = std::regex_replace(orig_source, e, "");
    }

    std::string mapped_source = get_mapping(escaped_source);
    return mapped_source;
}

std::string CoffeaConverter::get_mapped_dest(const AnalysisCommand &command) {
    std::string orig_dest = command.get_dest_argument();
    
    std::regex e("[_\\->]");

    std::string escaped_dest;

    if (is_string(orig_dest) || is_number(orig_dest)) {
        escaped_dest = orig_dest;
    } else {
        escaped_dest = std::regex_replace(orig_dest, e, "");
    }

    return escaped_dest;
}

std::string CoffeaConverter::list_append(std::string list_end, std::string delimiter, const AnalysisCommand &command, std::string to_add) {
    std::string old_list = get_mapped_source(command, 0);

    std::regex sanitize{R"([-[\]{}()*+?.,\^$|#\s])"};
    std::string list_end_sanitized = std::regex_replace(list_end, sanitize, R"(\$&)");

    std::regex reg_list_end(list_end_sanitized);
    old_list = std::regex_replace(old_list, reg_list_end, "");

    std::stringstream add_val;
    add_val << old_list;
    add_val << (to_add == "" ? get_mapped_source(command, 1) : to_add);
    add_val << delimiter << list_end;
    return add_val.str();
}

std::string CoffeaConverter::strip_collection_marker(std::string in) {
    std::string result = in;
    // Remove trailing \x1d
    while (!result.empty() && result.back() == '\x1d') {
        result.pop_back();
    }
    return result;
}

std::string CoffeaConverter::attribute(std::string attr, std::string object, std::string separator_chars) {
    // Strip any trailing \x1d first
    std::string clean_object = strip_collection_marker(object);
    
    if (attr.empty()) {
        return clean_object;
    }
    
    // Check if there's an internal \x1d (attribute insertion point)
    std::stringstream delimit;
    delimit << object;
    std::vector<std::string> delimited_at_attributing_points;
    std::string buffer;
    while (std::getline(delimit, buffer, '\x1d')) {
        if (!buffer.empty()) {
            delimited_at_attributing_points.push_back(buffer);
        }
    }

    if (delimited_at_attributing_points.size() <= 1) {
        std::stringstream attributed_text;
        attributed_text << clean_object << separator_chars << attr;
        return attributed_text.str();
    }

    // Multiple segments: insert attribute at each join point
    std::stringstream attributed_text;
    bool is_first = true;
    for (const std::string &chunk : delimited_at_attributing_points) {
        if (is_first) {
            attributed_text << chunk;
            is_first = false;
        } else {
            attributed_text << separator_chars << attr << chunk;
        }
    }

    return attributed_text.str();
}

std::string CoffeaConverter::attribute(std::string attr, std::string object) {
    return attribute(attr, object, attribute_delimiter);
}

std::string CoffeaConverter::lorentzify(std::string object) {
    std::string stripped = strip_collection_marker(object);
    
    // Already known as a Lorentz vector
    if (known_lorentz.count(stripped) > 0) {
        return stripped;
    }
    
    // Already an ak.zip Lorentz vector expression — don't re-wrap
    if (stripped.find("ToLV") != std::string::npos) {
        return stripped;
    }
    
    // Build the ak.zip from components
    std::stringstream lv;
    lv << "ToLV("
       << "pt=" << synthetic_attribute(main_names->pt(), object) << ", "
       << "eta=" << synthetic_attribute(main_names->eta(), object) << ", "
       << "phi=" << synthetic_attribute(main_names->phi(), object) << ", "
       << "mass=" << synthetic_attribute(main_names->mass(), object)
       << ")";
    return lv.str();
}

std::string CoffeaConverter::multi_arg_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok, bool is_lorentz) {
    std::stringstream func;
    func << func_name << "(";
    bool is_first = true;
    for (int i = 0; i < num_args; i++) {
        if (is_first) { is_first = false; }
        else { func << ", "; }

        std::string this_arg = get_mapped_source(command, i);

        if (is_lorentz) {
            func << lorentzify(this_arg);
        } else {
            func << this_arg;
        }
    }

    func << ")" << ending_tok;
    return func.str();
}

std::string CoffeaConverter::multi_arg_lorentz_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok) {
    return multi_arg_function(func_name, num_args, command, ending_tok, true);
}

std::string CoffeaConverter::binary_infix_operation(std::string op_name, const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(" << get_mapped_source(command, 0);
    computation << " " << op_name << " ";
    computation << get_mapped_source(command, 1) << ")";
    return computation.str();
}

std::string CoffeaConverter::interval(std::string left_bound_op, std::string right_bound_op, const AnalysisCommand &command) {
    std::stringstream within;
    within << "(";
    within << "(" << get_mapped_source(command, 0) << " " << left_bound_op << " " << get_mapped_source(command, 1) << ")";
    within << " & ";
    within << "(" << get_mapped_source(command, 0) << " " << right_bound_op << " " << get_mapped_source(command, 2) << ")";
    within << ")";
    return within.str();
}


std::string CoffeaConverter::add_subtract_particles(const AnalysisCommand &command, bool is_subtraction) {
    std::string last_val = get_mapped_source(command, 0);  // accumulated value or ""
    std::string this_val = get_mapped_source(command, 1);   // new particle to add/subtract
    
    if (last_val.empty()) {
        // First particle in the chain — just return it (lorentzified when needed later)
        if (is_subtraction) {
            // Negate: will be used as (-lv) in next step
            return "(-(" + lorentzify(this_val) + "))";
        }
        return this_val;  // Don't lorentzify yet — let the final step do it
    }
    
    // We have two operands to combine
    std::string dest = get_mapped_dest(command);
    
    emit_newline();
    emit_comment("Particle ", is_subtraction ? "subtraction" : "addition", 
                 " by combining Lorentz vectors");
    
    // Store each operand in a temp variable to avoid nested lorentzify
    std::string lv_a_var = dest + "_lv_a";
    std::string lv_b_var = dest + "_lv_b";
    
    emit(lv_a_var, " = ", lorentzify(last_val));
    known_lorentz.insert(lv_a_var);
    
    emit(lv_b_var, " = ", lorentzify(this_val));
    known_lorentz.insert(lv_b_var);
    
    // Combine
    std::string op = is_subtraction ? " - " : " + ";
    emit(dest, "_lv = ", lv_a_var, op, lv_b_var);
    known_lorentz.insert(dest + "_lv");
    
    // Extract components
    emit_comment("Extract kinematic variables from combined Lorentz vector");
    emit(dest, "_pt = ", dest, "_lv.pt");
    emit(dest, "_eta = ", dest, "_lv.eta");
    emit(dest, "_phi = ", dest, "_lv.phi");
    emit(dest, "_mass = ", dest, "_lv.mass");
    
    // Charge — must come from original collection, NOT from Lorentz vector
    emit_comment("Add charges manually");
    emit(dest, "_charge = ", 
         synthetic_attribute(main_names->charge(), last_val), op,
         synthetic_attribute(main_names->charge(), this_val));
    
    synthetic_particle_vars[dest] = true;
    
    return dest;
}

std::string CoffeaConverter::synthetic_attribute(std::string attr, std::string object) {
    if (synthetic_particle_vars.contains(object)) {
        return object + "_" + attr;
    }
    return attribute(attr, object);
}

std::string CoffeaConverter::use_within_region(std::string fun_within_node, std::string extra_arg, const AnalysisCommand &command) {
    std::string reg_name;
    std::string reg_mapped;

    if (command.get_num_source_arguments() == 1) {
        reg_name = command.get_source_argument(0);
        reg_mapped = get_mapped_source(command, 0);
    } else {
        reg_name = command.get_source_argument(1);
        reg_mapped = get_mapped_source(command, 1);
    }

    emit_newline();
    emit_comment("Apply function within region context: ", reg_name);
    emit("_region_events = events[", reg_mapped, "_mask]");
    emit(fun_within_node, "(", extra_arg, ", _region_events)");

    return command.has_dest_argument() ? get_mapped_dest(command) : "";
}


void CoffeaConverter::track_region_cut(std::string region_base, std::string mask_var, std::string description) {
    region_cutflow_steps[region_base].push_back({mask_var, description});
}


std::string CoffeaConverter::get_region_base_name(std::string mapped_name) {
    std::regex e("^V[0-9]+REG");
    return std::regex_replace(mapped_name, e, "");
}


std::string CoffeaConverter::provenance_attr(std::string object) {
    std::string clean = strip_collection_marker(object);
    return clean + ".provenance";
}

// Conversion methods

std::string CoffeaConverter::convert_conversion_error(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "";
}

std::string CoffeaConverter::convert_create_empty_info_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{}";
}

std::string CoffeaConverter::convert_add_to_info_list(const AnalysisCommand &command) {
    std::stringstream info_pair;
    info_pair << "'" << command.get_source_argument(1) << "': '" << command.get_source_argument(2) << "'";
    return list_append("}", ", ", command, info_pair.str());
}

std::string CoffeaConverter::convert_display_info(const AnalysisCommand &command) {
    std::string info_list = get_mapped_source(command, 0);
    emit_newline();
    emit_comment("Display analysis info");
    emit("print(", info_list, ")");
    return "";
}

std::string CoffeaConverter::convert_create_region(const AnalysisCommand &command) {
    std::string dest = get_mapped_dest(command);

    emit_newline();
    emit_comment("Create new region: ", command.get_dest_argument());
    emit(get_mapped_dest(command), "_mask = ak.ones_like(events.event, dtype=bool)");
    emit(get_mapped_dest(command), "_weights = []");
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_merge_regions(const AnalysisCommand &command) {
    std::string dest = get_mapped_dest(command);
    std::string name_first = get_mapped_source(command, 0);
    std::string name_second = get_mapped_source(command, 1);
    std::string base_name = get_region_base_name(dest);

    emit_newline();
    emit_comment("Merge regions ", name_first, " and ", name_second);
    emit(get_mapped_dest(command), "_mask = ", name_first, "_mask & ", name_second, "_mask");
    emit(get_mapped_dest(command), "_weights = ", name_first, "_weights + ", name_second, "_weights");

    track_region_cut(base_name, dest + "_mask", "Merge(" + name_first + ", " + name_second + ")");

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_cut_region(const AnalysisCommand &command) {
    std::string dest = get_mapped_dest(command);
    std::string prev = get_mapped_source(command, 0);
    std::string cut = get_mapped_source(command, 1);
    std::string base_name = get_region_base_name(dest);

    emit_newline();
    emit_comment("Apply cut to region");
    emit(get_mapped_dest(command), "_mask = ", prev, "_mask & (", cut, ")");
    emit(get_mapped_dest(command), "_weights = ", prev, "_weights.copy()");
    
    track_region_cut(base_name, dest + "_mask", cut);

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_create_bin_of_region(const AnalysisCommand &command) {
    std::string dest = get_mapped_dest(command);
    std::string prev = get_mapped_source(command, 0);
    std::string cut = get_mapped_source(command, 1);
    std::string base_name = get_region_base_name(dest);

    emit_newline();
    emit_comment("Create bin of region");
    emit(get_mapped_dest(command), "_mask = ", prev, "_mask & (", cut, ")");
    emit(get_mapped_dest(command), "_weights = ", prev, "_weights.copy()");
    
    track_region_cut(base_name, dest + "_mask", "Bin: " + cut);

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_add_alias(const AnalysisCommand &command) {
    return command.get_source_argument(0);
}

std::string CoffeaConverter::convert_add_external(const AnalysisCommand &command) {
    std::regex reg_quote("\"");
    std::string extern_final_name = std::regex_replace(command.get_source_argument(0), reg_quote, "");
    is_attribute.emplace(extern_final_name);
    return extern_final_name;
}

std::string CoffeaConverter::convert_add_extern_attr(const AnalysisCommand &command) {
    std::regex reg_quote("\"");
    std::string attr_final_name = std::regex_replace(command.get_source_argument(0), reg_quote, "");
    is_attribute.emplace(attr_final_name);
    return attr_final_name;
}

std::string CoffeaConverter::convert_add_extern_particle(const AnalysisCommand &command) {
    std::regex reg_quote("\"");
    std::string particle_name = std::regex_replace(command.get_source_argument(0), reg_quote, "");
    
    std::stringstream extern_part;
    extern_part << "events." << particle_name;

    emit("events = ak.with_field(");
    emit("    events,");
    emit("    ak.with_field(");
    emit("        events.", particle_name, ",");
    emit("        try_create_provenance(events.", particle_name, ", \"", main_names->pt(), "\"),");
    emit("        \"provenance\"");
    emit("    ),");
    emit("    \"", particle_name, "\"");
    emit(")");
    
    // Always register in provenance_map — downstream code checks for None at runtime
    provenance_map[extern_part.str()] = extern_part.str() + ".provenance";
    provenance_map[extern_part.str() + "\x1d"] = extern_part.str() + ".provenance";

    extern_part << '\x1d';
    return extern_part.str();
}

std::string CoffeaConverter::convert_add_correctionlib(const AnalysisCommand &command) {
    std::string filename_with_quotes = command.get_source_argument(0);
    std::string keyname_with_quotes = command.get_source_argument(1);
    
    emit_newline();
    emit_comment("Load correctionlib correction");
    emit("import correctionlib");
    emit(get_mapped_dest(command), "_cset = correctionlib.CorrectionSet.from_file(", filename_with_quotes, ")");
    emit(get_mapped_dest(command), " = ", get_mapped_dest(command), "_cset[", keyname_with_quotes, "]");
    
    return get_mapped_dest(command) + ".evaluate";
}

std::string CoffeaConverter::convert_create_mask(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Create selection mask ", command.get_dest_argument(), " from shape of ", command.get_source_argument(0));
    
    std::string shape_of_out = attribute(main_names->pt(), get_mapped_source(command, 0));
    
    emit(get_mapped_dest(command), " = ak.ones_like(", shape_of_out, ", dtype=bool)");

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_limit_mask(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Apply limit to mask");
    emit(get_mapped_dest(command), " = ", get_mapped_source(command, 0), " & (", get_mapped_source(command, 1), ")");

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_apply_mask(const AnalysisCommand &command) {
    std::string last_mask = get_mapped_source(command, 0);
    std::string source_name = get_mapped_source(command, 1);
    std::string clean_source = strip_collection_marker(source_name);
    std::string dest = get_mapped_dest(command);

    emit_newline();
    emit_comment("Create object ", command.get_dest_argument(), " from mask");

    if (synthetic_particle_vars.contains(clean_source)) {
        emit(dest, " = ak.zip({");
        emit("    'pt': ", clean_source, "_pt[", last_mask, "],");
        emit("    'eta': ", clean_source, "_eta[", last_mask, "],");
        emit("    'phi': ", clean_source, "_phi[", last_mask, "],");
        emit("    'mass': ", clean_source, "_mass[", last_mask, "],");
        emit("    'charge': ", clean_source, "_charge[", last_mask, "]");
        emit("})");
    } else {
        emit(dest, " = ", clean_source, "[", last_mask, "]");
    }

    // Embedded provenance automatically follows ordinary slicing
    std::string source_prov = find_provenance(source_name);
    if (!source_prov.empty()) {
        provenance_map[dest] = dest + ".provenance";
        provenance_map[dest + "\x1d"] = dest + ".provenance";
    }

    return dest + "\x1d";
}

std::string CoffeaConverter::convert_create_empty_hist_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "[]";
}

std::string CoffeaConverter::convert_add_hist_to_list(const AnalysisCommand &command) {
    return list_append("]", ", ", command);
}

std::string CoffeaConverter::convert_use_hist(const AnalysisCommand &command) {
    emit_comment("Use histogram in region");
    return use_within_region("fill_histogram", get_mapped_source(command, 0), command);
}

std::string CoffeaConverter::convert_use_hist_list(const AnalysisCommand &command) {
    emit_comment("Use histogram list in region");
    return use_within_region("fill_histogram_list", get_mapped_source(command, 0), command);
}

std::string CoffeaConverter::convert_hist_1d(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Define 1D histogram: ", command.get_dest_argument());
    
    std::string title = get_mapped_source(command, 0);
    std::string nbins = get_mapped_source(command, 1);
    std::string low = get_mapped_source(command, 2);
    std::string high = get_mapped_source(command, 3);
    std::string fill_var = get_mapped_source(command, 4);
    
    // Clean up title for axis label
    std::regex e1(",\"");
    std::regex e2("\\[\"");
    title = std::regex_replace(title, e1, ", r\"");
    title = std::regex_replace(title, e2, "[r\"");
    
    emit(get_mapped_dest(command), " = {");
    emit("    'name': '", get_mapped_dest(command), "',");
    emit("    'hist': hist.Hist(hist.axis.Regular(", nbins, ", ", low, ", ", high, ", name='x', label=", title, ")),");
    emit("    'fill_expr': lambda events: ", fill_var);
    emit("}");
    
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_hist_2d(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Define 2D histogram: ", command.get_dest_argument());
    
    std::string title = get_mapped_source(command, 0);
    std::string nbins_x = get_mapped_source(command, 1);
    std::string low_x = get_mapped_source(command, 2);
    std::string high_x = get_mapped_source(command, 3);
    std::string fill_var_x = get_mapped_source(command, 4);
    std::string nbins_y = get_mapped_source(command, 5);
    std::string low_y = get_mapped_source(command, 6);
    std::string high_y = get_mapped_source(command, 7);
    std::string fill_var_y = get_mapped_source(command, 8);
    
    std::regex e1(",\"");
    std::regex e2("\\[\"");
    title = std::regex_replace(title, e1, ", r\"");
    title = std::regex_replace(title, e2, "[r\"");
    
    emit(get_mapped_dest(command), " = {");
    emit("    'name': '", get_mapped_dest(command), "',");
    emit("    'hist': hist.Hist(");
    emit("        hist.axis.Regular(", nbins_x, ", ", low_x, ", ", high_x, ", name='x'),");
    emit("        hist.axis.Regular(", nbins_y, ", ", low_y, ", ", high_y, ", name='y')");
    emit("    ),");
    emit("    'fill_expr_x': lambda events: ", fill_var_x, ",");
    emit("    'fill_expr_y': lambda events: ", fill_var_y);
    emit("}");
    
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_weight_apply(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command, 0);
    std::string weight = get_mapped_source(command, 1);
    std::string dest = get_mapped_dest(command);
    std::string base_name = get_region_base_name(dest);

    emit_newline();
    emit_comment("Apply weight correction: ", weight);
    emit(dest, "_mask = ", prev, "_mask");
    emit(dest, "_weights = ", prev, "_weights + ['", weight, "']");

    track_region_cut(base_name, dest + "_mask", "Weight: " + weight);
    
    return dest;
}

std::string CoffeaConverter::convert_do_cutflow_on_region(const AnalysisCommand &command) {
    std::string reg_name = get_mapped_source(command, 0);
    std::string base_name = get_region_base_name(reg_name);

    emit_newline();
    emit_comment("Cutflow for region: " + base_name);

    // Initial count before any cuts
    emit("output[\"cutflow_", base_name, "::Initial\"] += float(len(events))");

    if (region_cutflow_steps.contains(base_name)) {
        const auto &steps = region_cutflow_steps[base_name];
        for (const auto &step : steps) {
            std::string safe;
            for (char c : step.description) {
                if (c == '\\') safe += "\\\\";
                else if (c == '"') safe += "\\\"";
                else safe += c;
            }
            emit("output[\"cutflow_", base_name, "::", safe, "\"] += float(ak.sum(", step.mask_var, "))");
        }
    }

    return "";
}

std::string CoffeaConverter::convert_do_eventlist_on_region(const AnalysisCommand &command) {
    std::string reg_name = get_mapped_source(command, 0);

    emit_newline();
    emit_comment("Generate event list for region: ", reg_name);
    emit("_region_events = events[", reg_name, "_mask]");
    emit("print('Event list for region ", reg_name, "')");
    emit("print('run, luminosityBlock, event')");
    emit("for _i in range(min(1000, len(_region_events))):");
    emit("    print(f'{_region_events.run[_i]}, {_region_events.luminosityBlock[_i]}, {_region_events.event[_i]}')");
    
    return "";
}

std::string CoffeaConverter::convert_create_table(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "[]";
}

std::string CoffeaConverter::convert_create_table_errored_value(const AnalysisCommand &command) {
    std::stringstream errored_val;
    errored_val << "(" << get_mapped_source(command, 0) << ", " << get_mapped_source(command, 1) << ", " << get_mapped_source(command, 2) << ")";
    return errored_val.str();
}

std::string CoffeaConverter::convert_create_table_value(const AnalysisCommand &command) {
    return get_mapped_source(command, 0);
}

std::string CoffeaConverter::convert_append_to_table(const AnalysisCommand &command) {
    std::string last_table = get_mapped_source(command, 0);
    std::string value = get_mapped_source(command, 1);
    std::string lower_bounds = get_mapped_source(command, 2);
    std::string upper_bounds = get_mapped_source(command, 3);

    std::regex open("\\{");
    std::regex close("\\}");
    std::string lower_bounds_brackets = std::regex_replace(std::regex_replace(lower_bounds, open, "("), close, ")");
    std::string upper_bounds_brackets = std::regex_replace(std::regex_replace(upper_bounds, open, "("), close, ")");

    std::regex end_table("\\s+\\]");
    std::string unended_table = std::regex_replace(last_table, end_table, "");

    std::stringstream new_table;
    new_table << unended_table << "\n    (" << value << ", " << lower_bounds_brackets << ", " << upper_bounds_brackets << "),  ]";
    return new_table.str();
}

std::string CoffeaConverter::convert_finish_table(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Creating a multi-argument function ", command.get_dest_argument(), " out of a table");
    emit(get_mapped_dest(command), " = create_function_from_table(", get_mapped_source(command, 0), ")");
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_obj_sort_ascend(const AnalysisCommand &command) {
    std::string source_obj = get_mapped_source(command, 0);
    std::string sort_key = get_mapped_source(command, 1);
    std::string clean_source = strip_collection_marker(source_obj);
    std::string dest = get_mapped_dest(command);

    emit_newline();
    emit_comment("Sort collection in ascending order");
    emit(dest, "_sort_idx = ak.argsort(", sort_key, ", ascending=True)");
    emit(dest, " = ", clean_source, "[", dest, "_sort_idx]");

    std::string source_prov = find_provenance(source_obj);
    if (!source_prov.empty()) {
        provenance_map[dest] = dest + ".provenance";
        provenance_map[dest + "\x1d"] = dest + ".provenance";
    }

    return dest + "\x1d";
}

std::string CoffeaConverter::convert_obj_sort_descend(const AnalysisCommand &command) {
    std::string source_obj = get_mapped_source(command, 0);
    std::string sort_key = get_mapped_source(command, 1);
    std::string clean_source = strip_collection_marker(source_obj);
    std::string dest = get_mapped_dest(command);

    emit_newline();
    emit_comment("Sort collection in descending order");
    emit(dest, "_sort_idx = ak.argsort(", sort_key, ", ascending=False)");
    emit(dest, " = ", clean_source, "[", dest, "_sort_idx]");

    std::string source_prov = find_provenance(source_obj);
    if (!source_prov.empty()) {
        provenance_map[dest] = dest + ".provenance";
        provenance_map[dest + "\x1d"] = dest + ".provenance";
    }

    return dest + "\x1d";
}

std::string CoffeaConverter::convert_expr_raise(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(" << get_mapped_source(command, 0) << " ** " << get_mapped_source(command, 1) << ")";
    return computation.str();
}

std::string CoffeaConverter::convert_expr_multiply(const AnalysisCommand &command) {
    return binary_infix_operation("*", command);
}

std::string CoffeaConverter::convert_expr_divide(const AnalysisCommand &command) {
    return binary_infix_operation("/", command);
}

std::string CoffeaConverter::convert_expr_add(const AnalysisCommand &command) {
    return binary_infix_operation("+", command);
}

std::string CoffeaConverter::convert_expr_subtract(const AnalysisCommand &command) {
    return binary_infix_operation("-", command);
}

std::string CoffeaConverter::convert_expr_lt(const AnalysisCommand &command) {
    return binary_infix_operation("<", command);
}

std::string CoffeaConverter::convert_expr_le(const AnalysisCommand &command) {
    return binary_infix_operation("<=", command);
}

std::string CoffeaConverter::convert_expr_gt(const AnalysisCommand &command) {
    return binary_infix_operation(">", command);
}

std::string CoffeaConverter::convert_expr_ge(const AnalysisCommand &command) {
    return binary_infix_operation(">=", command);
}

std::string CoffeaConverter::convert_expr_eq(const AnalysisCommand &command) {
    return binary_infix_operation("==", command);
}

std::string CoffeaConverter::convert_expr_ne(const AnalysisCommand &command) {
    return binary_infix_operation("!=", command);
}

std::string CoffeaConverter::convert_expr_bitwise_and(const AnalysisCommand &command) {
    return binary_infix_operation("&", command);
}

std::string CoffeaConverter::convert_expr_bitwise_or(const AnalysisCommand &command) {
    return binary_infix_operation("|", command);
}

std::string CoffeaConverter::convert_expr_and(const AnalysisCommand &command) {
    return binary_infix_operation("&", command); 
}

std::string CoffeaConverter::convert_expr_or(const AnalysisCommand &command) {
    return binary_infix_operation("|", command); 
}

std::string CoffeaConverter::convert_expr_within(const AnalysisCommand &command) {
    return interval(">=", "<=", command);
}

std::string CoffeaConverter::convert_expr_within_exclusive(const AnalysisCommand &command) {
    return interval(">", "<", command);
}

std::string CoffeaConverter::convert_expr_within_left_exclusive(const AnalysisCommand &command) {
    return interval(">", "<=", command);
}

std::string CoffeaConverter::convert_expr_within_right_exclusive(const AnalysisCommand &command) {
    return interval(">=", "<", command);
}

std::string CoffeaConverter::convert_expr_negate(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(-(" << get_mapped_source(command, 0) << "))";
    return computation.str();
}

std::string CoffeaConverter::convert_expr_logical_not(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(~(" << get_mapped_source(command, 0) << "))";
    return computation.str();
}

std::string CoffeaConverter::convert_expr_if_ternary(const AnalysisCommand &command) {
    std::stringstream ternary;
    ternary << "ak.where(" << get_mapped_source(command, 0) << ", ";
    ternary << get_mapped_source(command, 1) << ", ";
    ternary << get_mapped_source(command, 2) << ")";
    return ternary.str();
}

std::string CoffeaConverter::convert_expr_index(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    std::string idx = command.get_source_argument(1);

    std::string stripped = strip_collection_marker(src);
    std::stringstream expr;
    expr << "ak.pad_none(" << stripped << ", " << idx << "+1, axis=1)[:, " << idx << "]";

    return expr.str();
}

std::string CoffeaConverter::convert_expr_index_range(const AnalysisCommand &command) {
    std::string src = strip_collection_marker(get_mapped_source(command, 0));
    std::string from = get_mapped_source(command, 1);
    std::string to = get_mapped_source(command, 2);

    std::stringstream index;
    index << "ak.pad_none(" << src << ", " << to << ", axis=1)[:, " << from << ":" << to << "]";
    return index.str();
}

std::string CoffeaConverter::convert_expr_index_until(const AnalysisCommand &command) {
    std::string src = strip_collection_marker(get_mapped_source(command, 0));
    std::string to = get_mapped_source(command, 1);

    std::stringstream index;
    index << "ak.pad_none(" << src << ", " << to << ", axis=1)[:, :" << to << "]";
    return index.str();
}

std::string CoffeaConverter::convert_expr_index_from(const AnalysisCommand &command) {
    std::string src = strip_collection_marker(get_mapped_source(command, 0));
    std::string from = get_mapped_source(command, 1);

    std::stringstream index;
    index << "ak.pad_none(" << src << ", " << from << ", axis=1)[:, " << from << ":]";
    return index.str();
}

std::string CoffeaConverter::convert_func_charge(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    return synthetic_attribute(main_names->charge(), src);
}

std::string CoffeaConverter::convert_func_pt(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    return synthetic_attribute(main_names->pt(), src);
}

std::string CoffeaConverter::convert_func_eta(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    return synthetic_attribute(main_names->eta(), src);
}

std::string CoffeaConverter::convert_func_phi(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    return synthetic_attribute(main_names->phi(), src);
}

std::string CoffeaConverter::convert_func_mass(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    return synthetic_attribute(main_names->mass(), src);
}

std::string CoffeaConverter::convert_func_energy(const AnalysisCommand &command) {
    std::stringstream energy;
    energy << lorentzify(get_mapped_source(command, 0)) << ".energy";
    return energy.str();
}

std::string CoffeaConverter::convert_func_distinct(const AnalysisCommand &command) {
    std::string src0 = get_mapped_source(command, 0);
    std::string src1 = get_mapped_source(command, 1);
    
    std::string prov0 = find_provenance(src0);
    std::string prov1 = find_provenance(src1);
    
    std::stringstream distinct;
    
    if (!prov0.empty() && !prov1.empty()) {
        // Both have provenance entries — but they might be None at runtime
        // Emit a helper call that checks at runtime
        emit_newline();
        emit_comment("Distinctness check with runtime provenance fallback");
        
        std::string distinct_var = get_mapped_dest(command) + "_distinct";
        
        emit(distinct_var, " = check_distinct(",
             prov0, ", ", prov1, ", ",
             synthetic_attribute(main_names->pt(), src0), ", ",
             synthetic_attribute(main_names->pt(), src1), ", ",
             synthetic_attribute(main_names->eta(), src0), ", ",
             synthetic_attribute(main_names->eta(), src1), ", ",
             synthetic_attribute(main_names->phi(), src0), ", ",
             synthetic_attribute(main_names->phi(), src1), ")");
        
        distinct << distinct_var;
    } else {
        // No provenance at all — pure kinematic fallback
        distinct << "((" 
                 << synthetic_attribute(main_names->pt(), src0) << " != " 
                 << synthetic_attribute(main_names->pt(), src1) << ") | ("
                 << synthetic_attribute(main_names->eta(), src0) << " != " 
                 << synthetic_attribute(main_names->eta(), src1) << ") | ("
                 << synthetic_attribute(main_names->phi(), src0) << " != " 
                 << synthetic_attribute(main_names->phi(), src1) << "))";
    }
    
    return distinct.str();
}

std::string CoffeaConverter::convert_func_dr(const AnalysisCommand &command) {
    std::stringstream dr;
    dr << "delta_r_cross(" << lorentzify(get_mapped_source(command, 0)) << ", " << lorentzify(get_mapped_source(command, 1)) << ")";
    return dr.str();
}

std::string CoffeaConverter::convert_func_dphi(const AnalysisCommand &command) {
    std::stringstream dphi;
    dphi << "delta_phi_cross(" <<lorentzify(get_mapped_source(command, 0)) << ", " << lorentzify(get_mapped_source(command, 1)) << ")";
    return dphi.str();
}


std::string CoffeaConverter::convert_func_deta(const AnalysisCommand &command) {
    std::string src0 = get_mapped_source(command, 0);
    std::string src1 = get_mapped_source(command, 1);
    
    std::stringstream deta;
    deta << "(" << synthetic_attribute(main_names->eta(), src0);
    deta << " - " << synthetic_attribute(main_names->eta(), src1) << ")";
    return deta.str();
}

std::string CoffeaConverter::convert_func_dr_hadamard(const AnalysisCommand &command) {
    // for Hadamard (element-wise) operations on jagged arrays
    std::stringstream dr;
    dr  << lorentzify(get_mapped_source(command, 0)) << ".delta_r(" << lorentzify(get_mapped_source(command, 1)) << ")";
    return dr.str();
}

std::string CoffeaConverter::convert_func_dphi_hadamard(const AnalysisCommand &command) {
    std::stringstream dphi;
    dphi << lorentzify(get_mapped_source(command, 0)) << ".delta_phi(" << lorentzify(get_mapped_source(command, 1)) << ")";
    return dphi.str();
}

std::string CoffeaConverter::convert_func_deta_hadamard(const AnalysisCommand &command) {
    std::string src0 = get_mapped_source(command, 0);
    std::string src1 = get_mapped_source(command, 1);
    
    std::stringstream deta;
    deta << "(" << synthetic_attribute(main_names->eta(), src0);
    deta << " - " << synthetic_attribute(main_names->eta(), src1) << ")";
    return deta.str();
}

std::string CoffeaConverter::convert_func_size(const AnalysisCommand &command) {
    std::string src = get_mapped_source(command, 0);
    std::string momentum_of_part = synthetic_attribute(main_names->pt(), src);
    std::stringstream computation;
    computation << "ak.num(" << momentum_of_part << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_anyof(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.any(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_allof(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.all(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_sqrt(const AnalysisCommand &command) {
    return multi_arg_function("np.sqrt", 1, command);
}

std::string CoffeaConverter::convert_func_abs(const AnalysisCommand &command) {
    return multi_arg_function("np.abs", 1, command);
}

std::string CoffeaConverter::convert_func_cos(const AnalysisCommand &command) {
    return multi_arg_function("np.cos", 1, command);
}

std::string CoffeaConverter::convert_func_sin(const AnalysisCommand &command) {
    return multi_arg_function("np.sin", 1, command);
}

std::string CoffeaConverter::convert_func_tan(const AnalysisCommand &command) {
    return multi_arg_function("np.tan", 1, command);
}

std::string CoffeaConverter::convert_func_sinh(const AnalysisCommand &command) {
    return multi_arg_function("np.sinh", 1, command);
}

std::string CoffeaConverter::convert_func_cosh(const AnalysisCommand &command) {
    return multi_arg_function("np.cosh", 1, command);
}

std::string CoffeaConverter::convert_func_tanh(const AnalysisCommand &command) {
    return multi_arg_function("np.tanh", 1, command);
}

std::string CoffeaConverter::convert_func_exp(const AnalysisCommand &command) {
    return multi_arg_function("np.exp", 1, command);
}

std::string CoffeaConverter::convert_func_log(const AnalysisCommand &command) {
    return multi_arg_function("np.log", 1, command);
}

std::string CoffeaConverter::convert_func_ave(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.mean(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_sum(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.sum(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_min_of_pair(const AnalysisCommand &command) {
    return multi_arg_function("np.minimum", 2, command);
}

std::string CoffeaConverter::convert_func_max_of_pair(const AnalysisCommand &command) {
    return multi_arg_function("np.maximum", 2, command);
}

std::string CoffeaConverter::convert_func_min_of_list(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.min(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_max_of_list(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.max(" << get_mapped_source(command, 0) << ", axis=-1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_sort_ascend(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.sort(" << get_mapped_source(command, 0) << ", ascending=True)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_sort_descend(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.sort(" << get_mapped_source(command, 0) << ", ascending=False)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_named(const AnalysisCommand &command) {
    std::string name = get_mapped_source(command, 1);
    if (is_attribute.contains(name)) {
        return attribute(name, get_mapped_source(command, 0));
    } else {
        return multi_arg_function(name, 1, command);
    }
}

std::string CoffeaConverter::convert_create_empty_string_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "[]";
}

std::string CoffeaConverter::convert_add_string_to_list(const AnalysisCommand &command) {
    return list_append("]", ", ", command);
}

std::string CoffeaConverter::convert_create_empty_value_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{}";
}

std::string CoffeaConverter::convert_add_value_to_list(const AnalysisCommand &command) {
    return list_append("}", ", ", command);
}

std::string CoffeaConverter::convert_create_empty_union(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "";
}

std::string CoffeaConverter::convert_add_part_to_union(const AnalysisCommand &command) {
    std::string prev_union = get_mapped_source(command, 0);
    std::string new_to_add = get_mapped_source(command, 1);

    std::string clean_prev = strip_collection_marker(prev_union);
    std::string clean_new = strip_collection_marker(new_to_add);

    if (clean_prev == "") {
        // First collection in the union — just return it
        // Its provenance carries over naturally
        return new_to_add;
    } else {
        std::string dest = get_mapped_dest(command);
        
        emit_newline();
        emit_comment("Create object ", command.get_dest_argument(), " from a union");
        emit(dest, " = ak.concatenate([", clean_prev, ", ", clean_new, "], axis=1)");

        // Concatenate provenance arrays too
        std::string prov_prev = find_provenance(prev_union);
        std::string prov_new = find_provenance(new_to_add);

        if (!prov_prev.empty() && !prov_new.empty()) {
            emit(dest, " = ak.with_field(");
            emit("    ", dest, ",");
            emit("    concat_provenance(", prov_prev, ", ", prov_new, "),");
            emit("    \"provenance\"");
            emit(")");

            provenance_map[dest] = dest + ".provenance";
            provenance_map[dest + "\x1d"] = dest + ".provenance";

        } else if (!prov_prev.empty()) {
            emit(dest, " = ak.with_field(");
            emit("    ", dest, ",");
            emit("    ak.concatenate([");
            emit("        ", prov_prev, ",");
            emit("        ak.local_index(", synthetic_attribute(main_names->pt(), new_to_add),
                ", axis=1) + ak.max(", prov_prev, ", axis=1) + 1");
            emit("    ], axis=1),");
            emit("    \"provenance\"");
            emit(")");

            provenance_map[dest] = dest + ".provenance";
            provenance_map[dest + "\x1d"] = dest + ".provenance";

        } else if (!prov_new.empty()) {
            emit(dest, " = ak.with_field(");
            emit("    ", dest, ",");
            emit("    ak.concatenate([");
            emit("        ak.local_index(", synthetic_attribute(main_names->pt(), prev_union), ", axis=1),");
            emit("        ", prov_new, " + ak.num(", synthetic_attribute(main_names->pt(), prev_union), ", axis=1)");
            emit("    ], axis=1),");
            emit("    \"provenance\"");
            emit(")");

            provenance_map[dest] = dest + ".provenance";
            provenance_map[dest + "\x1d"] = dest + ".provenance";
        }

        return dest + "\x1d";
    }
}

std::string CoffeaConverter::convert_create_empty_cartesian(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    // Store as a unique composite ID; actual combination computed when naming elements
    std::string dest = get_mapped_dest(command);
    composite_info[dest] = {"cartesian", {}};
    return dest;
}

std::string CoffeaConverter::convert_create_empty_disjoint(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    std::string dest = get_mapped_dest(command);
    composite_info[dest] = {"combinations", {}};
    return dest;
}

std::string CoffeaConverter::convert_create_empty_direct(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    std::string dest = get_mapped_dest(command);
    composite_info[dest] = {"direct", {}};
    return dest;
}

std::string CoffeaConverter::convert_add_part_to_composite(const AnalysisCommand &command) {
    std::string prev_composite = get_mapped_source(command, 0);
    std::string new_collection = get_mapped_source(command, 1);

    std::string clean_composite = strip_collection_marker(prev_composite);
    std::string clean_collection = strip_collection_marker(new_collection);

    if (composite_info.contains(clean_composite)) {
        composite_info[clean_composite].collections.push_back(clean_collection);
        return clean_composite;
    }

    std::string dest = get_mapped_dest(command);
    composite_info[dest] = {"cartesian", {clean_collection}};
    return dest;
}

std::string CoffeaConverter::convert_name_element_of_composite(const AnalysisCommand &command) {
    std::string dest = get_mapped_dest(command);

    std::string comb_name = get_mapped_source(command, 0);
    std::string comb_index_str = get_mapped_source(command, 1);
    std::string source_name = get_mapped_source(command, 2);
    std::string clean_source = strip_collection_marker(source_name);
    std::string clean_comb = strip_collection_marker(comb_name);
    
    emit_newline();
    
    // Check if we've already computed the combination indices
    if (!composite_indices_computed.contains(clean_comb)) {
        composite_indices_computed.emplace(clean_comb);
        
        if (composite_info.contains(clean_comb)) {
            auto &info = composite_info[clean_comb];
            
            if (info.type == "cartesian") {
                emit_comment("Compute Cartesian product indices for composite ", clean_comb);
                if (info.collections.size() == 2) {
                    emit(clean_comb, "_idx = ak.argcartesian([", 
                         info.collections[0], ", ", info.collections[1], "], axis=1)");
                    emit(clean_comb, "_idx0, ", clean_comb, "_idx1 = ak.unzip(", clean_comb, "_idx)");
                } else if (info.collections.size() == 3) {
                    emit(clean_comb, "_idx = ak.argcartesian([",
                         info.collections[0], ", ", info.collections[1], ", ", info.collections[2], "], axis=1)");
                    emit(clean_comb, "_idx0, ", clean_comb, "_idx1, ", clean_comb, "_idx2 = ak.unzip(", clean_comb, "_idx)");
                } else if (info.collections.size() == 4) {
                    emit(clean_comb, "_idx = ak.argcartesian([",
                         info.collections[0], ", ", info.collections[1], ", ", 
                         info.collections[2], ", ", info.collections[3], "], axis=1)");
                    emit(clean_comb, "_idx0, ", clean_comb, "_idx1, ", clean_comb, "_idx2, ", clean_comb, "_idx3 = ak.unzip(", clean_comb, "_idx)");
                } else {
                    // General case
                    std::stringstream collections_list;
                    collections_list << "[";
                    for (size_t i = 0; i < info.collections.size(); i++) {
                        if (i > 0) collections_list << ", ";
                        collections_list << info.collections[i];
                    }
                    collections_list << "]";
                    emit(clean_comb, "_idx = ak.argcartesian(", collections_list.str(), ", axis=1)");
                    
                    std::stringstream unzip_lhs;
                    for (size_t i = 0; i < info.collections.size(); i++) {
                        if (i > 0) unzip_lhs << ", ";
                        unzip_lhs << clean_comb << "_idx" << i;
                    }
                    emit(unzip_lhs.str(), " = ak.unzip(", clean_comb, "_idx)");
                }
            } else if (info.type == "combinations") {
                // Disjoint = combinations (no repetition)
                if (info.collections.size() >= 2) {
                    emit_comment("Compute combinations indices for composite ", clean_comb);
                    // For disjoint, all collections should be the same; use argcombinations
                    emit(clean_comb, "_idx = ak.argcombinations(", info.collections[0], 
                         ", ", std::to_string(info.collections.size()), ", axis=1)");
                    
                    std::stringstream unzip_lhs;
                    for (size_t i = 0; i < info.collections.size(); i++) {
                        if (i > 0) unzip_lhs << ", ";
                        unzip_lhs << clean_comb << "_idx" << i;
                    }
                    emit(unzip_lhs.str(), " = ak.unzip(", clean_comb, "_idx)");
                }
            } else if (info.type == "direct") {
                emit_comment("Direct combination for composite ", clean_comb, " (element-wise pairing)");
                for (size_t i = 0; i < info.collections.size(); i++) {
                    emit(clean_comb, "_idx", std::to_string(i), 
                         " = ak.local_index(", info.collections[i], ", axis=1)");
                }
            }
        }
    }
    
    emit_comment("Create object ", command.get_dest_argument(), " as element ", comb_index_str, " of composite ", clean_comb);
    emit(dest, " = ", clean_source, "[", clean_comb, "_idx", comb_index_str, "]");
    
    // Propagate provenance: index into the source's provenance with the same index array
    std::string source_prov = find_provenance(source_name);
    if (!source_prov.empty()) {
        provenance_map[dest] = dest + ".provenance";
        provenance_map[dest + "\x1d"] = dest + ".provenance";
    }

    return get_mapped_dest(command) + "\x1d";

}

std::string CoffeaConverter::convert_create_empty_particle(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "";
}

std::string CoffeaConverter::convert_add_particle(const AnalysisCommand &command) {
    return add_subtract_particles(command);
}

std::string CoffeaConverter::convert_sub_particle(const AnalysisCommand &command) {
    return add_subtract_particles(command, true);
}

std::string CoffeaConverter::find_provenance(std::string object) {
    std::string clean = strip_collection_marker(object);
    
    // Check exact match
    if (provenance_map.contains(object)) {
        return provenance_map[object];
    }
    // Check clean name
    if (provenance_map.contains(clean)) {
        return provenance_map[clean];
    }
    // Check with marker
    if (provenance_map.contains(clean + "\x1d")) {
        return provenance_map[clean + "\x1d"];
    }
    
    return "";
}

void CoffeaConverter::handle_command(const AnalysisCommand &command) {
    std::string resultant = command_convert(command);
    if (command.has_dest_argument()) {
        add_mapping(command.get_dest_argument(), resultant);
        add_mapping(get_mapped_dest(command), resultant);
    }
}

void CoffeaConverter::print() {

    level_of_indent = 0;

    met_name = config.get_argument("MET");
    std::string in_file = config.get_argument("infile");
    std::string out_file = config.get_argument("outfile");
    std::string format = config.get_argument("format");

    std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    attribute_delimiter = ".";

    std::string events_tree_name;

    if (format == "NANOAOD") {
        events_tree_name = "Events";
        main_names = FourVectorNames2("pt", "eta", "phi", "mass", "charge");
        met_names = main_names;
    } else if (format == "DELPHES") {
        events_tree_name = "Delphes";
        main_names = FourVectorNames2("PT", "Eta", "Phi", "Mass", "Charge");
        met_names = FourVectorNames2("MET", "Eta", "Phi", "fBits", "fBits");
    } else {
        events_tree_name = "Events";
        main_names = FourVectorNames2("pt", "eta", "phi", "mass", "charge");
        met_names = main_names;
    }

    // Get the path for helper functions
    std::filesystem::path abs_path = std::filesystem::absolute(ROOT_DIR);
    std::filesystem::path path_to_helper_py = abs_path / "helpers";

    // Import statements
    emit("import coffea");
    emit("from coffea import processor"); 
    emit("from coffea.nanoevents import NanoEventsFactory, NanoAODSchema");
    emit("import awkward as ak");
    emit("import numpy as np");
    emit("import hist");
    emit("import vector");
    emit("vector.register_awkward()");
    emit_newline();
    emit("import sys, os");
    
    // Add path to helper functions
    emit("adl_help_dir = os.path.abspath('", path_to_helper_py.string(), "')");
    emit("if adl_help_dir not in sys.path:");
    emit("    sys.path.append(adl_help_dir)");
    
    emit("from adl_helpers_coffea import fill_histogram, fill_histogram_list, create_function_from_table, try_create_provenance, concat_provenance, check_distinct, delta_r_cross, delta_phi_cross, cutflow_generate, print_all_cutflows, combine_outputs, ToLV");
    
    emit_newline();
    if (config.get_argument("cutflowout") == "file") {
        emit("output_file_name = '", out_file, "'.replace('.root', '')");
    }
    emit_newline();

    emit_newline();
    emit_comment("Load events from input file");

    emit("files = []");
    emit("if '", in_file, "'[-5:] == '.root':");
    emit("    files = ['", in_file, "',]");
    emit("else:");
    emit("    with open('", in_file, "', 'r') as in_file_text:");
    emit("        for line in in_file_text:");
    emit("            files.append(line.strip())");
    // emit("events = NanoEventsFactory.from_root('", in_file, "', schemaclass=NanoAODSchema, treepath='", events_tree_name, "').events()");
    
    emit("fileset = {'mainset':files}");
    

    emit("class ADLProcessor(processor.ProcessorABC):");
    emit("    def process(self,events):");

    level_of_indent = 8;

    emit_newline();
    emit("from collections import defaultdict");
    emit("output = defaultdict(float)");
    emit_newline();

    emit_newline();
    emit_comment("Setup output file");
    emit("import uproot");
    emit("out_file = uproot.recreate('", out_file, "')");

    emit_newline();
    emit_comment("Setup MET as a Lorentz vector compatible object");
    emit("METV_pt = ak.unflatten(events.", met_name, ".", met_names->pt(), ", counts=ak.ones_like(events.event))");
    emit("METV_phi = ak.unflatten(events.", met_name, ".", met_names->phi(), ", counts=ak.ones_like(events.event))");
    emit("METV_eta = ak.zeros_like(METV_pt)");
    emit("METV_mass = ak.zeros_like(METV_pt)");
    
    met_name = "METV";

    emit_newline();

    ALILCollection &commands = alil->get_commands();
    for (auto &command : commands.get_commands()) {
        handle_command(command);
    }

    emit("out_file.close()");

    emit("output['n_events'] += len(events)");
    emit("return dict(output)");

    emit_newline();

    level_of_indent = 0;

    emit("    def postprocess(self, accumulator):");
    emit("        return accumulator");

    emit("runner = coffea.processor.Runner(");
    emit("    executor = coffea.processor.IterativeExecutor(),");
    emit("    schema = NanoAODSchema,");
    emit("    chunksize=1_000,");
    emit(")");

    emit_newline();
    emit("result = runner(");
    emit("    fileset,");
    emit("    treename='", events_tree_name, "',");
    emit("    processor_instance=ADLProcessor(),");
    emit(")");

    emit_newline();
    emit_comment("Print cutflow reports");
    if (config.get_argument("cutflowout") == "file") {
        emit("print_all_cutflows(result, file_out=output_file_name)");
    } else {
        emit("print_all_cutflows(result)");
    }

}