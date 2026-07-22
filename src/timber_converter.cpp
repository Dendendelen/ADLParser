#include "timber_converter.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "exceptions.hpp"
#include <filesystem>
#include <ostream>
#include <regex>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>


void add_mapping(std::string source, std::string dest) {}
std::string get_mapping(std::string in) {}

std::string multi_arg_function(std::string func_name, int num_args, AnalysisCommand &command, std::string ending_tok = "") {
    std::stringstream func;
    func << func_name << "(";
    bool is_first = true;
    for (int i = 0; i < num_args; i++) {
        if (is_first) {is_first = false;}
        else {func << ",";}

        func << get_mapping(command.get_source_argument(i));
    }

    func << ")" << ending_tok;

    add_mapping(command.get_dest_argument(), func.str());
    return "";
}

std::string binary_infix_operation(std::string op_name, AnalysisCommand &command) {
    std::stringstream computation;
    computation << "(" << get_mapping(command.get_source_argument(0));
    computation << op_name;
    computation << get_mapping(command.get_source_argument(1)) << ")";

    add_mapping(command.get_dest_argument(), computation.str());
    return "";
}

std::string interval(std::string left_bound_op, std::string right_bound_op, AnalysisCommand &command) {
    std::stringstream within;
    within << "(";
    within << "(" << get_mapping(command.get_source_argument(0)) << left_bound_op << get_mapping(command.get_source_argument(1)) << ")";
    within << "&&";
    within << "(" << get_mapping(command.get_source_argument(0)) << right_bound_op << get_mapping(command.get_source_argument(2)) << ")";
    within << ")";

    add_mapping(command.get_dest_argument(), within.str());
    return "";
}

std::string TimberConverter::convert_conversion_error(AnalysisCommand command) {
}
std::string TimberConverter::convert_create_empty_info_list(AnalysisCommand command) {

}
std::string TimberConverter::convert_add_to_info_list(AnalysisCommand command) {}
std::string TimberConverter::convert_display_info(AnalysisCommand command) {}
std::string TimberConverter::convert_create_region(AnalysisCommand command) {}
std::string TimberConverter::convert_merge_regions(AnalysisCommand command) {}
std::string TimberConverter::convert_cut_region(AnalysisCommand command) {}
std::string TimberConverter::convert_create_bin_of_region(AnalysisCommand command) {}
std::string TimberConverter::convert_add_alias(AnalysisCommand command) {}
std::string TimberConverter::convert_add_external(AnalysisCommand command) {}
std::string TimberConverter::convert_add_extern_attr(AnalysisCommand command) {}
std::string TimberConverter::convert_add_correctionlib(AnalysisCommand command) {}
std::string TimberConverter::convert_create_mask(AnalysisCommand command) {}
std::string TimberConverter::convert_limit_mask(AnalysisCommand command) {}
std::string TimberConverter::convert_apply_mask(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_hist_list(AnalysisCommand command) {}
std::string TimberConverter::convert_add_hist_to_list(AnalysisCommand command) {}
std::string TimberConverter::convert_use_hist(AnalysisCommand command) {}
std::string TimberConverter::convert_use_hist_list(AnalysisCommand command) {}
std::string TimberConverter::convert_hist_1d(AnalysisCommand command) {}
std::string TimberConverter::convert_hist_2d(AnalysisCommand command) {}
std::string TimberConverter::convert_weight_apply(AnalysisCommand command) {}
std::string TimberConverter::convert_do_cutflow_on_region(AnalysisCommand command) {}
std::string TimberConverter::convert_do_eventlist_on_regin(AnalysisCommand command) {}
std::string TimberConverter::convert_create_table(AnalysisCommand command) {}
std::string TimberConverter::convert_create_table_errored_value(AnalysisCommand command) {}
std::string TimberConverter::convert_create_table_value(AnalysisCommand command) {}
std::string TimberConverter::convert_create_table_lower_bounds(AnalysisCommand command) {}
std::string TimberConverter::convert_create_table_upper_bounds(AnalysisCommand command) {}
std::string TimberConverter::convert_append_to_table(AnalysisCommand command) {}
std::string TimberConverter::convert_finish_table(AnalysisCommand command) {}
std::string TimberConverter::convert_obj_sort_ascend(AnalysisCommand command) {}
std::string TimberConverter::convert_obj_sort_descend(AnalysisCommand command) {}
std::string TimberConverter::convert_expr_raise(AnalysisCommand command) {
    return multi_arg_function("raise_power", 2, command);
}
std::string TimberConverter::convert_expr_multiply(AnalysisCommand command) {
    return binary_infix_operation("*", command);
}
std::string TimberConverter::convert_expr_divide(AnalysisCommand command) {
    return binary_infix_operation("/", command);
}
std::string TimberConverter::convert_expr_add(AnalysisCommand command) {
    return binary_infix_operation("+", command);
}
std::string TimberConverter::convert_expr_subtract(AnalysisCommand command) {
    return binary_infix_operation("-", command);
}
std::string TimberConverter::convert_expr_lt(AnalysisCommand command) {
    return binary_infix_operation("<", command);
}
std::string TimberConverter::convert_expr_le(AnalysisCommand command) {
    return binary_infix_operation("<=", command);
}
std::string TimberConverter::convert_expr_gt(AnalysisCommand command) {
    return binary_infix_operation(">", command);
}
std::string TimberConverter::convert_expr_ge(AnalysisCommand command) {
    return binary_infix_operation(">=", command);
}
std::string TimberConverter::convert_expr_eq(AnalysisCommand command) {
    return binary_infix_operation("==", command);
}
std::string TimberConverter::convert_expr_ne(AnalysisCommand command) {
    return binary_infix_operation("!=", command);
}
std::string TimberConverter::convert_expr_bitwise_and(AnalysisCommand command) {
    return binary_infix_operation("&", command);
}
std::string TimberConverter::convert_expr_bitwise_or(AnalysisCommand command) {
    return binary_infix_operation("|", command);
}
std::string TimberConverter::convert_expr_and(AnalysisCommand command) {
    return binary_infix_operation("&&", command);
}
std::string TimberConverter::convert_expr_or(AnalysisCommand command) {
    return binary_infix_operation("||", command);
}
std::string TimberConverter::convert_expr_within(AnalysisCommand command) {
    return interval(">=", "<=", command);
}
std::string TimberConverter::convert_expr_within_exclusive(AnalysisCommand command) {
    return interval(">", "<", command);
}
std::string TimberConverter::convert_expr_within_left_exclusive(AnalysisCommand command) {
    return interval(">", "<=", command);
}
std ::string TimberConverter::convert_expr_within_right_exclusive(AnalysisCommand command) {
    return interval(">=", "<", command);
}

std::string TimberConverter::convert_expr_negate(AnalysisCommand command) {
    return multi_arg_function("-", 1, command);
}
std::string TimberConverter::convert_expr_logical_not(AnalysisCommand command) {
    return multi_arg_function("!", 1, command);
}
std::string TimberConverter::convert_expr_if_ternary(AnalysisCommand command) {}
std::string TimberConverter::convert_expr_index(AnalysisCommand command) {}
std::string TimberConverter::convert_expr_index_range(AnalysisCommand command) {}
std::string TimberConverter::convert_expr_index_until(AnalysisCommand command) {}
std::string TimberConverter::convert_expr_index_from(AnalysisCommand command) {}
std::string TimberConverter::convert_func_charge(AnalysisCommand command) {}
std::string TimberConverter::convert_func_pt(AnalysisCommand command) {}
std::string TimberConverter::convert_func_eta(AnalysisCommand command) {}
std::string TimberConverter::convert_func_phi(AnalysisCommand command) {}
std::string TimberConverter::convert_func_mass(AnalysisCommand command) {}
std::string TimberConverter::convert_func_energy(AnalysisCommand command) {}
std::string TimberConverter::convert_func_distinct(AnalysisCommand command) {}
std::string TimberConverter::convert_func_dr(AnalysisCommand command) {}
std::string TimberConverter::convert_func_dphi(AnalysisCommand command) {}
std::string TimberConverter::convert_func_deta(AnalysisCommand command) {}
std::string TimberConverter::convert_func_dr_hadamard(AnalysisCommand command) {}
std::string TimberConverter::convert_func_dphi_hadamard(AnalysisCommand command) {}
std::string TimberConverter::convert_func_deta_hadamard(AnalysisCommand command) {}
std::string TimberConverter::convert_func_size(AnalysisCommand command) {}
std::string TimberConverter::convert_func_anyof(AnalysisCommand command) {}
std::string TimberConverter::convert_func_allof(AnalysisCommand command) {}
std::string TimberConverter::convert_func_sqrt(AnalysisCommand command) {
    return multi_arg_function("sqrt", 1, command);
}
std::string TimberConverter::convert_func_abs(AnalysisCommand command) {
    return multi_arg_function("abs", 1, command);
}
std::string TimberConverter::convert_func_cos(AnalysisCommand command) {
    return multi_arg_function("cos", 1, command);
}
std::string TimberConverter::convert_func_sin(AnalysisCommand command) {
    return multi_arg_function("sin", 1, command);
}
std::string TimberConverter::convert_func_tan(AnalysisCommand command) {
    return multi_arg_function("tan", 1, command);
}
std::string TimberConverter::convert_func_sinh(AnalysisCommand command) {
    return multi_arg_function("sinh", 1, command);
}
std::string TimberConverter::convert_func_cosh(AnalysisCommand command) {
    return multi_arg_function("cosh", 1, command);
}
std::string TimberConverter::convert_func_tanh(AnalysisCommand command) {
    return multi_arg_function("tanh", 1, command);
}
std::string TimberConverter::convert_func_exp(AnalysisCommand command) {
    return multi_arg_function("exp", 1, command);
}
std::string TimberConverter::convert_func_log(AnalysisCommand command) {
    return multi_arg_function("log", 1, command);
}
std::string TimberConverter::convert_func_ave(AnalysisCommand command) {
    
}
std::string TimberConverter::convert_func_sum(AnalysisCommand command) {
    
}
std::string TimberConverter::convert_func_min_of_pair(AnalysisCommand command) {
    return multi_arg_function("std::min", 2, command);
}
std::string TimberConverter::convert_func_max_of_pair(AnalysisCommand command) {
    return multi_arg_function("std::max", 2, command);
}
std::string TimberConverter::convert_func_min_of_list(AnalysisCommand command) {
    return multi_arg_function("ROOT::VecOps::Min", 1, command);
}
std::string TimberConverter::convert_func_max_of_list(AnalysisCommand command) {
    return multi_arg_function("ROOT::VecOps::Max", 1, command);
}
std::string TimberConverter::convert_func_sort_ascend(AnalysisCommand command) {
    return multi_arg_function("ROOT::VecOps::Sort", 1, command);
}
std::string TimberConverter::convert_func_sort_descend(AnalysisCommand command) {
    return multi_arg_function("ROOT::VecOps::Reverse(ROOT::VecOps::Sort", 1, command, ")");
}
std::string TimberConverter::convert_func_named(AnalysisCommand command) {
    return multi_arg_function(get_mapping(command.get_source_argument(1)), 1, command);
}
std::string TimberConverter::convert_create_empty_value_list(AnalysisCommand command) {}
std::string TimberConverter::convert_add_value_to_list(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_union(AnalysisCommand command) {}
std::string TimberConverter::convert_add_part_to_union(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_cartesian(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_disjoint(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_direct(AnalysisCommand command) {}
std::string TimberConverter::convert_add_part_to_composite(AnalysisCommand command) {}
std::string TimberConverter::convert_name_element_of_composite(AnalysisCommand command) {}
std::string TimberConverter::convert_create_empty_particle(AnalysisCommand command) {}
std::string TimberConverter::convert_add_particle(AnalysisCommand command) {}
std::string TimberConverter::convert_sub_particle(AnalysisCommand command) {}

void TimberConverter::print() {

}