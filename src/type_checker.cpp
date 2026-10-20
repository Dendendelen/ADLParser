
#include "type_checker.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "node.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <vector>


std::unordered_map<BaseType, PType> Type::base_type_instances_map;
std::unordered_map<PType, int> Type::generic_map;
int Type::highest_mapped_generic;

Constraint::Constraint(std::weak_ptr<Type> type_in): originator_type(type_in) {}

void Constraint::add_premise(Statement statement) {
    premises.push_back(statement);
}
void Constraint::add_conclusion(Statement statement) {
    conclusions.push_back(statement);
}

std::vector<Statement> &Constraint::get_premises() {
    return premises;
}

std::vector<Statement> &Constraint::get_conclusions() {
    return conclusions;
}

std::weak_ptr<Type> Constraint::get_originator_type() {
    return originator_type;
}

void Constraint::internal_print(bool has_map,std::unordered_map<PType, PType> *map_ptr) {
    bool has_premises = false;
    bool first = true;
    for (auto premise : premises) {
        if (first) {
            first = false;
        } else {
            std::cout << " and ";
        }
        has_map ? premise.print(*map_ptr) : premise.print();
        has_premises = true;
    }
    if (has_premises) {
        std::cout << " ===> ";
    }

    first = true;
    for (auto conclusion : conclusions) {
        if (first) {
            first = false;
        } else {
            std::cout << " and ";
        }
        has_map ? conclusion.print(*map_ptr) : conclusion.print();
    }
}

// void Constraint::print() {
//     bool has_premises = false;
//     bool first = true;
//     for (auto premise : premises) {
//         if (first) {
//             first = false;
//         } else {
//             std::cout << " and ";
//         }
//         premise.print();
//         has_premises = true;
//     }
//     if (has_premises) {
//         std::cout << " ===> ";
//     }

//     first = true;
//     for (auto conclusion : conclusions) {
//         if (first) {
//             first = false;
//         } else {
//             std::cout << " and ";
//         }
//         conclusion.print();
//     }
// }

void Constraint::print() {
    internal_print(false, nullptr);
}

void Constraint::print(std::unordered_map<PType, PType> &substitutions) {
    internal_print(true, &substitutions);
}

Statement::Statement(StatementForm form_in, PType type1, PType type2) : form(form_in), lhs(type1), rhs(type2) {}

PType Type::fundamental_type_instance(BaseType bt) {
    // no non-fundamental-types should have this be called on it - lists, functions always have children, and generic is not a single type
    assert(bt != TYPE_GENERIC);
    assert(bt != TYPE_FUNCTION);
    assert(bt != TYPE_LIST);

    if (base_type_instances_map.count(bt) == 0) {
        base_type_instances_map.emplace(bt, std::make_shared<Type>(bt));
    }
    return base_type_instances_map[bt];
}


PType Statement::get_lhs() {
    return lhs;
}

PType Statement::get_rhs() {
    return rhs;
}

StatementForm Statement::get_form() {
    return form;
}

std::string Statement::get_infix_string() {
    switch (form) {

    case STATEMENT_EQUALITY:
        return " = ";
    case STATEMENT_INEQUALITY:
        return " =/= ";
    case STATEMENT_SUBTYPE:
        return " <: ";
    case STATEMENT_NONSUBTYPE:
        return " </:";
    case STATEMENT_SUPERTYPE:
        return " :> ";
    case STATEMENT_NONSUPERTYPE:
        return ":/>";
    case STATEMENT_HEREDITARY_SUBTYPE:
        return " <<: ";
    case STATEMENT_HEREDITARY_SUPERTYPE:
        return " :>> ";
    case STATEMENT_EQUAL_DEPTH:
        return " ~=~ ";
      break;
    }
}

void Statement::print() {
    lhs->print();
    std::cout << get_infix_string();
    rhs->print();
}

void Statement::print(std::unordered_map<PType, PType> & generic_map) {
    lhs->print(generic_map);
    std::cout << get_infix_string();
    rhs->print(generic_map);
}

Type::Type(BaseType in) : this_type(in), has_dest_type(false) {}

BaseType Type::get_base_type() {
    return this_type;
}

bool Type::is_fundamental_type() {
    return (this_type != TYPE_FUNCTION && this_type != TYPE_GENERIC && this_type != TYPE_LIST);
}


void Type::add_source_type(PType type) {
    assert(this_type == TYPE_FUNCTION);
    source_types.push_back(type);
}

void Type::add_source_type(BaseType type) {
    if (type != TYPE_GENERIC) {
        add_source_type(fundamental_type_instance(type));
    } else {
        // all generics are unique
        add_source_type(std::make_shared<Type>(TYPE_GENERIC));
    }


}

void Type::add_dest_type(PType type) {
    assert(this_type == TYPE_FUNCTION || this_type == TYPE_LIST);
    assert(!has_dest_type);

    dest_type = type;
    has_dest_type = true;
}

void Type::add_dest_type(BaseType type) {
    if (type != TYPE_GENERIC) {
        add_dest_type(fundamental_type_instance(type));
    } else {
        // all generics are unique
        add_dest_type(std::make_shared<Type>(TYPE_GENERIC));
    }
}



void Type::add_constraint(Constraint con) {
    constraints.push_back(con);
}

std::vector<Constraint> &Type::get_constraints() {
    return constraints;
}

int Type::get_num_of_sources() {
    assert(this_type == TYPE_FUNCTION);
    return source_types.size();
}

PType Type::get_source_type(int index) {
    assert(index < source_types.size());
    return source_types[index];
}

PType Type::get_dest_type() {
    return dest_type;
}

PToken Type::get_origin_token() {
    return originator_token;
}

void Type::set_origin_token(PToken tok_in) {
    originator_token = tok_in;
    if (this_type == TYPE_LIST) {
        dest_type->set_origin_token(tok_in);
    } else if (this_type == TYPE_FUNCTION) {
        if (has_dest_type) dest_type->set_origin_token(tok_in);
        for (auto src : source_types) {
            src->set_origin_token(tok_in);
        }
    }
}

std::string Type::get_name_string() {

    switch (this_type) {
        case TYPE_REGION:
            return "region";
        case TYPE_COND:
            return "cond";
        case TYPE_MASK:
            return "mask";
        case TYPE_STRING:
            return "string";
        case TYPE_LIST:
            return "list";
        case TYPE_NUMBER:
            return "number";
        case TYPE_PARTICLEINSTANCE:
            return "particleinstance";
        // case TYPE_UNION:
        //     return "union";
        case TYPE_COMB:
            return "comb";
        case TYPE_DISJOINT:
            return "disjoint";
        case TYPE_HIST:
            return "hist";
        case TYPE_TABLE:
            return "table";
        case TYPE_ERRORED_NUMBER:
            return "errored_number";
        case TYPE_ERROR:
            return "error";
        case TYPE_FUNCTION:
            return "function";
        case TYPE_GENERIC:
            return "generic";
    }
    
}


void Type::print(std::unordered_map<PType, PType> &substitutions) {


    if (substitutions.count(shared_from_this()) != 0 && substitutions[shared_from_this()] != shared_from_this()) {
        substitutions[shared_from_this()]->print(substitutions);
        return;
    }

    if (this_type != TYPE_GENERIC) {
        std::cout << get_name_string();
    } else {
        int this_generic;
        auto this_ptr = shared_from_this();
        if (generic_map.count(this_ptr) == 0) {
            generic_map.emplace(this_ptr, ++Type::highest_mapped_generic);
            this_generic = Type::highest_mapped_generic;
        } else {
            this_generic = generic_map[this_ptr];
        }
        std::cout << "`" << this_generic;
    }
    
    bool first = true;
    if (this_type == TYPE_FUNCTION) {
        std::cout << " ( ";
        for (auto it : source_types) {
            if (!first) {
                std::cout << " x ";
            } else {
                first = false;
            }
            it->print(substitutions);
        }
        std::cout << " -> ";

        if (has_dest_type) {
            dest_type->print(substitutions);
        } else {
            std::cout << " () ";
        }
        std::cout << ")";
    } else if (this_type == TYPE_LIST) {
        std::cout << "<";
        if  (has_dest_type) {
            dest_type->print(substitutions);
        }
        std::cout << ">";
    }

    first = true;    
    for (auto constraint : constraints) {
        if (first) {
            std::cout << " with constraints ";
            first = false;
        } else {
            std::cout << "; ";
        }
        constraint.print();
    }
}

void Type::print() {
    std::unordered_map<PType, PType> dummy_sub_map;
    print(dummy_sub_map); 
}
PType Typer::convert_conversion_error(const AnalysisCommand &) {
    return std::make_shared<Type>(TYPE_ERROR);
}
PType Typer::convert_create_empty_info_list(const AnalysisCommand &) {
    // () -> List<String>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE_LIST);
    dest_list->add_dest_type(TYPE_STRING);
    fun->add_dest_type(dest_list);
    return fun;
}
PType Typer::convert_add_to_info_list(const AnalysisCommand &) {
    // List<String> x String x String -> List<String>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_STRING);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE_STRING);
    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(list_type);
    return fun;
}
PType Typer::convert_display_info(const AnalysisCommand &) {
    // List<String> -> Error
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_STRING);
    fun->add_source_type(list_type);
    fun->add_dest_type(TYPE_ERROR);
    return fun;
}
PType Typer::convert_create_region(const AnalysisCommand &) {
    // () -> Region
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_dest_type(TYPE_REGION);
    return fun;
}

PType Typer::convert_merge_regions(const AnalysisCommand &) {
    // Region x Region -> Region
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_source_type(TYPE_REGION);
    fun->add_dest_type(TYPE_REGION);
    return fun;
}


PType Typer::convert_cut_region(const AnalysisCommand &) {
    // Region x Cond -> Region
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_source_type(TYPE_COND);
    fun->add_dest_type(TYPE_REGION);
    return fun;
}

PType Typer::convert_create_bin_of_region(const AnalysisCommand &) {
    // Region x Cond -> Region
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_source_type(TYPE_COND);
    fun->add_dest_type(TYPE_REGION);
    return fun;
}

PType Typer::convert_add_alias(const AnalysisCommand &) {
    // `a -> `a
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);
    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);
    return fun;
}

PType Typer::convert_add_external(const AnalysisCommand &) {
    // String -> `a
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(TYPE_GENERIC);
    return fun;
}

PType Typer::convert_add_extern_attr(const AnalysisCommand &) {
    // String -> (`a -> `b) | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto dest_fun = std::make_shared<Type>(TYPE_FUNCTION);
    auto source_of_dest_fun = std::make_shared<Type>(TYPE_GENERIC);
    dest_fun->add_source_type(source_of_dest_fun);
    dest_fun->add_dest_type(TYPE_GENERIC);

    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(dest_fun);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_of_dest_fun, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);
    
    return fun;
}

PType Typer::convert_add_extern_particle(const AnalysisCommand &) {
    // String -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE_LIST);
    dest_list->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(dest_list);
    return fun;
}

PType Typer::convert_add_correctionlib(const AnalysisCommand &) {
    // String x String -> (List<Number> -> Number)

    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_fun = std::make_shared<Type>(TYPE_FUNCTION);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(TYPE_NUMBER);

    dest_fun->add_source_type(source_list);
    dest_fun->add_dest_type(TYPE_NUMBER);

    fun->add_source_type(TYPE_STRING);
    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(dest_fun);

    return fun;
}

PType Typer::convert_create_mask(const AnalysisCommand &) {
    // List<ParticleInstance> -> Mask
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE_LIST);
    source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(source_part_list);
    fun->add_dest_type(TYPE_MASK);
    return fun;
}

PType Typer::convert_limit_mask(const AnalysisCommand &) {
    // Mask x List<Cond> -> Mask
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_MASK);
    auto source_cond_list = std::make_shared<Type>(TYPE_LIST);
    source_cond_list->add_dest_type(TYPE_COND);
    fun->add_source_type(source_cond_list);
    fun->add_dest_type(TYPE_MASK);
    return fun;
}

PType Typer::convert_apply_mask(const AnalysisCommand &) {
    // Mask x List<ParticleInstance> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_MASK);
    auto source_part_list = std::make_shared<Type>(TYPE_LIST);
    source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(source_part_list);
    fun->add_dest_type(source_part_list);
    return fun;
}
PType Typer::convert_create_empty_hist_list(const AnalysisCommand &) {
    // () -> List<Hist>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE_LIST);
    dest_list->add_dest_type(TYPE_HIST);
    fun->add_dest_type(dest_list);
    return fun;
}

PType Typer::convert_add_hist_to_list(const AnalysisCommand &) {
    // List<Hist> x Hist -> List<Hist>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_HIST);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE_HIST);
    fun->add_dest_type(list_type);
    return fun;
}

PType Typer::convert_use_hist(const AnalysisCommand &) {
    // Hist x Region -> Error
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_HIST);
    fun->add_source_type(TYPE_REGION);
    fun->add_dest_type(TYPE_ERROR);
    return fun;
}

PType Typer::convert_use_hist_list(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_STRING);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_HIST);
    return fun;
}

PType Typer::convert_hist_1d(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType src_list(std::make_shared<Type>(TYPE_LIST));
    src_list->add_dest_type(TYPE_STRING);

    fun->add_source_type(src_list);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_HIST);
    return fun;
}

PType Typer::convert_hist_2d(const AnalysisCommand &) {
    // List<String> x Number x Number x Number x Number x Number x Number x Number x Number -> Hist
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType src_list(std::make_shared<Type>(TYPE_LIST));
    src_list->add_dest_type(TYPE_STRING);

    fun->add_source_type(src_list);    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_HIST);
    return fun;
}

PType Typer::convert_weight_apply(const AnalysisCommand &) {
    // Region x String x Number -> Region


    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_source_type(TYPE_STRING);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_REGION);
    return fun;
}

PType Typer::convert_do_cutflow_on_region(const AnalysisCommand &) {
    // Region -> Error
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_dest_type(TYPE_ERROR);
    return fun;
}

PType Typer::convert_do_eventlist_on_region(const AnalysisCommand &) {
    // Region -> Error
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_REGION);
    fun->add_dest_type(TYPE_ERROR);
    return fun;
}

PType Typer::convert_create_table(const AnalysisCommand &) {
    // () -> Table
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_dest_type(TYPE_TABLE);
    return fun;
}

PType Typer::convert_create_table_errored_value(const AnalysisCommand &) {
    // Number x Number x Number -> ErroredNumber
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_ERRORED_NUMBER);
    return fun;
}

PType Typer::convert_create_table_value(const AnalysisCommand &) {
    // Number -> ErroredNumber
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(TYPE_ERRORED_NUMBER);
    return fun;
}

PType Typer::convert_append_to_table(const AnalysisCommand &) {
    // Table x ErroredNumber x List<Number> x List<Number> -> Table
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_TABLE);
    fun->add_source_type(TYPE_ERRORED_NUMBER);

    PType src_list(std::make_shared<Type>(TYPE_LIST));
    src_list->add_dest_type(TYPE_NUMBER);

    fun->add_source_type(src_list);
    fun->add_source_type(src_list);

    fun->add_dest_type(TYPE_TABLE);

    return fun;

}

PType Typer::convert_finish_table(const AnalysisCommand &) {
    // Table -> Fun (List<Number> -> ErroredNumber)
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_TABLE);

    PType dest_list(std::make_shared<Type>(TYPE_LIST));
    dest_list->add_dest_type(TYPE_NUMBER);

    PType dest_fun(std::make_shared<Type>(TYPE_FUNCTION));
    dest_fun->add_source_type(dest_list);
    dest_fun->add_dest_type(TYPE_ERRORED_NUMBER);

    fun->add_dest_type(dest_fun);

    return fun;
}

PType Typer::convert_obj_sort_ascend(const AnalysisCommand &) {
    // List<ParticleInstance> x List<Number> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE_LIST);
    source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(source_part_list);

    auto source_number_list = std::make_shared<Type>(TYPE_LIST);
    source_number_list->add_dest_type(TYPE_NUMBER);
    fun->add_source_type(source_number_list);

    fun->add_dest_type(source_part_list);
    return fun;
}

PType Typer::convert_obj_sort_descend(const AnalysisCommand &) {
    // List<ParticleInstance> x List<Number> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_part_list = std::make_shared<Type>(TYPE_LIST);
    source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(source_part_list);

    auto source_number_list = std::make_shared<Type>(TYPE_LIST);
    source_number_list->add_dest_type(TYPE_NUMBER);
    fun->add_source_type(source_number_list);

    fun->add_dest_type(source_part_list);
    return fun;
}

PType Typer::convert_expr_raise(const AnalysisCommand &) {
    // `a x Number -> `a | `a <<: Number
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);
    
    return fun;
}

// Helper function for binary numeric operations
static PType create_binary_numeric_op() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint primary_secondary_equality(fun);
    primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
    primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
    fun->add_constraint(primary_secondary_equality);

    Constraint primary_single_number(fun);
    primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    primary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_b));
    fun->add_constraint(primary_single_number);

    Constraint secondary_single_number(fun);
    secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    secondary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
    fun->add_constraint(secondary_single_number);

    Constraint error_condition(fun);
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition);

    return fun;
}

PType Typer::convert_expr_multiply(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType Typer::convert_expr_divide(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType Typer::convert_expr_add(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType Typer::convert_expr_subtract(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType Typer::convert_expr_bitwise_and(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

PType Typer::convert_expr_bitwise_or(const AnalysisCommand &) {
    return create_binary_numeric_op();
}

// Helper function for comparison operations
static PType create_comparison_op() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint condition(fun);
    condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE_COND)));
    fun->add_constraint(condition);

    Constraint primary_secondary_equality(fun);
    primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
    primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));
    fun->add_constraint(primary_secondary_equality);

    Constraint primary_single_number(fun);
    primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    primary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_b));
    fun->add_constraint(primary_single_number);

    Constraint secondary_single_number(fun);
    secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    secondary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));
    fun->add_constraint(secondary_single_number);

    Constraint error_condition(fun);
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition);

    return fun;
}

PType Typer::convert_expr_lt(const AnalysisCommand &) {
    return create_comparison_op();
}

PType Typer::convert_expr_le(const AnalysisCommand &) {
    return create_comparison_op();
}

PType Typer::convert_expr_gt(const AnalysisCommand &) {
    return create_comparison_op();
}

PType Typer::convert_expr_ge(const AnalysisCommand &) {
    return create_comparison_op();
}

PType Typer::convert_expr_eq(const AnalysisCommand &) {
    return create_comparison_op();
}

PType Typer::convert_expr_ne(const AnalysisCommand &) {
    return create_comparison_op();
}

// Helper function for logical operations
static PType create_logical_op() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint condition(fun);
    condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
    condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_COND)));
    fun->add_constraint(condition);

    Constraint primary_secondary_equality(fun);
    primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
    primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
    fun->add_constraint(primary_secondary_equality);

    Constraint primary_single_cond(fun);
    primary_single_cond.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
    primary_single_cond.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_b));
    fun->add_constraint(primary_single_cond);

    Constraint secondary_single_cond(fun);
    secondary_single_cond.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_COND)));
    secondary_single_cond.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
    fun->add_constraint(secondary_single_cond);

    Constraint error_condition(fun);
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_COND)));
    error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition);

    return fun;
}

PType Typer::convert_expr_and(const AnalysisCommand &) {
    return create_logical_op();
}

PType Typer::convert_expr_or(const AnalysisCommand &) {
    return create_logical_op();
}

static PType create_double_input_comparison_op() {
    // `a x `b x `b -> `c 
    //      | `a <<: Number
    //      | `b <<: Number
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint condition(fun);
    condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE_COND)));
    fun->add_constraint(condition);

    Constraint primary_secondary_equality(fun);
    primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
    primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));
    fun->add_constraint(primary_secondary_equality);

    Constraint primary_single_number(fun);
    primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    primary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_b));
    fun->add_constraint(primary_single_number);

    Constraint secondary_single_number(fun);
    secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    secondary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));
    fun->add_constraint(secondary_single_number);

    Constraint error_condition(fun);
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
    error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition);

    return fun;
}

PType Typer::convert_expr_within(const AnalysisCommand &) {
    return create_double_input_comparison_op();
}

PType Typer::convert_expr_within_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType Typer::convert_expr_within_left_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType Typer::convert_expr_within_right_exclusive(const AnalysisCommand &) {
    return create_double_input_comparison_op();

}

PType Typer::convert_expr_negate(const AnalysisCommand &) {
    // `a -> `a | `a <<: Number
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);
    
    return fun;
}

PType Typer::convert_expr_logical_not(const AnalysisCommand &) {
// `a -> `a | `a <<: Cond
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_COND)));
    fun->add_constraint(numeric);
    
    return fun;
}

PType Typer::convert_expr_if_ternary(const AnalysisCommand &) {
    // Cond x `a x `a -> `a
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto result_type = std::make_shared<Type>(TYPE_GENERIC);
    fun->add_source_type(TYPE_COND);
    fun->add_source_type(result_type);
    fun->add_source_type(result_type);
    fun->add_dest_type(result_type);
    return fun;
}

PType Typer::convert_expr_index(const AnalysisCommand &) {
    // List<`a> x Number -> `a
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
    fun->add_dest_type(element_type);
    return fun;
}

PType Typer::convert_expr_index_range(const AnalysisCommand &) {
    // List<`a> x Number x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
    fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

PType Typer::convert_expr_index_until(const AnalysisCommand &) {
    // List<`a> x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

PType Typer::convert_expr_index_from(const AnalysisCommand &) {
    // List<`a> x Number -> List<`a>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
    fun->add_dest_type(source_list);
    return fun;
}

// Helper function for particle -> number functions
static PType create_particle_to_number_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}


PType Typer::convert_func_charge(const AnalysisCommand &) {
    return create_particle_to_number_func();
}


PType Typer::convert_func_pt(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType Typer::convert_func_eta(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType Typer::convert_func_phi(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType Typer::convert_func_mass(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

PType Typer::convert_func_energy(const AnalysisCommand &) {
    return create_particle_to_number_func();
}

// Helper function for dr/dphi/deta style functions
static PType create_particle_pair_to_number_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type_a);
    fun->add_source_type(source_type_b);
    fun->add_dest_type(dest_type_c);

    Constraint particlelike_a(fun);
    particlelike_a.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike_a);

    Constraint particlelike_b(fun);
    particlelike_b.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike_b);

    Constraint numeric_c(fun);
    numeric_c.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric_c);

    Constraint a_is_single(fun);
    a_is_single.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    a_is_single.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type_b, dest_type_c));
    fun->add_constraint(a_is_single);

    Constraint b_is_single(fun);
    b_is_single.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    b_is_single.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type_a, dest_type_c));
    fun->add_constraint(b_is_single);

    auto list_particle_instance = std::make_shared<Type>(TYPE_LIST);
    list_particle_instance->add_dest_type(TYPE_PARTICLEINSTANCE);

    auto list_list_number = std::make_shared<Type>(TYPE_LIST);
    auto inner_list_number = std::make_shared<Type>(TYPE_LIST);
    inner_list_number->add_dest_type(TYPE_NUMBER);
    list_list_number->add_dest_type(inner_list_number);

    Constraint both_lists(fun);
    both_lists.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, list_particle_instance));
    both_lists.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, list_particle_instance));
    both_lists.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, list_list_number));
    fun->add_constraint(both_lists);

    Constraint error_condition_a(fun);
    error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, list_particle_instance));
    error_condition_a.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition_a);

    Constraint error_condition_b(fun);
    error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, list_particle_instance));
    error_condition_b.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
    fun->add_constraint(error_condition_b);

    return fun;
}

PType Typer::convert_func_distinct(const AnalysisCommand &) {
    // `a x `a -> `b | `a <<: ParticleInstance, `b <<: Number, `a ~=~ `b
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}

PType Typer::convert_func_dr(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

PType Typer::convert_func_dphi(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

PType Typer::convert_func_deta(const AnalysisCommand &) {
    return create_particle_pair_to_number_func();
}

// Helper for hadamard versions
static PType create_hadamard_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(dest_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    Constraint equal_depth(fun);
    equal_depth.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type, dest_type));
    fun->add_constraint(equal_depth);

    return fun;
}

PType Typer::convert_func_dr_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType Typer::convert_func_dphi_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType Typer::convert_func_deta_hadamard(const AnalysisCommand &) {
    return create_hadamard_func();
}

PType Typer::convert_func_size(const AnalysisCommand &) {
    // List<`a> -> Number
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(TYPE_GENERIC);
    fun->add_source_type(source_list);
    fun->add_dest_type(TYPE_NUMBER);
    return fun;
}

static PType list_reducer_func() {
    // List<`a> -> `a | `a <<: Cond
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);
    source_list->add_dest_type(source_type);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_type);

    Constraint boolean(fun);
    boolean.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_COND)));
    fun->add_constraint(boolean);
    
    return fun;
}

PType Typer::convert_func_anyof(const AnalysisCommand &) {
    return list_reducer_func();
}

PType Typer::convert_func_allof(const AnalysisCommand &) {
    return list_reducer_func();
}

// Helper for unary numeric functions
static PType create_unary_numeric_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType Typer::convert_func_sqrt(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_abs(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_cos(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_sin(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_tan(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_sinh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_cosh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_tanh(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_exp(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

PType Typer::convert_func_log(const AnalysisCommand &) {
    return create_unary_numeric_func();
}

// Helper for list reduction functions (ave, sum, min, max)
static PType create_list_reduction_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    auto source_list = std::make_shared<Type>(TYPE_LIST);
    source_list->add_dest_type(element_type);

    fun->add_source_type(source_list);
    fun->add_dest_type(element_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType Typer::convert_func_ave(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType Typer::convert_func_sum(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType Typer::convert_func_min_of_list(const AnalysisCommand &) {
    return create_list_reduction_func();
}

PType Typer::convert_func_max_of_list(const AnalysisCommand &) {
    return create_list_reduction_func();
}

// Helper for pair min/max functions
static PType create_pair_minmax_func() {
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto source_type = std::make_shared<Type>(TYPE_GENERIC);

    fun->add_source_type(source_type);
    fun->add_source_type(source_type);
    fun->add_dest_type(source_type);

    Constraint numeric(fun);
    numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
    fun->add_constraint(numeric);

    return fun;
}

PType Typer::convert_func_min_of_pair(const AnalysisCommand &) {
    return create_pair_minmax_func();
}

PType Typer::convert_func_max_of_pair(const AnalysisCommand &) {
    return create_pair_minmax_func();
}

PType Typer::convert_func_sort_ascend(const AnalysisCommand &) {
    // List<Number> -> List<Number>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType source_list(std::make_shared<Type>(TYPE_LIST));
    source_list->add_dest_type(TYPE_NUMBER);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_list);

    return fun;
}

PType Typer::convert_func_sort_descend(const AnalysisCommand &) {
    // List<Number> -> List<Number>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType source_list(std::make_shared<Type>(TYPE_LIST));
    source_list->add_dest_type(TYPE_NUMBER);

    fun->add_source_type(source_list);
    fun->add_dest_type(source_list);

    return fun;
}

PType Typer::convert_func_named(const AnalysisCommand &) {
    // `a x (`b->`c) -> `d | `a <: `b, (`a = `b => `c = `d)
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    
    auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
    auto type_b = std::make_shared<Type>(TYPE_GENERIC);
    auto type_c = std::make_shared<Type>(TYPE_GENERIC);
    auto dest_type_d = std::make_shared<Type>(TYPE_GENERIC);

    auto func_type = std::make_shared<Type>(TYPE_FUNCTION);
    func_type->add_source_type(type_b);
    func_type->add_dest_type(type_c);

    fun->add_source_type(source_type_a);
    fun->add_source_type(func_type);
    fun->add_dest_type(dest_type_d);

    Constraint subtype(fun);
    subtype.add_conclusion(Statement(STATEMENT_SUBTYPE, source_type_a, type_b));
    fun->add_constraint(subtype);

    Constraint implication_of_origin(fun);
    implication_of_origin.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, type_b));
    implication_of_origin.add_conclusion(Statement(STATEMENT_EQUALITY, type_c, dest_type_d));
    fun->add_constraint(implication_of_origin);

    return fun;
}

PType Typer::convert_create_empty_string_list(const AnalysisCommand &) {
    // () -> List<String>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE_LIST);
    dest_list->add_dest_type(TYPE_STRING);
    fun->add_dest_type(dest_list);
    return fun;
}

PType Typer::convert_add_string_to_list(const AnalysisCommand &) {
    // List<String> x String -> List<String>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_STRING);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE_STRING);
    fun->add_dest_type(list_type);
    return fun;
}

PType Typer::convert_create_empty_value_list(const AnalysisCommand &) {
    // () -> List<Number>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto dest_list = std::make_shared<Type>(TYPE_LIST);
    dest_list->add_dest_type(TYPE_NUMBER);
    fun->add_dest_type(dest_list);
    return fun;
}

PType Typer::convert_add_value_to_list(const AnalysisCommand &) {
    // List<Number> x Number -> List<Number>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_NUMBER);
    fun->add_source_type(list_type);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_dest_type(list_type);
    return fun;
}

PType Typer::convert_create_empty_union(const AnalysisCommand &) {
    // () -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType list(std::make_shared<Type>(TYPE_LIST));
    list->add_dest_type(TYPE_PARTICLEINSTANCE);

    fun->add_dest_type(list);
    return fun;
}

PType Typer::convert_add_part_to_union(const AnalysisCommand &) {
    // List<ParticleInstance> x List<ParticleInstance> -> List<ParticleInstance>
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));

    PType list(std::make_shared<Type>(TYPE_LIST));
    list->add_dest_type(TYPE_PARTICLEINSTANCE);

    fun->add_source_type(list);
    fun->add_source_type(list);
    fun->add_dest_type(list);
    return fun;
}

PType Typer::convert_create_empty_cartesian(const AnalysisCommand &) {
    // () -> Comb
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_dest_type(TYPE_COMB);
    return fun;
}

PType Typer::convert_create_empty_disjoint(const AnalysisCommand &) {
    // () -> Disjoint
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_dest_type(TYPE_COMB);
    return fun;
}

PType Typer::convert_create_empty_direct(const AnalysisCommand &) {
    // () -> Direct
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_dest_type(TYPE_COMB);
    return fun;
}

PType Typer::convert_add_part_to_composite(const AnalysisCommand &) {
    // Comb x List<ParticleInstance> -> Comb (or Disjoint -> Disjoint, Direct -> Direct)
    // Using generic composite type
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    fun->add_source_type(TYPE_COMB);
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_PARTICLEINSTANCE);
    fun->add_source_type(list_type);
    fun->add_dest_type(TYPE_COMB);
    return fun;
}

PType Typer::convert_name_element_of_composite(const AnalysisCommand &) {
    // Comb x Number x List<ParticleInstance> -> List<ParticleInstance>

    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto list_type = std::make_shared<Type>(TYPE_LIST);
    list_type->add_dest_type(TYPE_PARTICLEINSTANCE);

    fun->add_source_type(TYPE_COMB);
    fun->add_source_type(TYPE_NUMBER);
    fun->add_source_type(list_type);
    fun->add_dest_type(list_type);
    return fun;
}

PType Typer::convert_create_empty_particle(const AnalysisCommand &) {
    // () -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    return fun;
}

PType Typer::convert_add_particle(const AnalysisCommand &) {
    // `a x `a -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    fun->add_source_type(element_type);
    fun->add_source_type(element_type);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    // // List<ParticleInstance> x List<ParticleInstance> -> List<ParticleInstance>
    // PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    // auto element_type = std::make_shared<Type>(TYPE_LIST);
    // element_type->add_dest_type(TYPE_PARTICLEINSTANCE);

    // fun->add_source_type(element_type);
    // fun->add_source_type(element_type);
    // fun->add_dest_type(element_type);

    return fun;
}

PType Typer::convert_sub_particle(const AnalysisCommand &) {
    // `a x `a -> `a | `a <<: ParticleInstance
    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    auto element_type = std::make_shared<Type>(TYPE_GENERIC);
    fun->add_source_type(element_type);
    fun->add_source_type(element_type);
    fun->add_dest_type(element_type);

    Constraint particlelike(fun);
    particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
    fun->add_constraint(particlelike);

    return fun;
}




PType EquivalenceClasses::find_representative(PType source) {
    if (parent.count(source) == 0) {
        parent[source] = source; // self-represent an empty class
    }

    if (parent[source] != source) {
        parent[source] = find_representative(parent[source]);
    }   
    return parent[source];
}

void EquivalenceClasses::union_of_classes(PType first, PType second) {
    PType first_representative = find_representative(first);
    PType second_representative = find_representative(second);

    
    // these two classes are already the same
    if (first_representative == second_representative) return;


    // we would really like to have the class be represented by a not completely generic type
    bool first_generic_class = (first_representative->get_base_type() == TYPE_GENERIC);
    bool second_generic_class = (second_representative->get_base_type() == TYPE_GENERIC);

    if (first_generic_class && !second_generic_class) {
        // first class is represented by non-generic type - use it for the second as well
        parent[first_representative] = second_representative;
    } else if (!first_generic_class && second_generic_class) {
        // same with second class
        parent[second_representative] = first;
    } else {
        // either they are both non-generic or both not. Choose the first as our representative arbitrarily
        parent[second_representative] = first;

        if (!first_generic_class && !second_generic_class) {
            //TODO: replace with exception
            if (first_representative->get_base_type() != second_representative->get_base_type()) {
                parent[second_representative] = first;
            }

            if (first_representative->get_base_type() == TYPE_FUNCTION) {
                for (int i = 0; i < first_representative->get_num_of_sources(); i++) {
                    union_of_classes(first_representative->get_source_type(i), second_representative->get_source_type(i));
                }
            }

            if (first_representative->get_base_type() == TYPE_FUNCTION || first_representative->get_base_type() == TYPE_LIST) {
                union_of_classes(first_representative->get_dest_type(), second_representative->get_dest_type());
            }
        }
        
    }
}



PType EquivalenceClasses::resolve_higher_order(PType source) {
    PType rep = find_representative(source);

    if (rep->get_base_type() == TYPE_GENERIC) {
        // the best we can do is a generic
        return rep;
    }

    if (rep->get_base_type() == TYPE_FUNCTION) {
        // this is represented by a function - ensure we have the most specific form of function possible as the representative type for this

        PType fun(std::make_shared<Type>(TYPE_FUNCTION));
        for (int i = 0; i < rep->get_num_of_sources(); i++) {
            fun->add_source_type(resolve_higher_order(rep->get_source_type(i)));
        }
        PType resolved_dest = resolve_higher_order(rep->get_dest_type());
        fun->add_dest_type(resolved_dest);
        parent[rep] = fun;
        return fun;

    }

    if (rep->get_base_type() == TYPE_LIST) {
        // similarly, this is represented by a list - ensure we have the most specific form of list
        PType resolved_elem = resolve_higher_order(rep->get_dest_type());
        PType list(std::make_shared<Type>(TYPE_LIST));
        list->add_dest_type(resolved_elem);
        parent[rep] = list;
        return list;

    }

    return rep;
}

std::unordered_map<PType, PType> EquivalenceClasses::resolve_all() {
    std::unordered_map<PType, PType> resolved_map;
    
    // first collect all our types
    std::vector<PType> all_types;
    for (const auto &entry : parent) {
        all_types.push_back(entry.first);
    }

    for (PType type : all_types) {
        resolved_map[type] = resolve_higher_order(type);
    }

    return resolved_map;
}

std::optional<int> DepthEquivalence::compute_structural_depth(PType type) {
    if (!type) return std::nullopt;
    
    switch (type->get_base_type()) {
        case TYPE_GENERIC:
            return std::nullopt;
        case TYPE_LIST: {
            auto elem = compute_structural_depth(type->get_dest_type());
            return elem.has_value() ? std::optional<int>(1 + *elem) : std::nullopt;
        }
        case TYPE_FUNCTION:
            return compute_structural_depth(type->get_dest_type());
        default:
            return 0;
    }
}

std::optional<int> DepthEquivalence::get_depth(PType type) {
    PType rep = internal_equiv.find_representative(type);
    return compute_structural_depth(rep);
}

bool DepthEquivalence::add_equal_depth(PType first, PType second) {
    auto d1 = get_depth(first);
    auto d2 = get_depth(second);
    
    if (d1.has_value() && d2.has_value() && *d1 != *d2) {
        return false; // contradiction
    }
    
    internal_equiv.union_of_classes(first, second);
    return true;
}

Ternary DepthEquivalence::have_equal_depth(PType first, PType second) {
    if (internal_equiv.find_representative(first) == internal_equiv.find_representative(second)) {
        return Ternary::TERN_TRUE;
    }
    
    auto d1 = get_depth(first);
    auto d2 = get_depth(second);
    
    if (d1.has_value() && d2.has_value()) {
        return (*d1 == *d2) ? Ternary::TERN_TRUE : Ternary::TERN_FALSE;
    }
    
    return Ternary::TERN_UNKNOWN;
}

void PartialOrder::ensure_exists(PType type) {
    if (supertypes.find(type) == supertypes.end()) {
        supertypes[type] = {};
        subtypes[type] = {};
    }
}

bool PartialOrder::has_path(PType from, PType to, std::unordered_set<PType> &visited) {

    // trivial path
    if (from == to) return true;

    // if we have looped back around to this, we have definitely not found a path
    if (visited.count(from) != 0) return false;
    
    visited.insert(from);
    
    // check the supertypes of the LHS - if we have found a path from there, there is clearly a chain since a <: b <: c ==> a <: c
    for (PType super : supertypes[from]) {
        if (has_path(super, to, visited)) {
            return true;
        }
    }
    
    return false;
}

bool PartialOrder::add_subtype(PType sub, PType super){
    ensure_exists(sub);
    ensure_exists(super);

    // a <: a always
    if (sub == super) return true;

    bool sub_generic = sub->get_base_type() == TYPE_GENERIC;
    bool super_generic = super->get_base_type() == TYPE_GENERIC;

    if (!sub_generic && !super_generic) {
        // both are not fully generic types - we can use more information
        if (sub->get_base_type() == TYPE_FUNCTION && super->get_base_type() == TYPE_FUNCTION) {
            assert(sub->get_num_of_sources() == super->get_num_of_sources());
            
            // functions are contravariant in arguments - add those constraints
            for (int i = 0; i < sub->get_num_of_sources(); i++) {
                add_subtype(super->get_source_type(i), sub->get_source_type(i));
            }
            
            // functions are also covariant in return type
            add_subtype(sub->get_dest_type(), super->get_dest_type());
            
        } else if (sub->get_base_type() == TYPE_LIST && super->get_base_type() == TYPE_LIST) {
            // lists are covariant //TODO: check this against our proofs
            add_subtype(sub->get_dest_type(), super->get_dest_type());
        } else {
            // the two base types should always match if neither are directly generic
            if(sub->get_base_type() != super->get_base_type()) {
                return false;
            }
        }
    }

    // one is now a subtype of the other
    supertypes[sub].insert(super);
    subtypes[super].insert(sub);

    return true;


}


bool PartialOrder::is_subtype(PType sub, PType super) {
    ensure_exists(sub);
    ensure_exists(super);
    
    std::unordered_set<PType> visited;
    return has_path(sub, super, visited);
}

std::unordered_set<PType> PartialOrder::get_supertypes(PType type) {
    ensure_exists(type);
    
    std::unordered_set<PType> result;
    std::queue<PType> worklist;
    
    worklist.push(type);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        for (PType super : supertypes[current]) {
            if (result.insert(super).second) {
                worklist.push(super);
            }
        }
    }
    
    return result;
}

std::unordered_set<PType> PartialOrder::get_subtypes(PType type) {
    ensure_exists(type);
    
    std::unordered_set<PType> result;
    std::queue<PType> worklist;
    
    worklist.push(type);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        for (PType sub : subtypes[current]) {
            if (result.insert(sub).second) {
                worklist.push(sub);
            }
        }
    }
    
    return result;
}

PType PartialOrder::least_upper_bound(PType a, PType b) {
    ensure_exists(a);
    ensure_exists(b);
    
    if (is_subtype(a, b)) return b;
    if (is_subtype(b, a)) return a;
    
    std::unordered_set<PType> a_supers = get_supertypes(a);
    a_supers.insert(a);
    
    std::queue<PType> worklist;
    worklist.push(b);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        if (a_supers.count(current)) {
            return current;
        }
        
        for (PType super : supertypes[current]) {
            worklist.push(super);
        }
    }
    
    return nullptr; // no common supertype found TODO: error here?
}

PType PartialOrder::greatest_lower_bound(PType a, PType b) {
    ensure_exists(a);
    ensure_exists(b);
    
    if (is_subtype(a, b)) return a;
    if (is_subtype(b, a)) return b;
    
    std::unordered_set<PType> a_subs = get_subtypes(a);
    a_subs.insert(a);
    
    std::queue<PType> worklist;
    worklist.push(b);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        if (a_subs.count(current)) {
            return current;
        }
        
        for (PType sub : subtypes[current]) {
            worklist.push(sub);
        }
    }
    
    return nullptr; // no common subtype found TODO: error here?
}

std::unordered_map<PType, std::unordered_set<PType>> PartialOrder::get_all_supertypes() {
    std::unordered_map<PType, std::unordered_set<PType>> result;
    
    for (const auto &entry : supertypes) {
        result[entry.first] = get_supertypes(entry.first);
    }
    
    return result;
}

std::unordered_map<PType, std::unordered_set<PType>> PartialOrder::get_all_subtypes() {
    std::unordered_map<PType, std::unordered_set<PType>> result;
    
    for (const auto &entry : subtypes) {
        result[entry.first] = get_subtypes(entry.first);
    }
    
    return result;
}

std::vector<PType> PartialOrder::topological_sort() {
    std::unordered_map<PType, int> in_degree;
    std::vector<PType> result;
    std::queue<PType> worklist;
    
    for (const auto &entry : subtypes) {
        in_degree[entry.first] = entry.second.size();
    }
    
    for (const auto &entry : in_degree) {
        if (entry.second == 0) {
            worklist.push(entry.first);
        }
    }
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        result.push_back(current);
        
        for (PType super : supertypes[current]) {
            in_degree[super]--;
            if (in_degree[super] == 0) {
                worklist.push(super);
            }
        }
    }
    
    return result;
}



bool Typer::equality_of_types(PType first, PType second) {

    auto first_representative = equiv.find_representative(first);
    auto second_representative = equiv.find_representative(second);

    bool first_generic_class = (first_representative->get_base_type() == TYPE_GENERIC);
    bool second_generic_class = (second_representative->get_base_type() == TYPE_GENERIC);

    if (!first_generic_class && !second_generic_class && first_representative->get_base_type() != second_representative->get_base_type()) {
        return false;
    }


    equiv.union_of_classes(first, second);
    depth_equiv.add_equal_depth(first, second);

    return true;
}


bool Typer::equal_depth_of_types(PType first, PType second) {
    if (!depth_equiv.add_equal_depth(first, second)) {
        return false;
    }
    return true;
}

void Typer::apply_depth_hereditary_simplification() {
    // std::cout << "Simplifyinh" << std::endl;
    // For all pairs where a ~=~ b and a <<: b, add a <: b
    
    auto all_hereditary = hereditary_subtyping.get_all_supertypes();
    
    for (const auto& [sub, supers] : all_hereditary) {
        for (PType super : supers) {

            // sub->print();
            // depth_equiv.find_representative(sub)->print();
            // std::cout << "\n";
            // super->print();
            // std::cout << std::endl;

            Ternary same_depth = depth_equiv.have_equal_depth(sub, super);
            
            auto sub_depth = depth_equiv.get_depth(sub);

            if (same_depth == Ternary::TERN_TRUE) {
                // a ~=~ b and a <<: b => a <: b
                // subtyping.add_subtype(sub, super);

                // TODO: remove this simplification
                equality_of_types(sub, super);
                // equiv.union_of_classes(sub, super);
            } else if (sub_depth.has_value()) {
                auto super_depth = depth_equiv.get_depth(super);
                if (super_depth.has_value() && *sub_depth == *super_depth) {
                    // subtyping.add_subtype(sub, super);
                    // continue;
                    equality_of_types(sub, super);
                    // equiv.union_of_classes(sub, super);
                }
            }
        }
    }
}

bool Typer::subtype_of_types(PType sub, PType super) {
    return subtyping.add_subtype(sub, super);
}

bool Typer::hereditary_subtype_of_types(PType sub, PType super) {
    return hereditary_subtyping.add_subtype(sub, super);
}


Ternary Typer::truth_of_premise(PType lhs, PType rhs, StatementForm form) {
    // get representatives, this is the canonical form from our equivalence classes
    PType lhs_rep = equiv.find_representative(lhs);
    PType rhs_rep = equiv.find_representative(rhs);
    
    switch (form) {
        case STATEMENT_EQUALITY: {
            // same representative necessarily means equal
            if (lhs_rep == rhs_rep) {
                return Ternary::TERN_TRUE;
            }
            
            // structural incompatibility means there is no way for them to be equal
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT_INEQUALITY: {
            // same representative means equal, so inequality is false
            if (lhs_rep == rhs_rep) {
                return Ternary::TERN_FALSE;
            }
            
            // structural incompatibility means definitely unequal
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT_SUBTYPE: {
            // manifestly a subtype
            if (subtyping.is_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            // incompatable and so impossible for them to be a subtype without having been explicitly declared as such
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT_NONSUBTYPE: {
            if (subtyping.is_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT_SUPERTYPE: {
            return truth_of_premise(rhs, lhs, STATEMENT_SUBTYPE);
        }
        
        case STATEMENT_NONSUPERTYPE: {
            return truth_of_premise(rhs, lhs, STATEMENT_NONSUBTYPE);
        }
        
        case STATEMENT_HEREDITARY_SUBTYPE: {
            if (hereditary_subtyping.is_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            // cannot ever be hereditarily matched
            if (types_hereditarily_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT_HEREDITARY_SUPERTYPE: {
            return truth_of_premise(rhs, lhs, STATEMENT_HEREDITARY_SUBTYPE);
        }
        
        case STATEMENT_EQUAL_DEPTH: {
            return depth_equiv.have_equal_depth(lhs_rep, rhs_rep);
        }
    }
    
    return Ternary::TERN_UNKNOWN;
}


bool Typer::types_structurally_incompatible(PType lhs, PType rhs) {
    // in a vacuum, generic types are never structurally incompatible, as they can unify with anything
    if (lhs->get_base_type() == TYPE_GENERIC || rhs->get_base_type() == TYPE_GENERIC) {
        return false;
    }
    
    // different base types are incompatible
    if (lhs->get_base_type() != rhs->get_base_type()) {
        return true;
    }
    
    // same base type, need to check structural compatibility
    if (lhs->get_base_type() == TYPE_FUNCTION) {
        // functions with different arities are incompatible (since we do not allow Currying or anything)
        if (lhs->get_num_of_sources() != rhs->get_num_of_sources()) {
            return true;
        }
        
        // check argument and return type compatibility recursively
        for (int i = 0; i < lhs->get_num_of_sources(); i++) {
            PType lhs_src = equiv.find_representative(lhs->get_source_type(i));
            PType rhs_src = equiv.find_representative(rhs->get_source_type(i));
            if (types_structurally_incompatible(lhs_src, rhs_src)) {
                return true;
            }
        }
        
        PType lhs_dest = equiv.find_representative(lhs->get_dest_type());
        PType rhs_dest = equiv.find_representative(rhs->get_dest_type());
        if (types_structurally_incompatible(lhs_dest, rhs_dest)) {
            return true;
        }
    }
    
    if (lhs->get_base_type() == TYPE_LIST) {
        PType lhs_elem = equiv.find_representative(lhs->get_dest_type());
        PType rhs_elem = equiv.find_representative(rhs->get_dest_type());
        if (types_structurally_incompatible(lhs_elem, rhs_elem)) {
            return true;
        }
    }
    
    return false;
}

bool Typer::types_hereditarily_incompatible(PType lhs, PType rhs) {
    // generic types can potentially satisfy any hereditary relationship
    if (lhs->get_base_type() == TYPE_GENERIC || rhs->get_base_type() == TYPE_GENERIC) {
        return false;
    }
    
    // unwrap lists on the left side (hereditary means we can go through list layers)
    PType unwrapped_lhs = lhs;
    while (unwrapped_lhs->get_base_type() == TYPE_LIST) {
        unwrapped_lhs = equiv.find_representative(unwrapped_lhs->get_dest_type());
        if (unwrapped_lhs->get_base_type() == TYPE_GENERIC) {
            return false; // Generic element, can't determine
        }
    }
    
    // Unwrap lists on the right side
    PType unwrapped_rhs = rhs;
    while (unwrapped_rhs->get_base_type() == TYPE_LIST) {
        unwrapped_rhs = equiv.find_representative(unwrapped_rhs->get_dest_type());
        if (unwrapped_rhs->get_base_type() == TYPE_GENERIC) {
            return false;
        }
    }
    
    // Now compare the base (unwrapped) types
    // For hereditary subtyping, the unwrapped types must be compatible
    if (unwrapped_lhs->get_base_type() == TYPE_FUNCTION && 
        unwrapped_rhs->get_base_type() == TYPE_FUNCTION) {
        // Functions - check arity at minimum
        if (unwrapped_lhs->get_num_of_sources() != unwrapped_rhs->get_num_of_sources()) {
            return true;
        }
        return false; // Could still be compatible
    }
    
    // For fundamental types, they must match
    if (unwrapped_lhs->is_fundamental_type() && unwrapped_rhs->is_fundamental_type()) {
        return unwrapped_lhs->get_base_type() != unwrapped_rhs->get_base_type();
    }
    
    return false;
}

void Typer::resolve_constraints() {


    std::cout << "\n Resolving constraints... \n";

    std::vector<Constraint> new_running_valid_constraints;

    int i = -1;
    for (auto constraint : running_valid_constraints) {
        i++;

        // assume all premises are true until proven otherwise
        Ternary has_true_premises = Ternary::TERN_TRUE;

        for (auto premise : constraint.get_premises()) {
            auto first = premise.get_lhs();
            auto second = premise.get_rhs();

            auto true_first = equiv.find_representative(first);
            auto true_second = equiv.find_representative(second);
 
            has_true_premises.eq_land(truth_of_premise(true_first, true_second, premise.get_form()));
        }

        if (has_true_premises == Ternary::TERN_FALSE) {
            std::cout << "Rejecting a premise:";
            constraint.print();
            std::cout << std::endl;
        }

        if (has_true_premises == Ternary::TERN_UNKNOWN) {
            std::cout << "A premise is unknown:";
            constraint.print();
            std::cout << std::endl;
            new_running_valid_constraints.push_back(constraint);
        }

        if (has_true_premises != Ternary::TERN_TRUE) {
            continue;
        }

        for (auto conclusion : constraint.get_conclusions()) {

            auto first = conclusion.get_lhs();
            auto second = conclusion.get_rhs();


            std::streambuf *old_stdout = std::cout.rdbuf();

            std::stringstream print1;
            std::cout.rdbuf(print1.rdbuf());
            first->print();
            std::stringstream print2;
            std::cout.rdbuf(print2.rdbuf());
            second->print();
            std::cout.rdbuf(old_stdout);


            if (conclusion.get_form() == STATEMENT_EQUALITY) {
                std::cout <<"equaling " << print1.str() << " with " << print2.str() <<"\n";
                if (!equality_of_types(first, second)) {
                    raise_type_exception("Cannot equate "+print1.str() +" type with "+print2.str()+" type", constraint.get_originator_type().lock()->get_origin_token());
                }
            } else if (conclusion.get_form() == STATEMENT_SUBTYPE) {
                std::cout << "subtyping " << print1.str() << " with " << print2.str() <<"\n";
                if (!subtype_of_types(first, second)) {
                    raise_type_exception("Failed to subtype "+print1.str() +" with "+print2.str(), constraint.get_originator_type().lock()->get_origin_token());
                }
            } else if (conclusion.get_form() == STATEMENT_HEREDITARY_SUBTYPE) {
                std::cout << "chain-subtyping " << print1.str() << " with " << print2.str() <<"\n";
                if (!hereditary_subtype_of_types(first, second)) {
                    raise_type_exception("Failed to list-subtype "+print1.str() +" with "+print2.str(), constraint.get_originator_type().lock()->get_origin_token());
                }
            } else if (conclusion.get_form() == STATEMENT_EQUAL_DEPTH) {
                std::cout <<"enforcing equal depth of " << print1.str() << " with " << print2.str() <<"\n";
                if (!equal_depth_of_types(first, second)) {
                    raise_type_exception("Failed to enforce equal list-depth of "+print1.str() +" with "+print2.str(), constraint.get_originator_type().lock()->get_origin_token());  
                }
            } else {
                // the consequent is not something we are equipped to deal with yet, we add it and only it
                // remaining constraints are constraints of exclusion - not much to be done with them at this point
                Constraint consequent(constraint.get_originator_type());
                consequent.add_conclusion(conclusion);
                new_running_valid_constraints.push_back(consequent);
            }

        }
    }

    // allow depths to matter
    apply_depth_hereditary_simplification();

    auto equalities = equiv.resolve_all();

    std::cout << "\n Constraints resolved.\n";

    for (auto variable : order_of_variables) {
        auto type_of_var = types_of_variables[variable];

        if (used_variables.count(variable) == 0) {
            std::cout << "UNUSED ";
        }

        std::cout << variable << " : ";

        if (equalities.contains(type_of_var)) type_of_var = equalities[type_of_var];
        type_of_var->print(equalities);
        
        std::cout << "\n";
    }
    std::cout << std::endl;

    for (auto constraint : new_running_valid_constraints) {
        constraint.print(equalities);
        std::cout << "\n";
    }

    running_valid_constraints = new_running_valid_constraints;

}


void Typer::collect_existing_constraints() {

    std::regex reg_string;
    std::regex reg_number;

    reg_number = std::regex("-{0,1}[0-9]*\\.{0,1}[0-9]*([Ee][-+]{0,1}[0-9]+){0,1}");
    reg_string= std::regex("\"[^\"]*\"");

    for (auto &command : alil->get_commands()) {

        auto type_of_function = command_convert(command);
        all_live_types.push_back(type_of_function);
        type_of_function->set_origin_token(command.get_token());
        type_of_function->print();

        // all the constraints this type comes with, we add to ours
        for (auto constraint : type_of_function->get_constraints()) {
            running_valid_constraints.push_back(constraint);
            // constraint.print();
        }

        // generate a series of constraints for the inputs to match with the 
        for (int i = 0; i < command.get_num_arguments() - (command.has_dest_argument() ? 1 : 0); i++) {
            std::string arg = command.get_source_argument(i);

            used_variables.emplace(arg);

            PType type_of_arg;
            if (types_of_variables.count(arg) == 0) {
                if (std::regex_match(arg, reg_string)) {
                    type_of_arg = Type::fundamental_type_instance(TYPE_STRING);
                } else if (std::regex_match(arg, reg_number)) {
                    type_of_arg = Type::fundamental_type_instance(TYPE_NUMBER);
                } else {
                    // error condition - variable is undefined
                    // std::cerr << arg << std::endl;
                    if (false) { //TODO: add the actual config
                        raise_type_exception("The variable " + arg + " used in this function is undefined - use the automatic setting if you want this to automatically infer a type for it", command.get_token());
                    } else {
                        type_of_arg = std::make_shared<Type>(TYPE_GENERIC);
                        types_of_variables[arg] = type_of_arg;
                    }
                }
            } else {
                type_of_arg = types_of_variables[arg];
            }
            Constraint equality_of_input(type_of_function);
            if (type_of_function->get_num_of_sources() <= i) {
                std::cerr << type_of_function->get_num_of_sources();
                assert(false);
            } 
            equality_of_input.add_conclusion(Statement(STATEMENT_EQUALITY, type_of_arg, type_of_function->get_source_type(i)));
            // equality_of_input.print();
            running_valid_constraints.push_back(equality_of_input);
        }


        if (command.has_dest_argument()) {
            // the destination necessarily has the type of the codomain of the function in question
            order_of_variables.push_back(command.get_dest_argument());
            types_of_variables.emplace(command.get_dest_argument(), type_of_function->get_dest_type());
        }

        

        // out->print();
        std::cout << std::endl;

    }

    int i = 0;
    for (auto constraint : running_valid_constraints) {
        std::cout << i++ << "|";
        constraint.print();
        std::cout << "\n";
    }
    std::cout << std::endl;
}


void Typer::print() {

    Type::highest_mapped_generic = 0;
    Type::generic_map.clear();
    types_of_variables.clear();

    collect_existing_constraints();

    for (int i = 0; i < 4; i++) {
        std::cout << i+1 << "th pass \n" << std::endl;
        resolve_constraints();
    }

    // resolve_constraints();

    // std::cout << "\n\n\n" << "Second pass \n" << std::endl;

    // resolve_constraints();

    // std::cout << "\n\n\n" << "Thirdm pass \n" << std::endl;

    // resolve_constraints();

    auto equals = equiv.resolve_all();

    for (auto command : alil->get_commands()) {
        if (command.has_dest_argument()) {
            types_of_variables[command.get_dest_argument()]->print(equals);
        } else {
            std::cout << "void";
        }
        std::cout << " : ";
        command.print_instruction();
        std::cout << std::endl;
    }

    // while (alil->clear_to_next()) {
    //     auto out = command_handle(alil->next_command());
    //     out->print();
    //     std::cout << std::endl;

        
    // }
}

