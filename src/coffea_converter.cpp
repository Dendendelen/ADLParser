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

static bool is_string(std::string in) {
    static const std::regex reg_string("\"[^\"]*\"");
    return (std::regex_match(in, reg_string));
}

static bool is_number(std::string in) {
    static const std::regex reg_number("-{0,1}[0-9]*\\.{0,1}[0-9]*([Ee][-+]{0,1}[0-9]+){0,1}");
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

std::string CoffeaConverter::attribute(std::string attr, std::string object, std::string separator_chars) {
    std::stringstream delimit;
    std::stringstream attributed_text;

    delimit << object;
    std::vector<std::string> delimited_at_attributing_points;
    std::string buffer;
    while (std::getline(delimit, buffer, '\x1d')) {
        delimited_at_attributing_points.push_back(buffer);
    }

    if (delimited_at_attributing_points.size() == 1) {
        attributed_text << delimited_at_attributing_points[0] << separator_chars << attr;
        return attributed_text.str();
    }

    bool is_first = true;
    for (std::string chunk : delimited_at_attributing_points) {
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
    std::stringstream lorentz;
    lorentz << "ak.zip({";
    lorentz << "'pt': " << attribute(main_names->pt(), object);
    lorentz << ", 'eta': " << attribute(main_names->eta(), object);
    lorentz << ", 'phi': " << attribute(main_names->phi(), object);
    lorentz << ", 'mass': " << attribute(main_names->mass(), object);
    lorentz << "}, with_name='PtEtaPhiMLorentzVector', behavior=vector.behavior)";
    return lorentz.str();
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
    std::string last_val = get_mapped_source(command, 0);
    std::string this_val = get_mapped_source(command, 1);
    
    if (last_val == "") {
        if (is_subtraction) {
            std::stringstream negate;
            negate << "-(" << lorentzify(this_val) << ")";
            return negate.str();
        }
        return lorentzify(this_val);
    } else {
        std::stringstream lorentz_addition;
        std::string dest = get_mapped_dest(command);
        
        emit_newline();
        emit_comment("Particle ", is_subtraction ? "subtraction" : "addition", " by combining Lorentz vectors");
        
        lorentz_addition << "(" << lorentzify(last_val) << (is_subtraction ? " - " : " + ") << lorentzify(this_val) << ")";
        
        emit(dest, "_lorentzvector = ", lorentz_addition.str());
        
        emit_comment("Get NanoAOD equivalent variables from Lorentz vector");
        emit(dest, attribute_delimiter, main_names->pt(), " = ", dest, "_lorentzvector.pt");
        emit(dest, attribute_delimiter, main_names->eta(), " = ", dest, "_lorentzvector.eta");
        emit(dest, attribute_delimiter, main_names->phi(), " = ", dest, "_lorentzvector.phi");
        emit(dest, attribute_delimiter, main_names->mass(), " = ", dest, "_lorentzvector.mass");

        emit_comment("Add charges manually");
        emit(dest, attribute_delimiter, main_names->charge(), " = ", attribute(main_names->charge(), last_val), " + ", attribute(main_names->charge(), this_val));

        return dest;
    }
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
    emit_newline();
    emit_comment("Create new region: ", command.get_dest_argument());
    emit(get_mapped_dest(command), "_mask = ak.ones_like(events.event, dtype=bool)");
    emit(get_mapped_dest(command), "_weights = []");
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_merge_regions(const AnalysisCommand &command) {
    std::string name_first = get_mapped_source(command, 0);
    std::string name_second = get_mapped_source(command, 1);

    emit_newline();
    emit_comment("Merge regions ", name_first, " and ", name_second);
    emit(get_mapped_dest(command), "_mask = ", name_first, "_mask & ", name_second, "_mask");
    emit(get_mapped_dest(command), "_weights = ", name_first, "_weights + ", name_second, "_weights");

    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_cut_region(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command, 0);
    std::string cut = get_mapped_source(command, 1);

    emit_newline();
    emit_comment("Apply cut to region");
    emit(get_mapped_dest(command), "_mask = ", prev, "_mask & (", cut, ")");
    emit(get_mapped_dest(command), "_weights = ", prev, "_weights.copy()");
    
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_create_bin_of_region(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command, 0);
    std::string cut = get_mapped_source(command, 1);
    
    emit_newline();
    emit_comment("Create bin of region");
    emit(get_mapped_dest(command), "_mask = ", prev, "_mask & (", cut, ")");
    emit(get_mapped_dest(command), "_weights = ", prev, "_weights.copy()");
    
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
    std::stringstream extern_part;
    extern_part << "events." << std::regex_replace(command.get_source_argument(0), reg_quote, "");
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

    emit_newline();
    emit_comment("Create object ", command.get_dest_argument(), " from mask");
    emit(get_mapped_dest(command), " = ", attribute("", source_name, ""), "[", last_mask, "]");

    return get_mapped_dest(command) + "\x1d";
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
    emit("    'fill_var': '", fill_var, "'");
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
    emit("    'fill_var_x': '", fill_var_x, "',");
    emit("    'fill_var_y': '", fill_var_y, "'");
    emit("}");
    
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_weight_apply(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command, 0);
    std::string weight = get_mapped_source(command, 1);

    emit_newline();
    emit_comment("Apply weight correction: ", weight);
    emit(get_mapped_dest(command), "_mask = ", prev, "_mask");
    emit(get_mapped_dest(command), "_weights = ", prev, "_weights + ['", weight, "']");
    
    return get_mapped_dest(command);
}

std::string CoffeaConverter::convert_do_cutflow_on_region(const AnalysisCommand &command) {
    std::string reg_name = get_mapped_source(command, 0);

    std::regex e("^V[0-9]+REG");
    std::string clean_reg_name = std::regex_replace(reg_name, e, "");

    emit_newline();
    emit_comment("Generate cutflow report for region: ", reg_name);
    emit("print('Cutflow for region ", clean_reg_name, "')");
    emit("print(f'Events passing: {ak.sum(", reg_name, "_mask)}')");
    
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
    emit_newline();
    emit_comment("Sort collection in ascending order");
    emit(get_mapped_dest(command), " = ", attribute("", get_mapped_source(command, 0), ""), "[ak.argsort(", get_mapped_source(command, 1), ", ascending=True)]");
    return get_mapped_dest(command) + "\x1d";
}

std::string CoffeaConverter::convert_obj_sort_descend(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Sort collection in descending order");
    emit(get_mapped_dest(command), " = ", attribute("", get_mapped_source(command, 0), ""), "[ak.argsort(", get_mapped_source(command, 1), ", ascending=False)]");
    return get_mapped_dest(command) + "\x1d";
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
    std::stringstream index;
    index << get_mapped_source(command, 0);
    index << "[:, " << get_mapped_source(command, 1) << "]";
    return index.str();
}

std::string CoffeaConverter::convert_expr_index_range(const AnalysisCommand &command) {
    std::stringstream index;
    index << get_mapped_source(command, 0);
    index << "[:, " << get_mapped_source(command, 1) << ":" << get_mapped_source(command, 2) << "]";
    return index.str();
}

std::string CoffeaConverter::convert_expr_index_until(const AnalysisCommand &command) {
    std::stringstream index;
    index << get_mapped_source(command, 0);
    index << "[:, :" << get_mapped_source(command, 1) << "]";
    return index.str();
}

std::string CoffeaConverter::convert_expr_index_from(const AnalysisCommand &command) {
    std::stringstream index;
    index << get_mapped_source(command, 0);
    index << "[:, " << get_mapped_source(command, 1) << ":]";
    return index.str();
}

std::string CoffeaConverter::convert_func_charge(const AnalysisCommand &command) {
    return attribute(main_names->charge(), get_mapped_source(command, 0));
}

std::string CoffeaConverter::convert_func_pt(const AnalysisCommand &command) {
    return attribute(main_names->pt(), get_mapped_source(command, 0));
}

std::string CoffeaConverter::convert_func_eta(const AnalysisCommand &command) {
    return attribute(main_names->eta(), get_mapped_source(command, 0));
}

std::string CoffeaConverter::convert_func_phi(const AnalysisCommand &command) {
    return attribute(main_names->phi(), get_mapped_source(command, 0));
}

std::string CoffeaConverter::convert_func_mass(const AnalysisCommand &command) {
    return attribute(main_names->mass(), get_mapped_source(command, 0));
}

std::string CoffeaConverter::convert_func_energy(const AnalysisCommand &command) {
    std::stringstream energy;
    energy << lorentzify(get_mapped_source(command, 0)) << ".energy";
    return energy.str();
}

std::string CoffeaConverter::convert_func_distinct(const AnalysisCommand &command) {
    std::string first_provenance = attribute("provenance", get_mapped_source(command, 0));
    std::string second_provenance = attribute("provenance", get_mapped_source(command, 1));

    std::stringstream computation;
    computation << "(" << first_provenance << " != " << second_provenance << ")";
    return computation.str();
}

std::string CoffeaConverter::convert_func_dr(const AnalysisCommand &command) {
    std::stringstream dr;
    dr << lorentzify(get_mapped_source(command, 0)) << ".delta_r(" << lorentzify(get_mapped_source(command, 1)) << ")";
    return dr.str();
}

std::string CoffeaConverter::convert_func_dphi(const AnalysisCommand &command) {
    std::stringstream dphi;
    dphi << lorentzify(get_mapped_source(command, 0)) << ".delta_phi(" << lorentzify(get_mapped_source(command, 1)) << ")";
    return dphi.str();
}

std::string CoffeaConverter::convert_func_deta(const AnalysisCommand &command) {
    std::stringstream deta;
    deta << "(" << attribute(main_names->eta(), get_mapped_source(command, 0));
    deta << " - " << attribute(main_names->eta(), get_mapped_source(command, 1)) << ")";
    return deta.str();
}

std::string CoffeaConverter::convert_func_dr_hadamard(const AnalysisCommand &command) {
    // for Hadamard (element-wise) operations on jagged arrays
    std::stringstream dr;
    dr << "ak.flatten(" << lorentzify(get_mapped_source(command, 0)) << ".delta_r(" << lorentzify(get_mapped_source(command, 1)) << "))";
    return dr.str();
}

std::string CoffeaConverter::convert_func_dphi_hadamard(const AnalysisCommand &command) {
    std::stringstream dphi;
    dphi << "ak.flatten(" << lorentzify(get_mapped_source(command, 0)) << ".delta_phi(" << lorentzify(get_mapped_source(command, 1)) << "))";
    return dphi.str();
}

std::string CoffeaConverter::convert_func_deta_hadamard(const AnalysisCommand &command) {
    std::stringstream deta;
    deta << "ak.flatten(" << attribute(main_names->eta(), get_mapped_source(command, 0));
    deta << " - " << attribute(main_names->eta(), get_mapped_source(command, 1)) << ")";
    return deta.str();
}

std::string CoffeaConverter::convert_func_size(const AnalysisCommand &command) {
    std::string momentum_of_part = attribute(main_names->pt(), get_mapped_source(command, 0));
    std::stringstream computation;
    computation << "ak.num(" << momentum_of_part << ", axis=1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_anyof(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.any(" << get_mapped_source(command, 0) << ", axis=1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_allof(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.all(" << get_mapped_source(command, 0) << ", axis=1)";
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
    computation << "ak.mean(" << get_mapped_source(command, 0) << ", axis=1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_sum(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.sum(" << get_mapped_source(command, 0) << ", axis=1)";
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
    computation << "ak.min(" << get_mapped_source(command, 0) << ", axis=1)";
    return computation.str();
}

std::string CoffeaConverter::convert_func_max_of_list(const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "ak.max(" << get_mapped_source(command, 0) << ", axis=1)";
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

    if (prev_union == "") {
        return new_to_add;
    } else {
        emit_newline();
        emit_comment("Create object ", command.get_dest_argument(), " from a union");
        emit(get_mapped_dest(command), " = ak.concatenate([", 
             attribute("", prev_union, ""), ", ", 
             attribute("", new_to_add, ""), "], axis=1)");
        return get_mapped_dest(command) + "\x1d";
    }
}

std::string CoffeaConverter::convert_create_empty_cartesian(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{'type': 'cartesian', 'collections': []}";
}

std::string CoffeaConverter::convert_create_empty_disjoint(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{'type': 'disjoint', 'collections': []}";
}

std::string CoffeaConverter::convert_create_empty_direct(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{'type': 'direct', 'collections': []}";
}

std::string CoffeaConverter::convert_add_part_to_composite(const AnalysisCommand &command) {
    std::string prev_composite = get_mapped_source(command, 0);
    std::string new_collection = attribute(main_names->pt(), get_mapped_source(command, 1));
    
    std::regex end_list("\\]\\}");
    std::string unended = std::regex_replace(prev_composite, end_list, "");
    
    std::stringstream new_composite;
    new_composite << unended << new_collection << "]}";
    return new_composite.str();
}

std::string CoffeaConverter::convert_name_element_of_composite(const AnalysisCommand &command) {
    std::string source_name = get_mapped_source(command, 2);
    std::string comb_name = get_mapped_source(command, 0);
    std::string comb_index = get_mapped_source(command, 1);

    emit_newline();
    emit_comment("Create object ", command.get_dest_argument(), " as an element of a composite");
    emit(get_mapped_dest(command), " = ", attribute("", source_name, ""), "[", comb_name, "[:, ", comb_index, "]]");
    
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


void CoffeaConverter::handle_command(const AnalysisCommand &command) {
    std::string resultant = command_convert(command);
    if (command.has_dest_argument()) {
        add_mapping(command.get_dest_argument(), resultant);
        add_mapping(get_mapped_dest(command), resultant);
    }
}

void CoffeaConverter::print() {
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
    emit("from coffea.nanoevents import NanoEventsFactory, NanoAODSchema");
    emit("import awkward as ak");
    emit("import numpy as np");
    emit("import hist");
    emit("import vector");
    emit("vector.register_awkward()");
    emit("");
    emit("import sys, os");
    
    // Add path to helper functions
    emit("adl_help_dir = os.path.abspath('", path_to_helper_py.string(), "')");
    emit("if adl_help_dir not in sys.path:");
    emit("    sys.path.append(adl_help_dir)");
    
    emit("from adl_coffea_helpers import fill_histogram, fill_histogram_list, create_function_from_table");
    
    emit_newline();
    emit_comment("Load events from input file");
    emit("events = NanoEventsFactory.from_root('", in_file, "', schemaclass=NanoAODSchema, treepath='", events_tree_name, "').events()");
    
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

    emit_newline();
    emit("out_file.close()");
}