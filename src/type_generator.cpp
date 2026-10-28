#include "type_generator.hpp"
#include "alil.hpp"
#include "type.hpp"
#include "type_checker.hpp"

PType TypeGenerator::convert_conversion_error(const AnalysisCommand &) {
    return std::make_shared<Type>(TYPE::ERROR);
}
PType TypeGenerator::convert_create_empty_info_list(const AnalysisCommand &) {
    // () -> List<String>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE::LIST);
    dest_list->add_dest_type(TYPE::STRING);
    fun->add_dest_type(dest_list);
    return fun;
}
PType TypeGenerator::convert_add_to_info_list(const AnalysisCommand &) {
    // List<String> x String x String -> List<String>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::STRING);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE::STRING);
    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(list_type);
    return fun;
}
PType TypeGenerator::convert_display_info(const AnalysisCommand &) {
    // List<String> -> Error
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::STRING);
    fun->add_source_type(list_type);
    fun->add_dest_type(TYPE::ERROR);
    return fun;
}
PType TypeGenerator::convert_create_region(const AnalysisCommand &) {
    // () -> Region
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_dest_type(TYPE::REGION);
    return fun;
}

PType TypeGenerator::convert_merge_regions(const AnalysisCommand &) {
    // Region x Region -> Region
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_source_type(TYPE::REGION);
    fun->add_dest_type(TYPE::REGION);
    return fun;
}


PType TypeGenerator::convert_cut_region(const AnalysisCommand &) {
    // Region x Cond -> Region
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_source_type(TYPE::COND);
    fun->add_dest_type(TYPE::REGION);
    return fun;
}

PType TypeGenerator::convert_create_bin_of_region(const AnalysisCommand &) {
    // Region x Cond -> Region
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_source_type(TYPE::COND);
    fun->add_dest_type(TYPE::REGION);
    return fun;
}

PType TypeGenerator::convert_add_alias(const AnalysisCommand &) {
    // `a -> `a
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);
    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);
    return fun;
}

PType TypeGenerator::convert_add_external(const AnalysisCommand &) {
    // String -> `a
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(TYPE::GENERIC);
    return fun;
}

PType TypeGenerator::convert_add_extern_attr(const AnalysisCommand &) {
    // String -> (`a -> `b) | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto dest_fun = std::make_shared<Type>(TYPE::FUNCTION);
    auto source_of_dest_fun = std::make_shared<Type>(TYPE::GENERIC);
    dest_fun->add_source_type(source_of_dest_fun);
    dest_fun->add_dest_type(TYPE::GENERIC);

    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(dest_fun);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_of_dest_fun, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);
    
    return fun;
}

PType TypeGenerator::convert_add_extern_particle(const AnalysisCommand &) {
    // String -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE::LIST);
    dest_list->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(dest_list);
    return fun;
}

PType TypeGenerator::convert_add_correctionlib(const AnalysisCommand &) {
    // String x String -> (List<Number> -> Number)

    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_fun = std::make_shared<Type>(TYPE::FUNCTION);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(TYPE::NUMBER);

    dest_fun->add_source_type(source_list);
    dest_fun->add_dest_type(TYPE::NUMBER);

    fun->add_source_type(TYPE::STRING);
    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(dest_fun);

    return fun;
}

PType TypeGenerator::convert_create_mask(const AnalysisCommand &) {
    // List<ParticleInstance> -> Mask
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE::LIST);
    source_part_list->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(source_part_list);
    fun->add_dest_type(TYPE::MASK);
    return fun;
}

PType TypeGenerator::convert_limit_mask(const AnalysisCommand &) {
    // Mask x List<Cond> -> Mask
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::MASK);
    auto source_cond_list = std::make_shared<Type>(TYPE::LIST);
    source_cond_list->add_dest_type(TYPE::COND);
    fun->add_source_type(source_cond_list);
    fun->add_dest_type(TYPE::MASK);
    return fun;
}

PType TypeGenerator::convert_apply_mask(const AnalysisCommand &) {
    // Mask x List<ParticleInstance> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::MASK);
    auto source_part_list = std::make_shared<Type>(TYPE::LIST);
    source_part_list->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(source_part_list);
    fun->add_dest_type(source_part_list);
    return fun;
}
PType TypeGenerator::convert_create_empty_hist_list(const AnalysisCommand &) {
    // () -> List<Hist>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE::LIST);
    dest_list->add_dest_type(TYPE::HIST);
    fun->add_dest_type(dest_list);
    return fun;
}

PType TypeGenerator::convert_add_hist_to_list(const AnalysisCommand &) {
    // List<Hist> x Hist -> List<Hist>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::HIST);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE::HIST);
    fun->add_dest_type(list_type);
    return fun;
}

PType TypeGenerator::convert_use_hist(const AnalysisCommand &) {
    // Hist x Region -> Error
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::HIST);
    fun->add_source_type(TYPE::REGION);
    fun->add_dest_type(TYPE::ERROR);
    return fun;
}

PType TypeGenerator::convert_use_hist_list(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::STRING);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::HIST);
    return fun;
}

PType TypeGenerator::convert_hist_1d(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType src_list(std::make_shared<Type>(TYPE::LIST));
    src_list->add_dest_type(TYPE::STRING);

    fun->add_source_type(src_list);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::HIST);
    return fun;
}

PType TypeGenerator::convert_hist_2d(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType src_list(std::make_shared<Type>(TYPE::LIST));
    src_list->add_dest_type(TYPE::STRING);

    fun->add_source_type(src_list);    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::HIST);
    return fun;
}

PType TypeGenerator::convert_weight_apply(const AnalysisCommand &) {
    // Region x String x Number -> Region


    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_source_type(TYPE::STRING);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::REGION);
    return fun;
}

PType TypeGenerator::convert_do_cutflow_on_region(const AnalysisCommand &) {
    // Region -> Error
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_dest_type(TYPE::ERROR);
    return fun;
}

PType TypeGenerator::convert_do_eventlist_on_region(const AnalysisCommand &) {
    // Region -> Error
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::REGION);
    fun->add_dest_type(TYPE::ERROR);
    return fun;
}

PType TypeGenerator::convert_create_table(const AnalysisCommand &) {
    // () -> Table
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_dest_type(TYPE::TABLE);
    return fun;
}

PType TypeGenerator::convert_create_table_errored_value(const AnalysisCommand &) {
    // Number x Number x Number -> ErroredNumber
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::ERRORED_NUMBER);
    return fun;
}

PType TypeGenerator::convert_create_table_value(const AnalysisCommand &) {
    // Number -> ErroredNumber
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(TYPE::ERRORED_NUMBER);
    return fun;
}

PType TypeGenerator::convert_append_to_table(const AnalysisCommand &) {
    // Table x ErroredNumber x List<Number> x List<Number> -> Table
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::TABLE);
    fun->add_source_type(TYPE::ERRORED_NUMBER);

    PType src_list(std::make_shared<Type>(TYPE::LIST));
    src_list->add_dest_type(TYPE::NUMBER);

    fun->add_source_type(src_list);
    fun->add_source_type(src_list);

    fun->add_dest_type(TYPE::TABLE);

    return fun;

}

PType TypeGenerator::convert_finish_table(const AnalysisCommand &) {
    // Table -> Fun (List<Number> -> ErroredNumber)
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::TABLE);

    PType dest_list(std::make_shared<Type>(TYPE::LIST));
    dest_list->add_dest_type(TYPE::NUMBER);

    PType dest_fun(std::make_shared<Type>(TYPE::FUNCTION));
    dest_fun->add_source_type(dest_list);
    dest_fun->add_dest_type(TYPE::ERRORED_NUMBER);

    fun->add_dest_type(dest_fun);

    return fun;
}

PType TypeGenerator::convert_obj_sort_ascend(const AnalysisCommand &) {
    // List<ParticleInstance> x List<Number> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE::LIST);
    source_part_list->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(source_part_list);

    auto source_number_list = std::make_shared<Type>(TYPE::LIST);
    source_number_list->add_dest_type(TYPE::NUMBER);
    fun->add_source_type(source_number_list);

    fun->add_dest_type(source_part_list);
    return fun;
}

PType TypeGenerator::convert_obj_sort_descend(const AnalysisCommand &) {
    // List<ParticleInstance> x List<Number> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE::LIST);
    source_part_list->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(source_part_list);

    auto source_number_list = std::make_shared<Type>(TYPE::LIST);
    source_number_list->add_dest_type(TYPE::NUMBER);
    fun->add_source_type(source_number_list);

    fun->add_dest_type(source_part_list);
    return fun;
}

PType TypeGenerator::convert_expr_raise(const AnalysisCommand &) {
    // `a x Number -> `a | `a <<: Number
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);
    
    return fun;
}

// Helper function for binary numeric operations
static PType create_binary_numeric_op() {
    // `a x `b -> `c | `a <<: number, `b  <<: number,
    // a ~= b <==> a ~= c and b ~= c
    // b ~= c and a ~/= c <==> a is list-free and b is list-structured
    // a ~= c and b ~/= c <==> a is list-structured and b is list-free
    // c = Err <==> a is list-structured and b is list-structured and a ~/= b
    // c </: Err <==> c <<: number 
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint forward_equality(fun);
    forward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(forward_equality);

    Constraint backward_equality(fun);
    backward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(backward_equality);

    Constraint a_free_forward(fun);
    a_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_forward); 

    Constraint a_free_backward(fun);
    a_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_backward); 

    Constraint b_free_forward(fun);
    b_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_forward); 

    Constraint b_free_backward(fun);
    b_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_backward); 

    Constraint error_forward(fun);
    error_forward.add_premise(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_forward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_forward); 

    Constraint error_backward(fun);
    error_backward.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_backward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_backward);
    
    Constraint validity_forward(fun);
    validity_forward.add_premise(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_forward.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(validity_forward);

    Constraint validity_backward(fun);
    validity_backward.add_conclusion(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_backward.add_premise(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(validity_backward);

    // Constraint primary_secondary_equality(fun);
    // primary_secondary_equality.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, source_type_b));
    // primary_secondary_equality.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_b));
    // fun->add_constraint(primary_secondary_equality);

    // Constraint primary_secondary_equality_2(fun);
    // primary_secondary_equality_2.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, source_type_a));
    // primary_secondary_equality_2.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_a));
    // fun->add_constraint(primary_secondary_equality_2);

    // Constraint primary_single_number(fun);
    // primary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // primary_single_number.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_b));
    // fun->add_constraint(primary_single_number);

    // Constraint secondary_single_number(fun);
    // secondary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // secondary_single_number.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_a));
    // fun->add_constraint(secondary_single_number);

    // Constraint error_condition(fun);
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, source_type_b));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, source_type_a));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // error_condition.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition);

    return fun;
}

PType TypeGenerator::convert_expr_multiply(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType TypeGenerator::convert_expr_divide(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType TypeGenerator::convert_expr_add(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType TypeGenerator::convert_expr_subtract(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType TypeGenerator::convert_expr_bitwise_and(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType TypeGenerator::convert_expr_bitwise_or(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

// Helper function for comparison operations
static PType create_comparison_op() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint forward_equality(fun);
    forward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(forward_equality);

    Constraint backward_equality(fun);
    backward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(backward_equality);

    Constraint a_free_forward(fun);
    a_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_forward); 

    Constraint a_free_backward(fun);
    a_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_backward); 

    Constraint b_free_forward(fun);
    b_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_forward); 

    Constraint b_free_backward(fun);
    b_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_backward); 

    Constraint error_forward(fun);
    error_forward.add_premise(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_forward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_forward); 

    Constraint error_backward(fun);
    error_backward.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_backward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_backward);
    
    Constraint validity_forward(fun);
    validity_forward.add_premise(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_forward.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_forward);

    Constraint validity_backward(fun);
    validity_backward.add_conclusion(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_backward.add_premise(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_backward);

    // Constraint numeric(fun);
    // numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // fun->add_constraint(numeric);

    // Constraint condition(fun);
    // condition.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    // fun->add_constraint(condition);

    // Constraint primary_secondary_equality(fun);
    // primary_secondary_equality.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, source_type_b));
    // primary_secondary_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_b));
    // fun->add_constraint(primary_secondary_equality);

    // Constraint primary_secondary_equality_2(fun);
    // primary_secondary_equality_2.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, source_type_a));
    // primary_secondary_equality_2.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_a));
    // fun->add_constraint(primary_secondary_equality_2);

    // Constraint primary_single_number(fun);
    // primary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // primary_single_number.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_b));
    // fun->add_constraint(primary_single_number);

    // Constraint secondary_single_number(fun);
    // secondary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // secondary_single_number.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_a));
    // fun->add_constraint(secondary_single_number);

    // Constraint error_condition(fun);
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, source_type_b));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, source_type_a));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // error_condition.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition);

    return fun;
}

PType TypeGenerator::convert_expr_lt(const AnalysisCommand &) {
    return create_comparison_op();
}

PType TypeGenerator::convert_expr_le(const AnalysisCommand &) {
    return create_comparison_op();
}

PType TypeGenerator::convert_expr_gt(const AnalysisCommand &) {
    return create_comparison_op();
}

PType TypeGenerator::convert_expr_ge(const AnalysisCommand &) {
    return create_comparison_op();
}

PType TypeGenerator::convert_expr_eq(const AnalysisCommand &) {
    return create_comparison_op();
}

PType TypeGenerator::convert_expr_ne(const AnalysisCommand &) {
    return create_comparison_op();
}

// Helper function for logical operations
static PType create_logical_op() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::COND)));
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(numeric);

    Constraint forward_equality(fun);
    forward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(forward_equality);

    Constraint backward_equality(fun);
    backward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(backward_equality);

    Constraint a_free_forward(fun);
    a_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_forward); 

    Constraint a_free_backward(fun);
    a_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_backward); 

    Constraint b_free_forward(fun);
    b_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_forward); 

    Constraint b_free_backward(fun);
    b_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_backward); 

    Constraint error_forward(fun);
    error_forward.add_premise(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_forward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_forward); 

    Constraint error_backward(fun);
    error_backward.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_backward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_backward);
    
    Constraint validity_forward(fun);
    validity_forward.add_premise(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_forward.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_forward);

    Constraint validity_backward(fun);
    validity_backward.add_conclusion(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_backward.add_premise(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_backward);

    // Constraint condition(fun);
    // condition.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::COND)));
    // condition.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::COND)));
    // fun->add_constraint(condition);

    // Constraint primary_secondary_equality(fun);
    // primary_secondary_equality.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, source_type_b));
    // primary_secondary_equality.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_b));
    // fun->add_constraint(primary_secondary_equality);

    // Constraint primary_secondary_equality_2(fun);
    // primary_secondary_equality_2.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, source_type_a));
    // primary_secondary_equality_2.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_a));
    // fun->add_constraint(primary_secondary_equality_2);

    // Constraint primary_single_cond(fun);
    // primary_single_cond.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::COND)));
    // primary_single_cond.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_b));
    // fun->add_constraint(primary_single_cond);

    // Constraint secondary_single_cond(fun);
    // secondary_single_cond.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::COND)));
    // secondary_single_cond.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, source_type_a));
    // fun->add_constraint(secondary_single_cond);

    // Constraint error_condition(fun);
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, source_type_b));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, source_type_a));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::COND)));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::COND)));
    // error_condition.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition);

    return fun;
}

PType TypeGenerator::convert_expr_and(const AnalysisCommand &) {
    return create_logical_op();
}

PType TypeGenerator::convert_expr_or(const AnalysisCommand &) {
    return create_logical_op();
}

static PType create_double_input_comparison_op() {
    // `a x `b x `b -> `c 
    //      | `a <<: Number
    //      | `b <<: Number
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint forward_equality(fun);
    forward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(forward_equality);

    Constraint backward_equality(fun);
    backward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(backward_equality);

    Constraint a_free_forward(fun);
    a_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_forward); 

    Constraint a_free_backward(fun);
    a_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_backward); 

    Constraint b_free_forward(fun);
    b_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_forward); 

    Constraint b_free_backward(fun);
    b_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_backward); 

    Constraint error_forward(fun);
    error_forward.add_premise(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_forward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_forward); 

    Constraint error_backward(fun);
    error_backward.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_backward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_backward);
    
    Constraint validity_forward(fun);
    validity_forward.add_premise(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_forward.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_forward);

    Constraint validity_backward(fun);
    validity_backward.add_conclusion(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_backward.add_premise(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(validity_backward);

    // Constraint numeric(fun);
    // numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // fun->add_constraint(numeric);

    // Constraint condition(fun);
    // condition.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::COND)));
    // fun->add_constraint(condition);

    // Constraint primary_secondary_equality(fun);
    // primary_secondary_equality.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, source_type_b));
    // primary_secondary_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_b));
    // fun->add_constraint(primary_secondary_equality);

    // Constraint primary_secondary_equality_2(fun);
    // primary_secondary_equality_2.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, source_type_a));
    // primary_secondary_equality_2.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_a));
    // fun->add_constraint(primary_secondary_equality_2);

    // Constraint primary_single_number(fun);
    // primary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::NUMBER)));
    // primary_single_number.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_b));
    // fun->add_constraint(primary_single_number);

    // Constraint secondary_single_number(fun);
    // secondary_single_number.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::NUMBER)));
    // secondary_single_number.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, dest_type_c, source_type_a));
    // fun->add_constraint(secondary_single_number);

    // Constraint error_condition(fun);
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, source_type_b));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, source_type_a));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::COND)));
    // error_condition.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::COND)));
    // error_condition.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition);

    return fun;
}

PType TypeGenerator::convert_expr_within(const AnalysisCommand &) {
    return create_double_input_comparison_op();
}

PType TypeGenerator::convert_expr_within_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType TypeGenerator::convert_expr_within_left_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType TypeGenerator::convert_expr_within_right_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType TypeGenerator::convert_expr_negate(const AnalysisCommand &) {
    // `a -> `a | `a <<: Number
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);
    
    return fun;
}

PType TypeGenerator::convert_expr_logical_not(const AnalysisCommand &) {
// `a -> `a | `a <<: Cond
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(numeric);
    
    return fun;
}

PType TypeGenerator::convert_expr_if_ternary(const AnalysisCommand &) {
    // Cond x `a x `b -> `c | `a <: `c and `b <: `c

    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    auto branch_a = std::make_shared<Type>(TYPE::GENERIC);
    auto branch_b = std::make_shared<Type>(TYPE::GENERIC);
    auto result = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(TYPE::COND);
    fun->add_source_type(branch_a);
    fun->add_source_type(branch_b);
    fun->add_dest_type(result);

    Constraint branch_a_result(fun);
    branch_a_result.add_conclusion(
        Statement(STATEMENT::SUBTYPE, branch_a, result)
    );
    fun->add_constraint(branch_a_result);

    Constraint branch_b_result(fun);
    branch_b_result.add_conclusion(
        Statement(STATEMENT::SUBTYPE, branch_b, result)
    );
    fun->add_constraint(branch_b_result);

    return fun;
}

PType TypeGenerator::convert_expr_index(const AnalysisCommand &) {
    // List<`a> x Number -> `a
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE::NUMBER));
    fun->add_dest_type(element_type);
    return fun;
}

PType TypeGenerator::convert_expr_index_range(const AnalysisCommand &) {
    // List<`a> x Number x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE::NUMBER));
    fun->add_source_type(Type::fundamental_type_instance(TYPE::NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

PType TypeGenerator::convert_expr_index_until(const AnalysisCommand &) {
    // List<`a> x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE::NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

PType TypeGenerator::convert_expr_index_from(const AnalysisCommand &) {
    // List<`a> x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE::NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

// Helper function for particle -> number functions
static PType create_particle_to_number_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}


PType TypeGenerator::convert_func_charge(const AnalysisCommand &) {
    return create_particle_to_number_func();
}


PType TypeGenerator::convert_func_pt(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType TypeGenerator::convert_func_eta(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType TypeGenerator::convert_func_phi(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType TypeGenerator::convert_func_mass(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType TypeGenerator::convert_func_energy(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

// Helper function for dr/dphi/deta style functions
static PType create_particle_pair_to_number_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::PARTICLE)));
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(numeric);

    Constraint forward_equality(fun);
    forward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    forward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(forward_equality);

    Constraint backward_equality(fun);
    backward_equality.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, source_type_b));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    backward_equality.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(backward_equality);

    Constraint a_free_forward(fun);
    a_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_forward); 

    Constraint a_free_backward(fun);
    a_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, dest_type_c));
    a_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_a, source_type_a));
    a_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(a_free_backward); 

    Constraint b_free_forward(fun);
    b_free_forward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_forward.add_premise(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_forward); 

    Constraint b_free_backward(fun);
    b_free_backward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_b, dest_type_c));
    b_free_backward.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_FREE, source_type_b, source_type_b));
    b_free_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    fun->add_constraint(b_free_backward); 

    Constraint error_forward(fun);
    error_forward.add_premise(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_forward.add_conclusion(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_forward.add_conclusion(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_forward); 

    Constraint error_backward(fun);
    error_backward.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    error_backward.add_premise(Statement(STATEMENT::NOT_EQUAL_DEPTH, source_type_a, source_type_b));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_a, source_type_a));
    error_backward.add_premise(Statement(STATEMENT::IS_LIST_STRUCTURED, source_type_b, source_type_b));
    fun->add_constraint(error_backward);
    
    Constraint validity_forward(fun);
    validity_forward.add_premise(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_forward.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(validity_forward);

    Constraint validity_backward(fun);
    validity_backward.add_conclusion(Statement(STATEMENT::NONSUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    validity_backward.add_premise(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(validity_backward);

    // Constraint particlelike_a(fun);
    // particlelike_a.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // fun->add_constraint(particlelike_a);

    // Constraint particlelike_b(fun);
    // particlelike_b.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // fun->add_constraint(particlelike_b);

    // Constraint numeric_c(fun);
    // numeric_c.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE::NUMBER)));
    // fun->add_constraint(numeric_c);

    // Constraint a_is_single(fun);
    // a_is_single.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // a_is_single.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_b, dest_type_c));
    // fun->add_constraint(a_is_single);

    // Constraint b_is_single(fun);
    // b_is_single.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // b_is_single.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type_a, dest_type_c));
    // fun->add_constraint(b_is_single);

    // auto list_particle_instance = std::make_shared<Type>(TYPE::LIST);
    // list_particle_instance->add_dest_type(TYPE::PARTICLE);

    // auto list_list_number = std::make_shared<Type>(TYPE::LIST);
    // auto inner_list_number = std::make_shared<Type>(TYPE::LIST);
    // inner_list_number->add_dest_type(TYPE::NUMBER);
    // list_list_number->add_dest_type(inner_list_number);

    // Constraint both_lists(fun);
    // both_lists.add_premise(Statement(STATEMENT::SUBTYPE, source_type_a, list_particle_instance));
    // both_lists.add_premise(Statement(STATEMENT::SUBTYPE, source_type_b, list_particle_instance));
    // both_lists.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, list_list_number));
    // fun->add_constraint(both_lists);

    
    // Constraint error_condition_a(fun);
    // error_condition_a.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // error_condition_a.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // error_condition_a.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, list_particle_instance));
    // error_condition_a.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition_a);

    // Constraint error_condition_b(fun);
    // error_condition_b.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // error_condition_b.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE::PARTICLE)));
    // error_condition_b.add_premise(Statement(STATEMENT::NONSUBTYPE, source_type_b, list_particle_instance));
    // error_condition_b.add_conclusion(Statement(STATEMENT::EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE::ERROR)));
    // fun->add_constraint(error_condition_b);

    return fun;
}

PType TypeGenerator::convert_func_distinct(const AnalysisCommand &) {
    // `a x `a -> `b | `a <<: ParticleInstance, `b <<: Number, `a ~=~ `b
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}

PType TypeGenerator::convert_func_dr(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

PType TypeGenerator::convert_func_dphi(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

PType TypeGenerator::convert_func_deta(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

// Helper for hadamard versions
static PType create_hadamard_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT::EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}

PType TypeGenerator::convert_func_dr_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType TypeGenerator::convert_func_dphi_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType TypeGenerator::convert_func_deta_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType TypeGenerator::convert_func_size(const AnalysisCommand &) {
    // List<`a> -> Number
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(TYPE::GENERIC);
    fun->add_source_type(source_list);
    fun->add_dest_type(TYPE::NUMBER);
    return fun;
}

static PType list_reducer_func() {
    // List<`a> -> `a | `a <<: Cond
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);
    source_list->add_dest_type(source_type);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_type);

    Constraint boolean(fun);
    boolean.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::COND)));
    fun->add_constraint(boolean);
    
    return fun;
}

PType TypeGenerator::convert_func_anyof(const AnalysisCommand &) {
    return list_reducer_func();
}

PType TypeGenerator::convert_func_allof(const AnalysisCommand &) {
    return list_reducer_func();
}

// Helper for unary numeric functions
static PType create_unary_numeric_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType TypeGenerator::convert_func_sqrt(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_abs(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_cos(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_sin(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_tan(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_sinh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_cosh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_tanh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_exp(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType TypeGenerator::convert_func_log(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

// Helper for list reduction functions (ave, sum, min, max)
static PType create_list_reduction_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    auto source_list = std::make_shared<Type>(TYPE::LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_dest_type(element_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType TypeGenerator::convert_func_ave(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType TypeGenerator::convert_func_sum(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType TypeGenerator::convert_func_min_of_list(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType TypeGenerator::convert_func_max_of_list(const AnalysisCommand &) {
    return create_list_reduction_func();
}

// Helper for pair min/max functions
static PType create_pair_minmax_func() {
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE::GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE::NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType TypeGenerator::convert_func_min_of_pair(const AnalysisCommand &) {
    return create_pair_minmax_func();
}

PType TypeGenerator::convert_func_max_of_pair(const AnalysisCommand &) {
    return create_pair_minmax_func();
}

PType TypeGenerator::convert_func_sort_ascend(const AnalysisCommand &) {
    // List<Number> -> List<Number>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType source_list(std::make_shared<Type>(TYPE::LIST));
    source_list->add_dest_type(TYPE::NUMBER);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_list);

    return fun;
}

PType TypeGenerator::convert_func_sort_descend(const AnalysisCommand &) {
    // List<Number> -> List<Number>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType source_list(std::make_shared<Type>(TYPE::LIST));
    source_list->add_dest_type(TYPE::NUMBER);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_list);

    return fun;
}

PType TypeGenerator::convert_func_named(const AnalysisCommand &) {
    // `a x (`b->`c) -> `c | `a <: `b
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE::GENERIC);
    auto type_b = std::make_shared<Type>(TYPE::GENERIC);
    auto type_c = std::make_shared<Type>(TYPE::GENERIC);

    auto func_type = std::make_shared<Type>(TYPE::FUNCTION);
    func_type->add_source_type(type_b);
    func_type->add_dest_type(type_c);

    fun->add_source_type(source_type_a);
    fun->add_source_type(func_type);
    fun->add_dest_type(type_c);

    Constraint subtype(fun);
    subtype.add_conclusion(Statement(STATEMENT::SUBTYPE, source_type_a, type_b));
    fun->add_constraint(subtype);

    // Constraint implication_of_origin(fun);
    // implication_of_origin.add_premise(Statement(STATEMENT::EQUALITY, source_type_a, type_b));
    // implication_of_origin.add_conclusion(Statement(STATEMENT::EQUALITY, type_c, dest_type_d));
    // fun->add_constraint(implication_of_origin);

    return fun;
}

PType TypeGenerator::convert_create_empty_string_list(const AnalysisCommand &) {
    // () -> List<String>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE::LIST);
    dest_list->add_dest_type(TYPE::STRING);
    fun->add_dest_type(dest_list);
    return fun;
}

PType TypeGenerator::convert_add_string_to_list(const AnalysisCommand &) {
    // List<String> x String -> List<String>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::STRING);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE::STRING);
    fun->add_dest_type(list_type);
    return fun;
}

PType TypeGenerator::convert_create_empty_value_list(const AnalysisCommand &) {
    // () -> List<Number>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE::LIST);
    dest_list->add_dest_type(TYPE::NUMBER);
    fun->add_dest_type(dest_list);
    return fun;
}

PType TypeGenerator::convert_add_value_to_list(const AnalysisCommand &) {
    // List<Number> x Number -> List<Number>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::NUMBER);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_dest_type(list_type);
    return fun;
}

PType TypeGenerator::convert_create_empty_union(const AnalysisCommand &) {
    // () -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType list(std::make_shared<Type>(TYPE::LIST));
    list->add_dest_type(TYPE::PARTICLE);

    fun->add_dest_type(list);
    return fun;
}

PType TypeGenerator::convert_add_part_to_union(const AnalysisCommand &) {
    // List<ParticleInstance> x List<ParticleInstance> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));

    PType list(std::make_shared<Type>(TYPE::LIST));
    list->add_dest_type(TYPE::PARTICLE);

    fun->add_source_type(list);
    fun->add_source_type(list);
    fun->add_dest_type(list);
    return fun;
}

PType TypeGenerator::convert_create_empty_cartesian(const AnalysisCommand &) {
    // () -> Comb
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_dest_type(TYPE::COMB);
    return fun;
}

PType TypeGenerator::convert_create_empty_disjoint(const AnalysisCommand &) {
    // () -> Disjoint
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_dest_type(TYPE::COMB);
    return fun;
}

PType TypeGenerator::convert_create_empty_direct(const AnalysisCommand &) {
    // () -> Direct
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_dest_type(TYPE::COMB);
    return fun;
}

PType TypeGenerator::convert_add_part_to_composite(const AnalysisCommand &) {
    // Comb x List<ParticleInstance> -> Comb (or Disjoint -> Disjoint, Direct -> Direct)
    // Using generic composite type
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    fun->add_source_type(TYPE::COMB);
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::PARTICLE);
    fun->add_source_type(list_type);
    fun->add_dest_type(TYPE::COMB);
    return fun;
}

PType TypeGenerator::convert_name_element_of_composite(const AnalysisCommand &) {
    // Comb x Number x List<ParticleInstance> -> List<ParticleInstance>

    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE::LIST);
    list_type->add_dest_type(TYPE::PARTICLE);

    fun->add_source_type(TYPE::COMB);
    fun->add_source_type(TYPE::NUMBER);
    fun->add_source_type(list_type);
    fun->add_dest_type(list_type);
    return fun;
}

PType TypeGenerator::convert_create_empty_particle(const AnalysisCommand &) {
    // () -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    return fun;
}

PType TypeGenerator::convert_add_particle(const AnalysisCommand &) {
    // `a x `a -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    fun->add_source_type(element_type);
    fun->add_source_type(element_type);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    // // List<ParticleInstance> x List<ParticleInstance> -> List<ParticleInstance>
    // PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    // auto element_type = std::make_shared<Type>(TYPE::LIST);
    // element_type->add_dest_type(TYPE::PARTICLE);

    // fun->add_source_type(element_type);
    // fun->add_source_type(element_type);
    // fun->add_dest_type(element_type);

    return fun;
}

PType TypeGenerator::convert_sub_particle(const AnalysisCommand &) {
    // `a x `a -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE::FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE::GENERIC);
    fun->add_source_type(element_type);
    fun->add_source_type(element_type);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT::HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE::PARTICLE)));
    fun->add_constraint(particlelike);

    return fun;
}

PType TypeGenerator::get_type_of_command(const AnalysisCommand &command) {
    return command_convert(command);
}

void TypeGenerator::print() {
    for (auto &command : alil->get_commands()) {
        auto type_of_function = command_convert(command);
        type_of_function->set_origin_token(command.get_token());
        type_of_function->print();
    }
}