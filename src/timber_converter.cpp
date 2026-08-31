#include "timber_converter.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


void TimberConverter::add_mapping(std::string source, std::string dest) {
    var_mappings.emplace(source, dest);
}

bool is_string(std::string in) {
    static const std::regex reg_string("\"[^\"]*\"");
    return (std::regex_match(in, reg_string));
}

bool is_number(std::string in) {
    static const std::regex reg_number("-{0,1}[0-9]*\\.{0,1}[0-9]*([Ee][-+]{0,1}[0-9]+){0,1}");
    return (std::regex_match(in, reg_number));
}

std::string TimberConverter::get_mapping(std::string in) {

    if (is_string(in) || is_number(in)) return in;
    
    std::string mapped = var_mappings[in];

    if (mapped == in) return in;
    if (var_mappings.contains(mapped)) return get_mapping(mapped);

    return mapped;
}

std::string TimberConverter::get_mapped_source(const AnalysisCommand &command, size_t pos) {
    std::string orig_source = command.get_source_argument(pos);

    // before mapping, apply a regex to remove all special characters from variable names which are allowed in ALIL but forbidden in C++/Python
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

std::string TimberConverter::get_mapped_dest(const AnalysisCommand &command) {
    std::string orig_dest = command.get_dest_argument();
    
    // apply a regex to remove all special characters from variable names which are allowed in ALIL but forbidden in C++/Python
    std::regex e("[_\\->]");


    std::string escaped_dest;

    if (is_string(orig_dest) || is_number(orig_dest)) {
        escaped_dest = orig_dest;
    } else {
        escaped_dest = std::regex_replace(orig_dest, e, "");
    }

    return escaped_dest;
}


std::string TimberConverter::list_append(std::string list_end, std::string delimiter, const AnalysisCommand &command, std::string to_add) {
    std::string old_list = get_mapped_source(command, 0);

    std::regex sanitize{R"([-[\]{}()*+?.,\^$|#\s])"};
    std::string list_end_sanitized = std::regex_replace(list_end, sanitize, R"(\$&)");

    std::regex reg_list_end(list_end_sanitized);
    old_list = std::regex_replace(old_list, reg_list_end, "");

    std::stringstream add_val;
    add_val << old_list;
    add_val << (to_add == "" ? get_mapped_source(command,1) : to_add);
    add_val << delimiter << list_end;
    return add_val.str();
}

std::string TimberConverter::attribute(std::string attr, std::string object, std::string separator_chars) {
    std::stringstream delimit;
    std::stringstream attributed_text;

    // we need to add our attribute name at each of these points, since the attribute must belong to the particle itself, and not any resultants.
    delimit << object;
    std::vector<std::string> delimited_at_attributing_points;
    std::string buffer;
    while (std::getline(delimit, buffer, '\x1d')) {
        delimited_at_attributing_points.push_back(buffer);
    }

    if (delimited_at_attributing_points.size() == 1) {
        // if we have only one element, presume that we need to append to the end regardless of anything else
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

std::string TimberConverter::attribute(std::string attr, std::string object) {
    return attribute(attr, object, attribute_delimiter);
}

std::string TimberConverter::lorentzify(std::string object) {
    std::stringstream lorentz;
    lorentz << "TLV(" << attribute("pt", object);
    lorentz << "," << attribute("eta", object);
    lorentz << "," << attribute("phi", object);
    lorentz << "," << attribute("mass", object);
    lorentz << ")";

    return lorentz.str();
}

std::string TimberConverter::multi_arg_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok, bool is_lorentz) {
    std::stringstream func;
    func << func_name << "(";
    bool is_first = true;
    for (int i = 0; i < num_args; i++) {
        if (is_first) {is_first = false;}
        else {func << ",";}

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

std::string TimberConverter::multi_arg_lorentz_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok) {
    return multi_arg_function(func_name, num_args, command, ending_tok, true);
}

std::string TimberConverter::binary_infix_operation(std::string op_name, const AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(" << get_mapped_source(command, 0);
    computation << op_name;
    computation << get_mapped_source(command, 1) << ")";

    return computation.str();
}

std::string TimberConverter::interval(std::string left_bound_op, std::string right_bound_op, const AnalysisCommand &command) {
    std::stringstream within;
    within << "(";
    within << "(" << get_mapped_source(command,0) << left_bound_op << get_mapped_source(command,1) << ")";
    within << "&&";
    within << "(" << get_mapped_source(command,0) << right_bound_op << get_mapped_source(command,2) << ")";
    within << ")";

    return within.str();
}

std::string TimberConverter::add_subtract_particles(const AnalysisCommand &command, bool is_subtraction) {
    std::string last_val = get_mapped_source(command,0);
    std::string this_val = get_mapped_source(command,1);
    if (last_val == "") {
        if (is_subtraction) {
            std::stringstream negate;
            negate << "-(" << this_val << ")";
            return negate.str();
        }
        return this_val;
    } else {
        std::stringstream lorentz_addition;
        std::string dest = get_mapped_dest(command);
        lorentz_addition << "(" << lorentzify(last_val) << (is_subtraction ? "-" : "+") << lorentzify(this_val) << ")";
        
        emit_newline();
        emit_comment("Particle ", is_subtraction ? "subtraction" : "addition", "by combining Lorentz vectors");
        emit("a.Define('",dest,"_lorentzvector','",lorentz_addition.str(),"')");
        
        emit_comment("Get NanoAOD equivalent variables from Lorentz vector");
        emit("a.Define('",dest,"_pt', 'Pt(",dest,"_lorentzvector)')");
        emit("a.Define('",dest,"_eta', 'Eta(",dest,"_lorentzvector)')");
        emit("a.Define('",dest,"_phi', 'Phi(",dest,"_lorentzvector)')");
        emit("a.Define('",dest,"_mass', 'M(",dest,"_lorentzvector)')");

        emit_comment("Add charges manually");
        emit("a.Define('",dest,"_charge', '", attribute("charge", last_val), "+", attribute("charge", this_val), "')");

        return dest;
    }
} 

std::string TimberConverter::use_within_region(std::string fun_within_node, const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Save current node before applying region");
    emit("_old_node = a.GetActiveNode()");
    
    emit_comment("Apply region cuts and corrections");
    emit("_this_reg_node_", command.get_source_argument(1), " = a.Apply(", get_mapped_source(command,1), "[0])");
    emit("_this_reg_node_", command.get_source_argument(1), " = a.AddCorrections(", get_mapped_source(command,1), "[1])");
    
    emit_comment("Execute function within region context");
    emit(fun_within_node, "(", get_mapped_source(command,0), ", _this_reg_node_", command.get_source_argument(1), ")");
    
    emit_comment("Restore previous node");
    emit("a.SetActiveNode(_old_node)");

    return get_mapped_dest(command);
}



std::string TimberConverter::convert_conversion_error(const AnalysisCommand &command) {
    assert(command.get_num_arguments() == 0);
    assert(false);
}
std::string TimberConverter::convert_create_empty_info_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{}";
}
std::string TimberConverter::convert_add_to_info_list(const AnalysisCommand &command) {
    std::stringstream info_pair;
    info_pair << "'" << command.get_source_argument(1) << "':'" << command.get_source_argument(2) << "'";
    return list_append("}", ",", command, info_pair.str());
}
std::string TimberConverter::convert_display_info(const AnalysisCommand &command) {
    std::string info_list = get_mapped_source(command,0);
    emit_newline();
    emit_comment("Display analysis info");
    emit("print("
        , info_list
        , ")"
    );
    return "";
}
std::string TimberConverter::convert_create_region(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Create new region: ", command.get_dest_argument());
    emit(get_mapped_dest(command), "= [CutGroup('", get_mapped_dest(command), "'), []]");
    return get_mapped_dest(command);
}
std::string TimberConverter::convert_merge_regions(const AnalysisCommand &command) {

    std::string name_first = get_mapped_source(command,0);
    std::string name_second = get_mapped_source(command,1);

    emit_newline();
    emit_comment("Merge regions ", name_first, " and ", name_second);
    emit(get_mapped_dest(command)," = [", name_first,"[0] + ",name_second,"[0], "
    , name_first,"[1] + ", name_second,"[1]","]");

    return get_mapped_dest(command);
}
std::string TimberConverter::convert_cut_region(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command,0);
    std::string cut = get_mapped_source(command,1);

    emit_comment("Apply cut to region");
    emit(get_mapped_dest(command), " = [",prev,"[0].Add('",get_mapped_dest(command),"','",cut,"', makeCopy=True), ",prev,"[1]","]");
    return get_mapped_dest(command);
}
std::string TimberConverter::convert_create_bin_of_region(const AnalysisCommand &command) {

    std::string prev = get_mapped_source(command,0);
    std::string cut = get_mapped_source(command,1);
    
    emit_comment("Create bin of region");
    emit(get_mapped_dest(command), " = [",prev,"[0].Add('",get_mapped_dest(command),"','",cut,"', makeCopy=True), ",prev,"[1]","]");
    return get_mapped_dest(command);
}

std::string TimberConverter::convert_add_alias(const AnalysisCommand &command) {
    return command.get_source_argument(0);
}
std::string TimberConverter::convert_add_external(const AnalysisCommand &command) {
    std::regex reg_quote;
    reg_quote= std::regex("\"");

    std::string extern_final_name = std::regex_replace(command.get_source_argument(0),reg_quote,"");
    is_attribute.emplace(extern_final_name);
    return extern_final_name;
}
std::string TimberConverter::convert_add_extern_attr(const AnalysisCommand &command) {
    std::regex reg_quote;
    reg_quote= std::regex("\"");

    std::string attr_final_name = std::regex_replace(command.get_source_argument(0),reg_quote,"");
    is_attribute.emplace(attr_final_name);
    return attr_final_name;
}
std::string TimberConverter::convert_add_extern_particle(const AnalysisCommand &command) {
    std::regex reg_quote;
    reg_quote= std::regex("\"");
    
    std::stringstream extern_part;
    extern_part << std::regex_replace(command.get_source_argument(0), reg_quote, "");
    extern_part << '\x1d';
    return extern_part.str();
}
std::string TimberConverter::convert_add_correctionlib(const AnalysisCommand &command) {
    //TODO:
}
std::string TimberConverter::convert_create_mask(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Create selection mask ", command.get_dest_argument(), " from shape of ", command.get_source_argument(0));
    emit(get_mapped_dest(command)
        , " = VarGroup('"
        , get_mapped_dest(command)
        , "')");

    std::string shape_of_out = attribute("pt", get_mapped_source(command, 0));

    emit(get_mapped_dest(command)
        , ".Add('"
        , get_mapped_dest(command)
        , "', 'create_mask("
        , shape_of_out
        , ")')"
    );

    return get_mapped_dest(command);
       
}
std::string TimberConverter::convert_limit_mask(const AnalysisCommand &command) {

    emit_comment("Apply limit to mask");
    emit(get_mapped_dest(command)
        , " = "
        , get_mapped_source(command,0)
        , ".Add('"
        , get_mapped_dest(command)
        , "', 'limit_mask("
        , get_mapped_source(command,0)
        , ", "
        , get_mapped_source(command,1)
        , ")', makeCopy=True)"
    );

    return get_mapped_dest(command);
}
std::string TimberConverter::convert_apply_mask(const AnalysisCommand &command) {

    std::string last_mask = get_mapped_source(command, 0);
    std::string source_name = get_mapped_source(command, 1);

    emit_newline();
    emit_comment("Create object ", command.get_dest_argument(), " from mask");
    emit("a.Apply("
        , last_mask
        , ")"
    );

    emit("a.SubCollection('"
        , get_mapped_dest(command)
        , "', '"
        , attribute("", source_name, "")
        , "', '"
        , last_mask
        , "', useTake=False, skip=[\"idx\"])"
    );

    return get_mapped_dest(command);
}
std::string TimberConverter::convert_create_empty_hist_list(const AnalysisCommand &command) {
    assert(command.get_num_arguments() == 0);
    return "[]";
}
std::string TimberConverter::convert_add_hist_to_list(const AnalysisCommand &command) {
    return list_append("]", ",", command);
}
std::string TimberConverter::convert_use_hist(const AnalysisCommand &command) {
    emit_comment("Use histogram in region");
    return use_within_region("use_histo", command);
}
std::string TimberConverter::convert_use_hist_list(const AnalysisCommand &command) {
    emit_comment("Use histogram list in region");
    return use_within_region("use_histo_list",command);
}
std::string TimberConverter::convert_hist_1d(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Define 1D histogram: ", command.get_dest_argument());
    emit(get_mapped_dest(command), " = []");
    emit(get_mapped_dest(command), ".append('", get_mapped_dest(command), "')");
    for (int i = 0; i < 4; i++) {
        emit(get_mapped_dest(command), ".append('", get_mapped_source(command,i), "')");
    }
    return get_mapped_dest(command);
}
std::string TimberConverter::convert_hist_2d(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Define 2D histogram: ", command.get_dest_argument());
    emit(get_mapped_dest(command), " = []");
    emit(get_mapped_dest(command), ".append('", get_mapped_dest(command), "')");
    for (int i = 0; i < 7; i++) { //TODO: check that this is the correct ordering
        emit(get_mapped_dest(command), ".append('", get_mapped_source(command,i), "')");
    }
    return get_mapped_dest(command);  
}
std::string TimberConverter::convert_weight_apply(const AnalysisCommand &command) {
    std::string prev = get_mapped_source(command,0);
    std::string weight = get_mapped_source(command,1);

    emit_comment("Apply weight correction: ", weight);
    emit(get_mapped_dest(command), " = [",prev,"[0], " ,prev,"[1] + [Correction('", weight, "', '', '", weight, "')] ]");
    return get_mapped_dest(command);
}


std::string TimberConverter::convert_do_cutflow_on_region(const AnalysisCommand &command) {

    std::string reg_name = get_mapped_source(command,0);

    emit_newline();
    emit_comment("Generate cutflow report for region: ", reg_name);
    emit("_old_node = a.GetActiveNode()");
    std::stringstream line;

    emit_comment("Apply region cuts and corrections for cutflow");
    emit("_cutflow_node_", reg_name, " = a.Apply(", reg_name, "[0])");
    emit("_cutflow_node_", reg_name, " = a.AddCorrections(", reg_name, "[1])");
    
    emit_newline();
    emit_comment("Print cutflow table in LaTeX format");
    emit("print('\\n---\\n \\\\begin{tabular}{c c c c} \\\\multicolumn{4}{c}{Cutflow report for region "
        , reg_name
        ,"}\\\\\\\\ \\\\hline Cut & Events left & Eff from previous & Eff from initial \\\\\\\\ \\\\hline')");
    emit("for _cutflow_k, _cutflow_v in CutflowDict(_cutflow_node_", reg_name, ").items():");
    emit("    _this_name = _cutflow_k");
    emit("    if _this_name != 'Initial':");
    emit("        _this_name = ", reg_name, "[0].items[_cutflow_k]");
    emit("        _this_name = re.sub('[A-Za-z0-9]*UNION','',_this_name)");
    emit("    else:");
    emit("        _init = _cutflow_v\n        _prev = _init");
    emit("    print('\\\\verb`' + _this_name + '` "
        , "& ' + str(_cutflow_v) + ' & ' + f'{(_cutflow_v/(_prev+1e-9)):.2%}'[:-1] + '\\\\% & ' + f'{(_cutflow_v/_init):.4%}'[:-1] + '\\\\%\\\\\\\\')");
    emit("    _prev = _cutflow_v");
    emit("print('\\\\end{tabular} \\n---\\n')");
    
    emit_newline();
    emit_comment("Restore previous node after cutflow");
    emit("a.SetActiveNode(_old_node)");

    return "";
}
std::string TimberConverter::convert_do_eventlist_on_region(const AnalysisCommand &command) {

    std::string reg_name = get_mapped_source(command,0);

    emit_newline();
    emit_comment("Generate event list for region: ", reg_name);
    emit("_old_node = a.GetActiveNode()");
    
    emit_comment("Apply region cuts and corrections for event list");
    emit("_eventlist_node_", reg_name, " = a.Apply(", reg_name, "[0])");

    emit("_eventlist_node_", reg_name, " = a.AddCorrections(", reg_name, "[1])");

    emit_newline();
    emit_comment("Print event list with run, luminosity block, and event numbers");
    emit("print('\\n---\\nBeginning event list for region ", reg_name, "')");


    emit("_eventlist_node_", reg_name, ".DataFrame.Display(columnList=['run', 'luminosityBlock', 'event'], nRows=1000).Print()");
    emit("print('\\n---\\n')");
    
    emit_comment("Restore previous node after event list");
    emit("a.SetActiveNode(_old_node)");

    return "";
}
std::string TimberConverter::convert_create_table(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "[ ]";
}
std::string TimberConverter::convert_create_table_errored_value(const AnalysisCommand &command) {
    std::stringstream errored_val;
    errored_val << "(" << get_mapped_source(command, 0) << "," << get_mapped_source(command, 1) << "," << get_mapped_source(command, 2) << ")";
    return errored_val.str();
}
std::string TimberConverter::convert_create_table_value(const AnalysisCommand &command) {
    return get_mapped_source(command, 0);
}
std::string TimberConverter::convert_append_to_table(const AnalysisCommand &command) {
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
    new_table << unended_table << "\n    (" << value << "," << lower_bounds_brackets << "," << upper_bounds_brackets << "),  ]";
    return new_table.str();
}
std::string TimberConverter::convert_finish_table(const AnalysisCommand &command) {
    emit_newline();
    emit_comment("Creating a multi-argument function ", command.get_dest_argument(), " out of a table");
    emit("create_function_out_of_table('",get_mapped_dest(command), "', ", get_mapped_source(command, 0), ")");
    return get_mapped_dest(command); 
}


std::string TimberConverter::convert_obj_sort_ascend(const AnalysisCommand &command) {
    emit_comment("Sort collection in ascending order");
    emit("a.SubCollection('"
        , get_mapped_dest(command)
        , "', '"
        , attribute("", get_mapped_source(command, 0), "")
        , "', 'ROOT::VecOps::Argsort("
        , get_mapped_source(command,1)
        , ")', useTake=True)"
    );
    return get_mapped_dest(command);
}
std::string TimberConverter::convert_obj_sort_descend(const AnalysisCommand &command) {
    emit_comment("Sort collection in descending order");
    emit("a.SubCollection('"
        , get_mapped_dest(command)
        , "', '"
        , attribute("", get_mapped_source(command, 0), "")
        , "', 'ROOT::VecOps::Reverse(ROOT::VecOps::Argsort("
        , get_mapped_source(command,1)
        , "))', useTake=True)"
    );
    return get_mapped_dest(command);
}
std::string TimberConverter::convert_expr_raise(const AnalysisCommand &command) {
    return multi_arg_function("raise_power", 2, command);
}
std::string TimberConverter::convert_expr_multiply(const AnalysisCommand &command) {
    return binary_infix_operation("*", command);
}
std::string TimberConverter::convert_expr_divide(const AnalysisCommand &command) {
    return binary_infix_operation("/", command);
}
std::string TimberConverter::convert_expr_add(const AnalysisCommand &command) {
    return binary_infix_operation("+", command);
}
std::string TimberConverter::convert_expr_subtract(const AnalysisCommand &command) {
    return binary_infix_operation("-", command);
}
std::string TimberConverter::convert_expr_lt(const AnalysisCommand &command) {
    return binary_infix_operation("<", command);
}
std::string TimberConverter::convert_expr_le(const AnalysisCommand &command) {
    return binary_infix_operation("<=", command);
}
std::string TimberConverter::convert_expr_gt(const AnalysisCommand &command) {
    return binary_infix_operation(">", command);
}
std::string TimberConverter::convert_expr_ge(const AnalysisCommand &command) {
    return binary_infix_operation(">=", command);
}
std::string TimberConverter::convert_expr_eq(const AnalysisCommand &command) {
    return binary_infix_operation("==", command);
}
std::string TimberConverter::convert_expr_ne(const AnalysisCommand &command) {
    return binary_infix_operation("!=", command);
}
std::string TimberConverter::convert_expr_bitwise_and(const AnalysisCommand &command) {
    return binary_infix_operation("&", command);
}
std::string TimberConverter::convert_expr_bitwise_or(const AnalysisCommand &command) {
    return binary_infix_operation("|", command);
}
std::string TimberConverter::convert_expr_and(const AnalysisCommand &command) {
    return binary_infix_operation("&&", command);
}
std::string TimberConverter::convert_expr_or(const AnalysisCommand &command) {
    return binary_infix_operation("||", command);
}
std::string TimberConverter::convert_expr_within(const AnalysisCommand &command) {
    return interval(">=", "<=", command);
}
std::string TimberConverter::convert_expr_within_exclusive(const AnalysisCommand &command) {
    return interval(">", "<", command);
}
std::string TimberConverter::convert_expr_within_left_exclusive(const AnalysisCommand &command) {
    return interval(">", "<=", command);
}
std ::string TimberConverter::convert_expr_within_right_exclusive(const AnalysisCommand &command) {
    return interval(">=", "<", command);
}

std::string TimberConverter::convert_expr_negate(const AnalysisCommand &command) {
    return multi_arg_function("-", 1, command);
}
std::string TimberConverter::convert_expr_logical_not(const AnalysisCommand &command) {
    return multi_arg_function("!", 1, command);
}
std::string TimberConverter::convert_expr_if_ternary(const AnalysisCommand &command) {
    std::stringstream ternary;
    ternary << binary_infix_operation("?", command);
    ternary << ":" << get_mapped_source(command,2);
    return ternary.str();
}
std::string TimberConverter::convert_expr_index(const AnalysisCommand &command) {
    std::stringstream index;
    index << get_mapped_source(command,0);
    index << "[" << get_mapped_source(command,1) << "]";
    return index.str();
}
std::string TimberConverter::convert_expr_index_range(const AnalysisCommand &command) {
    return multi_arg_function("index_get", 3, command);
}
std::string TimberConverter::convert_expr_index_until(const AnalysisCommand &command) {
    return multi_arg_function("index_unti", 2, command);
}
std::string TimberConverter::convert_expr_index_from(const AnalysisCommand &command) {
    return multi_arg_function("index_from", 2, command);
}
std::string TimberConverter::convert_func_charge(const AnalysisCommand &command) {
    return attribute("charge", get_mapped_source(command,0));
}
std::string TimberConverter::convert_func_pt(const AnalysisCommand &command) {
    return attribute("pt", get_mapped_source(command,0));
}
std::string TimberConverter::convert_func_eta(const AnalysisCommand &command) {
    return attribute("eta", get_mapped_source(command,0));
}
std::string TimberConverter::convert_func_phi(const AnalysisCommand &command) {
    return attribute("phi", get_mapped_source(command,0));
}
std::string TimberConverter::convert_func_mass(const AnalysisCommand &command) {
    return attribute("mass", get_mapped_source(command,0));
}
std::string TimberConverter::convert_func_energy(const AnalysisCommand &command) {
    std::stringstream energy;
    energy << lorentzify(get_mapped_source(command,0)) << ".E()";
    return energy.str();
}
std::string TimberConverter::convert_func_distinct(const AnalysisCommand &command) {
    std::string first_provenance = attribute("provenance", get_mapped_source(command,0));
    std::string second_provenance = attribute("provenance", get_mapped_source(command,0));

    std::stringstream computation;
    computation << "(" << first_provenance << "!=" << second_provenance << ")";

    return computation.str();
}
std::string TimberConverter::convert_func_dr(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaR", 2, command);
}
std::string TimberConverter::convert_func_dphi(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaPhi", 2, command);
}
std::string TimberConverter::convert_func_deta(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaEta", 2, command);
}
std::string TimberConverter::convert_func_dr_hadamard(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaRHadamard", 2, command);
}
std::string TimberConverter::convert_func_dphi_hadamard(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaPhiHadamard", 2, command);
}
std::string TimberConverter::convert_func_deta_hadamard(const AnalysisCommand &command) {
    return multi_arg_lorentz_function("DeltaEtaHadamard", 2, command);
}
std::string TimberConverter::convert_func_size(const AnalysisCommand &command) {
    std::string momentum_of_part = attribute("pt", command.get_source_argument(0));
    std::stringstream computation;
    computation << "(size(" << momentum_of_part << "))";
    return computation.str();
}
std::string TimberConverter::convert_func_anyof(const AnalysisCommand &command) {
    return multi_arg_function("AnyOf", 1, command);
}
std::string TimberConverter::convert_func_allof(const AnalysisCommand &command) {
    return multi_arg_function("AllOf", 1, command);
}
std::string TimberConverter::convert_func_sqrt(const AnalysisCommand &command) {
    return multi_arg_function("sqrt", 1, command);
}
std::string TimberConverter::convert_func_abs(const AnalysisCommand &command) {
    return multi_arg_function("abs", 1, command);
}
std::string TimberConverter::convert_func_cos(const AnalysisCommand &command) {
    return multi_arg_function("cos", 1, command);
}
std::string TimberConverter::convert_func_sin(const AnalysisCommand &command) {
    return multi_arg_function("sin", 1, command);
}
std::string TimberConverter::convert_func_tan(const AnalysisCommand &command) {
    return multi_arg_function("tan", 1, command);
}
std::string TimberConverter::convert_func_sinh(const AnalysisCommand &command) {
    return multi_arg_function("sinh", 1, command);
}
std::string TimberConverter::convert_func_cosh(const AnalysisCommand &command) {
    return multi_arg_function("cosh", 1, command);
}
std::string TimberConverter::convert_func_tanh(const AnalysisCommand &command) {
    return multi_arg_function("tanh", 1, command);
}
std::string TimberConverter::convert_func_exp(const AnalysisCommand &command) {
    return multi_arg_function("exp", 1, command);
}
std::string TimberConverter::convert_func_log(const AnalysisCommand &command) {
    return multi_arg_function("log", 1, command);
}
std::string TimberConverter::convert_func_ave(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Mean", 1, command);
}
std::string TimberConverter::convert_func_sum(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Sum", 1, command);
}
std::string TimberConverter::convert_func_min_of_pair(const AnalysisCommand &command) {
    return multi_arg_function("std::min", 2, command);
}
std::string TimberConverter::convert_func_max_of_pair(const AnalysisCommand &command) {
    return multi_arg_function("std::max", 2, command);
}
std::string TimberConverter::convert_func_min_of_list(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Min", 1, command);
}
std::string TimberConverter::convert_func_max_of_list(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Max", 1, command);
}
std::string TimberConverter::convert_func_sort_ascend(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Sort", 1, command);
}
std::string TimberConverter::convert_func_sort_descend(const AnalysisCommand &command) {
    return multi_arg_function("ROOT::VecOps::Reverse(ROOT::VecOps::Sort", 1, command, ")");
}
std::string TimberConverter::convert_func_named(const AnalysisCommand &command) {
    std::string name = get_mapped_source(command,1);
    if (is_attribute.contains(name)) {
        return attribute(name, get_mapped_source(command,0));
    } else {
        return multi_arg_function(name, 1, command);
    }
}
std::string TimberConverter::convert_create_empty_value_list(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "{}";
}
std::string TimberConverter::convert_add_value_to_list(const AnalysisCommand &command) {
    return list_append("}", ",", command);
}
std::string TimberConverter::convert_create_empty_union(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "";
}
std::string TimberConverter::convert_add_part_to_union(const AnalysisCommand &command) {
    std::string prev_union = get_mapped_source(command,0);
    std::string new_to_add = get_mapped_source(command,1);

    if (prev_union == "") {
        return new_to_add;
    } else {
        emit_newline();
        emit_comment("Create object ", command.get_dest_argument(), " from a union");
        emit("a.MergeCollections('"
            , get_mapped_dest(command)
            , "', ['"
            , attribute("", prev_union, "")
            , "', '"
            , attribute("", new_to_add, "")
            , "'])"
        );
        return get_mapped_dest(command);
    }
}
std::string TimberConverter::convert_create_empty_cartesian(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "Comb({})";
}
std::string TimberConverter::convert_create_empty_disjoint(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "Disjoint({})";
}
std::string TimberConverter::convert_create_empty_direct(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "Direct({})";
}
std::string TimberConverter::convert_add_part_to_composite(const AnalysisCommand &command) {
    return list_append("})", ",", command, attribute("pt", get_mapped_source(command,1)));
}
std::string TimberConverter::convert_name_element_of_composite(const AnalysisCommand &command) {

    std::string source_name = get_mapped_source(command,2);
    std::string comb_name = get_mapped_source(command,0);
    std::string comb_index = get_mapped_source(command,1);

    emit_newline();
    emit_comment("Create object ", command.get_dest_argument(), " as an element of a composite");
    emit("a.SubCollection('"
        , get_mapped_dest(command)
        , "', '"
        , attribute("", source_name, "")
        , "', '"
        , comb_name
        , "["
        , comb_index
        , "]"
        , "', useTake=True, skip=[\"idx\"])"
    );

    return get_mapped_dest(command);
}
std::string TimberConverter::convert_create_empty_particle(const AnalysisCommand &command) {
    assert(command.get_num_source_arguments() == 0);
    return "";
}
std::string TimberConverter::convert_add_particle(const AnalysisCommand &command) {
    return add_subtract_particles(command);
}
std::string TimberConverter::convert_sub_particle(const AnalysisCommand &command) {
    return add_subtract_particles(command, true);
}

void TimberConverter::handle_command(const AnalysisCommand &command) {

    std::string resultant = command_convert(command);
    if (command.has_dest_argument()) {
        add_mapping(command.get_dest_argument(), resultant);
        add_mapping(get_mapped_dest(command), resultant);
    }

}

void TimberConverter::print() {

    met_name = config.get_argument("MET");
    std::string in_file = config.get_argument("infile");
    std::string out_file = config.get_argument("outfile");

    std::string format = config.get_argument("format");

    std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) {
        return std::toupper(c);
    });


    attribute_delimiter = "_";
    if (format == "NANOAOD") {
        attribute_delimiter = "_";
    } else if (format == "DELPHES") {
        attribute_delimiter = ".";
    }

    // get the path for our helper functions, relying on the "ROOT DIR" macro which we set during compile time
    std::filesystem::path abs_path = std::filesystem::absolute(ROOT_DIR);
    std::filesystem::path path_to_helper_cpp = abs_path / "helpers" / "adl_helpers.cc";
    std::filesystem::path path_to_helper_py = abs_path / "helpers";

    // import our needed TIMBER and other python functions
    emit("from TIMBER.Analyzer import *");
    emit("from TIMBER.Tools.Common import *");
    emit("import ROOT");
    emit("import sys, os, re");
        
    // add the path to the python helper file as an import, regardless of its location
    emit("adl_help_dir = os.path.abspath('", path_to_helper_py.string(), "')");
    emit("if adl_help_dir not in sys.path:");
    emit("    sys.path.append(adl_help_dir)");

    // import all our needed python helper functions
    emit("from adl_helpers import combine_without_duplicates, use_histo, use_histo_list");
        
    // compile the cpp helper functions into this
    emit("CompileCpp('", path_to_helper_cpp.string(), "')");
        
    // open up the input file and an output file
    emit("a = analyzer('", in_file, "')\nout = ROOT.TFile.Open('", out_file, "','UPDATE')");

    emit_newline();

    // predefine MET to have the requisite variables to be a Lorentz vector
    emit("a.Define('METV", attribute_delimiter, "pt','RVec<float> {", met_name, attribute_delimiter, "pt}')");
    emit("a.Define('METV", attribute_delimiter, "phi','RVec<float> {", met_name, attribute_delimiter, "phi}')");

    met_name.clear();
    met_name = "METV";

    // a trick to get the eta and m to be an arraay of zeros in the right shape
    emit("a.Define('", met_name, attribute_delimiter, "eta','", met_name, attribute_delimiter, "pt - ", met_name, attribute_delimiter, "pt')");
    emit("a.Define('", met_name, attribute_delimiter, "mass', '", met_name, attribute_delimiter, "eta')");

    ALILCollection &commands = alil->get_commands();
    for (auto &command : commands.get_commands()) {
        handle_command(command);
    }

    emit_newline();
    emit("out.Close()");

}