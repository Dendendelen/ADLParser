
#include "type_checker.hpp"
#include "ali_converter.hpp"
#include "node.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <unordered_map>
#include <vector>


std::unordered_map<BaseType, PType> Type::base_type_instances_map;
std::unordered_map<PType, int> Type::generic_map;
int Type::highest_mapped_generic;


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
    if (this_type != TYPE_FUNCTION) {
        assert(false);
    }
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
    if (this_type != TYPE_FUNCTION && this_type != TYPE_LIST) {
        assert(false);
    }
    if (has_dest_type) {
        assert(false);
    }
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
        case TYPE_UNION:
            return "union";
        case TYPE_COMB:
            return "comb";
        case TYPE_DISJOINT:
            return "disjoint";
        case TYPE_HIST:
            return "hist";
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
        dest_type->print(substitutions);
        std::cout << ")";
    } else if (this_type == TYPE_LIST) {
        std::cout << "<";
        dest_type->print(substitutions);
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

PType Typer::command_handle(AnalysisCommand in) {

    in.print_instruction();

    PType fun(std::make_shared<Type>(TYPE_FUNCTION));
    switch (in.get_instruction()) {

    case CREATE_REGION:
        // () -> Region
        fun->add_dest_type(TYPE_REGION);
        break;
    case MERGE_REGIONS:
        // Region x Region -> Region
        fun->add_source_type(TYPE_REGION);
        fun->add_source_type(TYPE_REGION);
        fun->add_dest_type(TYPE_REGION);
        break;
    case CUT_REGION:
        // Region x Cond -> Region
        fun->add_source_type(TYPE_REGION);
        fun->add_source_type(TYPE_COND);
        fun->add_dest_type(TYPE_REGION);
        break;
    case ADD_ALIAS: case END_EXPRESSION:
    {
        // `a -> `a
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_dest_type(source_type);

        // // | `a = `b
        // Constraint equality;
        // equality.add_conclusion(Statement(STATEMENT_EQUALITY, source_type, dest_type));

        // fun->add_constraint(equality);
        break;

    }
    case ADD_EXTERNAL:
    {
        // String -> `a
        fun->add_source_type(TYPE_STRING);
        fun->add_dest_type(TYPE_GENERIC);
        break;
    }
    
    case ADD_EXTERN_ATTR:
    {
        // (`a -> `b)
        auto dest_fun = std::make_shared<Type>(TYPE_FUNCTION);
        auto source_of_dest_fun = std::make_shared<Type>(TYPE_GENERIC);
        dest_fun->add_source_type(source_of_dest_fun);
        dest_fun->add_dest_type(TYPE_GENERIC);

        // String -> (`a -> `b)
        fun->add_source_type(TYPE_STRING);
        fun->add_dest_type(dest_fun);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_of_dest_fun, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));

        fun->add_constraint(particlelike);
        break;
    }
    case ADD_CORRECTIONLIB:
        assert(false);
    case CREATE_MASK:
    {
        // List<ParticleInstance> -> Mask
        auto source_part_list = std::make_shared<Type>(TYPE_LIST);
        source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
        fun->add_source_type(source_part_list);
        fun->add_dest_type(TYPE_MASK);
        break;
    }
    case LIMIT_MASK:
    {
        // Mask x List<Cond> -> Mask
        fun->add_source_type(TYPE_MASK);

        auto source_cond_list = std::make_shared<Type>(TYPE_LIST);
        source_cond_list->add_dest_type(TYPE_COND);
        fun->add_source_type(source_cond_list);
        fun->add_dest_type(TYPE_MASK);
        break;
    }
    case APPLY_MASK:
    {
        // Mask x List<ParticleInstance> -> List<ParticleInstance>
        fun->add_source_type(TYPE_MASK);

        auto source_part_list = std::make_shared<Type>(TYPE_LIST);
        source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
        fun->add_source_type(source_part_list);

        // reuse the list
        fun->add_dest_type(source_part_list);
        break;
    }

    case USE_HIST:
        fun->add_source_type(TYPE_HIST);
        fun->add_source_type(TYPE_REGION);
        fun->add_dest_type(TYPE_ERROR);
        break;
    case USE_HIST_LIST:
    case HIST_1D:
        fun->add_source_type(TYPE_STRING);

        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);

        fun->add_dest_type(TYPE_HIST);
        break;
    case HIST_2D:
        fun->add_source_type(TYPE_STRING);

        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);

        fun->add_dest_type(TYPE_HIST);
        break;

    case WEIGHT_APPLY:
        assert(false);
    case DO_CUTFLOW_ON_REGION:
    case DO_EVENTLIST_ON_REGION:
        // Untypable function since it has no codomain. This will be treated as a ->Error function to show that it cannot and should not be typed
        fun->add_source_type(TYPE_REGION);
        fun->add_dest_type(TYPE_ERROR);
        break;

    case CREATE_HIST_LIST:
    case ADD_HIST_TO_LIST:
    case CREATE_BIN:
    case CREATE_TABLE:
    case CREATE_TABLE_VALUE:
    case CREATE_TABLE_LOWER_BOUNDS:
    case CREATE_TABLE_UPPER_BOUNDS:
    case APPEND_TO_TABLE:
    case FINISH_TABLE:
        assert(false);
        break;
    case BEGIN_EXPRESSION:
        // () -> `a
        fun->add_dest_type(TYPE_GENERIC);
        break;
    case BEGIN_IF:
    case END_IF:
        assert(false);
    case SORT_ASCEND: case SORT_DESCEND:
        {
        // List<ParticleInstance> x List<Number> -> List<ParticleInstance>
        auto source_part_list = std::make_shared<Type>(TYPE_LIST);
        source_part_list->add_dest_type(TYPE_PARTICLEINSTANCE);
        fun->add_source_type(source_part_list);

        auto source_number_list = std::make_shared<Type>(TYPE_LIST);
        source_number_list->add_dest_type(TYPE_NUMBER);
        fun->add_source_type(source_number_list);

        // reuse the list
        fun->add_dest_type(source_part_list);
        break;
    }
    
    case EXPR_RAISE:
    {
        // `a x Number -> `b
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(source_type);

        // | `a = `b
        // Constraint equality;
        // equality.add_conclusion(Statement(STATEMENT_EQUALITY, source_type, dest_type));

        // fun->add_constraint(equality);

        // | `a <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
        
        fun->add_constraint(numeric);
        break;
    }
    case EXPR_MULTIPLY:
    case EXPR_DIVIDE:
    case EXPR_ADD:
    case EXPR_SUBTRACT:
    case EXPR_AMPERSAND:
    case EXPR_PIPE:
    {
        
        // `a <<: Number, `b <<: Number -> {if `a = `b then `a else if `a = Number then `b else if `b = Number then `a else Error}
        

        // `a x `b -> `c
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);

        auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_a);
        fun->add_source_type(source_type_b);
        fun->add_dest_type(dest_type_c);

        // | `a <<: Number
        // | `b <<: Number
        Constraint numeric;

        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));

        fun->add_constraint(numeric);

        // | `a = `b ==> `c = `a
        Constraint primary_secondary_equality;
        primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
        primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));

        fun->add_constraint(primary_secondary_equality);

        // | `a =/= `b and `a = Number ==> `c = `b
        Constraint primary_single_number;
        // primary_single_number.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
        primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        primary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_b));

        fun->add_constraint(primary_single_number);

        // | `a =/= `b and `a =/= Number and `b = Number ==> `c = `a
        Constraint secondary_single_number;
        secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
        secondary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
        

        fun->add_constraint(secondary_single_number);

        // | `a =/= `b and `a =/= Number and `b =/= Number then `c = Error
        Constraint error_condition;
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
        error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));

        fun->add_constraint(error_condition);

        break;
    }

    case EXPR_LT:
    case EXPR_LE:
    case EXPR_GT:
    case EXPR_GE:
    case EXPR_EQ:
    case EXPR_NE:
    {
        // `a x `b -> `c
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);

        auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_a);
        fun->add_source_type(source_type_b);
        fun->add_dest_type(dest_type_c);

        // | `a <<: Number
        // | `b <<: Number
        Constraint numeric;

        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));

        fun->add_constraint(numeric);

        // | `c <<: Cond

        Constraint condition;
        condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE_COND)));

        fun->add_constraint(condition);

        // `a = `b ==> `c ~=~ `a
        Constraint primary_secondary_equality;
        primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
        primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));

        fun->add_constraint(primary_secondary_equality);

        //`a = Number ==> `c ~=~ `b
        Constraint primary_single_number;
        primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        primary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_b));

        fun->add_constraint(primary_single_number);

        // `b = Number ==> `c ~=~ `a
        Constraint secondary_single_number;
        secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
        secondary_single_number.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, dest_type_c, source_type_a));
        
        fun->add_constraint(secondary_single_number);

        // `a =/= `b and `a =/= Number and `b =/= Number ==> `c = Error
        Constraint error_condition;
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_NUMBER)));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_NUMBER)));
        error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));

        fun->add_constraint(error_condition);
        break;
    }

    case EXPR_AND:
    case EXPR_OR:
{
        
        // `a <<: Cond, `b <<: Cond -> {if `a = `b then `a else if `a = Cond then `b else if `b = Cond then `a else Error}
        

        // `a x `b -> `c
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);

        auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_a);
        fun->add_source_type(source_type_b);
        fun->add_dest_type(dest_type_c);

        // | `a <<: Cond
        // | `b <<: Cond
        Constraint condition;

        condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
        condition.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_COND)));

        fun->add_constraint(condition);

        // | `a = `b ==> `c = `a
        Constraint primary_secondary_equality;
        primary_secondary_equality.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, source_type_b));
        primary_secondary_equality.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));

        fun->add_constraint(primary_secondary_equality);

        // | `a =/= `b and `a = Cond ==> `c = `b
        Constraint primary_single_number;
        primary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
        primary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_b));

        fun->add_constraint(primary_single_number);

        // | `a =/= `b and `a =/= Cond and `b = Cond ==> `c = `a
        Constraint secondary_single_number;
        secondary_single_number.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_COND)));
        secondary_single_number.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, source_type_a));
        

        fun->add_constraint(secondary_single_number);

        // | `a =/= `b and `a =/= Cond and `b =/= Cond then `c = Error
        Constraint error_condition;
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, source_type_b));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_COND)));
        error_condition.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_COND)));
        error_condition.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));

        fun->add_constraint(error_condition);

        break;
    }

    case EXPR_INDEX:
    {
        // List<`a> x Number -> `a
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
        auto source_list = std::make_shared<Type>(TYPE_LIST);
        source_list->add_dest_type(element_type);

        fun->add_source_type(source_list);
        fun->add_source_type(Type::fundamental_type_instance(TYPE_NUMBER));
        fun->add_dest_type(element_type);
        break;
    }

    case EXPR_WITHIN:
    case EXPR_WITHIN_EXCLUSIVE:
    case EXPR_WITHIN_LEFT_EXCLUSIVE:
    case EXPR_WITHIN_RIGHT_EXCLUSIVE:
    case EXPR_OUTSIDE:
    case EXPR_NEGATE:
    case EXPR_LOGICAL_NOT:
    case FUNC_GEN_PART_IDX:
    case FUNC_CHARGE:
    case FUNC_BTAG:
        assert(false);
        break;
    case FUNC_PT:
    case FUNC_ETA:
    case FUNC_RAPIDITY:
    case FUNC_PHI:
    case FUNC_MASS:
    case FUNC_ENERGY:
    case FUNC_MSOFTDROP:
    case FUNC_THETA:
    case FUNC_ABS_ISO:
    case FUNC_MINI_ISO:
    {
        // `a <<: ParticleInstance -> `b <<: Number

        // `a -> `b

        auto source_type = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_dest_type(dest_type);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));

        fun->add_constraint(particlelike);

        // | `b <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE_NUMBER)));

        fun->add_constraint(numeric);

        // | `a ~=~ `b
        Constraint equal_depth;
        equal_depth.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type, dest_type));

        fun->add_constraint(equal_depth);
        break;
    }


    case FUNC_DR:
    case FUNC_DPHI:
    case FUNC_DETA:
        // `a Particlelike, `b Particlelike -> {If `a < ParticleInstance then `b else if `b < ParticleInstance then `a else if `a < Particle and `b < Particle then ParticleMatrix else Error}

    {
        // `a x `b -> `c
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type_c = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_a);
        fun->add_source_type(source_type_b);
        fun->add_dest_type(dest_type_c);

        // | `a <<: ParticleInstance
        Constraint particlelike_a;
        particlelike_a.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_a);

        // | `b <<: ParticleInstance
        Constraint particlelike_b;
        particlelike_b.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_b);

        // | `c <<: Number
        Constraint numeric_c;
        numeric_c.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type_c, Type::fundamental_type_instance(TYPE_NUMBER)));
        fun->add_constraint(numeric_c);

        // | `a = ParticleInstance ==> `b ~=~ `c
        Constraint a_is_single;
        a_is_single.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        a_is_single.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type_b, dest_type_c));
        fun->add_constraint(a_is_single);

        // | `b = ParticleInstance ==> `a ~=~ `c
        Constraint b_is_single;
        b_is_single.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        b_is_single.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type_a, dest_type_c));
        fun->add_constraint(b_is_single);

        // | `a = List<ParticleInstance> and `b = List<ParticleInstance> ==> `c = List<List<Number>>
        auto list_particle_instance = std::make_shared<Type>(TYPE_LIST);
        list_particle_instance->add_dest_type(TYPE_PARTICLEINSTANCE);

        auto list_list_number = std::make_shared<Type>(TYPE_LIST);
        auto inner_list_number = std::make_shared<Type>(TYPE_LIST);
        inner_list_number->add_dest_type(TYPE_NUMBER);
        list_list_number->add_dest_type(inner_list_number);

        Constraint both_lists;
        both_lists.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, list_particle_instance));
        both_lists.add_premise(Statement(STATEMENT_EQUALITY, source_type_b, list_particle_instance));
        both_lists.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, list_list_number));
        fun->add_constraint(both_lists);

        // | `a =/= ParticleInstance and `b =/= ParticleInstance and `a =/= List<ParticleInstance> ==> `c = Error
        Constraint error_condition_a;
        error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        error_condition_a.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, list_particle_instance));
        error_condition_a.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
        fun->add_constraint(error_condition_a);

        // | `a =/= ParticleInstance and `b =/= ParticleInstance and `b =/= List<ParticleInstance> ==> `c = Error
        Constraint error_condition_b;
        error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        error_condition_b.add_premise(Statement(STATEMENT_INEQUALITY, source_type_b, list_particle_instance));
        error_condition_b.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_c, Type::fundamental_type_instance(TYPE_ERROR)));
        fun->add_constraint(error_condition_b);
        break;
    }

    case FUNC_DISTINCT:
    case FUNC_DR_HADAMARD:
    case FUNC_DPHI_HADAMARD:
    case FUNC_DETA_HADAMARD:
        // `a Particlelike x `a Particlelike -> `b Numeric : `b ~=~ `a
    {
        // `a x `a -> `b
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_source_type(source_type);
        fun->add_dest_type(dest_type);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);

        // | `b <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, dest_type, Type::fundamental_type_instance(TYPE_NUMBER)));
        fun->add_constraint(numeric);

        // | `a ~=~ `b
        Constraint equal_depth;
        equal_depth.add_conclusion(Statement(STATEMENT_EQUAL_DEPTH, source_type, dest_type));
        fun->add_constraint(equal_depth);
        break;
    }

    case FUNC_SIZE:
    {
        // List<`a> -> Number
        auto source_list = std::make_shared<Type>(TYPE_LIST);
        source_list->add_dest_type(TYPE_GENERIC);
        fun->add_source_type(source_list);
        fun->add_dest_type(TYPE_NUMBER);
        break;
    }
    case FUNC_ANYOF:
    case FUNC_ALLOF:
        assert(false);

    case FUNC_SQRT:
    case FUNC_ABS:
    case FUNC_COS:
    case FUNC_SIN:
    case FUNC_TAN:
    case FUNC_SINH:
    case FUNC_COSH:
    case FUNC_TANH:
    case FUNC_EXP:
    case FUNC_LOG:
        // `a Numeric -> `a
    {
        // `a -> `a
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_dest_type(source_type);

        // | `a <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
        fun->add_constraint(numeric);
        break;
    }
    case FUNC_AVE:
    case FUNC_SUM:
    case FUNC_MIN:
    case FUNC_MAX:
        // List<`a Numeric> -> `a

    {        
        // List<`a> -> `a
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
        auto source_list = std::make_shared<Type>(TYPE_LIST);
        source_list->add_dest_type(element_type);

        fun->add_source_type(source_list);
        fun->add_dest_type(element_type);

        // | `a <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_NUMBER)));
        fun->add_constraint(numeric);
        break;
    }
    case FUNC_MAX_LIST:
    case FUNC_MIN_LIST:
        
    {
        // `a x `a -> `a
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_source_type(source_type);
        fun->add_dest_type(source_type);

        // | `a <<: Number
        Constraint numeric;
        numeric.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_NUMBER)));
        fun->add_constraint(numeric);
        break;
    }
    
    case FUNC_ANYOCCURRENCES:
    case FUNC_FIRST:
    case FUNC_SECOND:
    case FUNC_SORT_ASCEND:
    case FUNC_SORT_DESCEND:
    case FUNC_FLAVOR:
    case FUNC_CONSTITUENTS:
    case FUNC_PDG_ID:
    case FUNC_JET_ID:
    case FUNC_TAUTAG:
    case FUNC_CTAG:
    case FUNC_DXY:
    case FUNC_DZ:
    case FUNC_IS_TIGHT:
    case FUNC_IS_MEDIUM:
    case FUNC_IS_LOOSE:

        assert(false);

    case FUNC_NAMED:

    {
        // `a x (`b->`c) -> `d
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto type_b = std::make_shared<Type>(TYPE_GENERIC);
        auto type_c = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type_d = std::make_shared<Type>(TYPE_GENERIC);

        // (`b -> `c)
        auto func_type = std::make_shared<Type>(TYPE_FUNCTION);
        func_type->add_source_type(type_b);
        func_type->add_dest_type(type_c);

        fun->add_source_type(source_type_a);
        fun->add_source_type(func_type);
        fun->add_dest_type(dest_type_d);

        Constraint subtype; 
        subtype.add_conclusion(Statement(STATEMENT_SUBTYPE, source_type_a, type_b));
        fun->add_constraint(subtype);

        // // | `c <: `d //
        // Constraint second_subtype;
        // second_subtype.add_conclusion(Statement(STATEMENT_SUBTYPE, type_c, dest_type_d));
        // fun->add_constraint(second_subtype);


        // | (`a = `b => `c = `d)
        Constraint implication_of_origin;
        implication_of_origin.add_premise(Statement(STATEMENT_EQUALITY, source_type_a, type_b));
        implication_of_origin.add_conclusion(Statement(STATEMENT_EQUALITY, type_c, dest_type_d));
        fun->add_constraint(implication_of_origin);

        break;
    
    }


    case MAKE_EMPTY_PARTICLE:
    {
        // () -> `a
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
        // auto dest_list = std::make_shared<Type>(TYPE_LIST);
        // dest_list->add_dest_type(element_type);

        fun->add_dest_type(element_type);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);
        break;
    }

    case MAKE_EMPTY_UNION:
        // () -> Union
        fun->add_dest_type(TYPE_UNION);
        break;
    case ADD_NAMED_TO_UNION:
    case ADD_ELECTRON_TO_UNION:
    case ADD_MUON_TO_UNION:
    case ADD_TAU_TO_UNION:
    case ADD_TRACK_TO_UNION:
    case ADD_PHOTON_TO_UNION:
    case ADD_QGJET_TO_UNION:
    case ADD_METLV_TO_UNION:
    case ADD_GEN_TO_UNION:
    case ADD_JET_TO_UNION:
    case ADD_FJET_TO_UNION:
       // Union -> Union
        fun->add_source_type(TYPE_UNION);
        fun->add_dest_type(TYPE_UNION);
        break;

    case MAKE_EMPTY_COMB:
        // () -> Comb
        fun->add_dest_type(TYPE_COMB);
        break;    case ADD_NAMED_TO_COMB:
    case ADD_ELECTRON_TO_COMB:
    case ADD_MUON_TO_COMB:
    case ADD_TAU_TO_COMB:
    case ADD_TRACK_TO_COMB:
    case ADD_PHOTON_TO_COMB:
    case ADD_QGJET_TO_COMB:
    case ADD_METLV_TO_COMB:
    case ADD_GEN_TO_COMB:
    case ADD_JET_TO_COMB:
    case ADD_FJET_TO_COMB:
        // Comb -> Comb
        fun->add_source_type(TYPE_COMB);
        fun->add_dest_type(TYPE_COMB);
        break;

    case NAME_ELEMENT_OF_COMB:
        //TODO:
        assert(false);

    case MAKE_EMPTY_DISJOINT:
    case ADD_NAMED_TO_DISJOINT:
    case ADD_ELECTRON_TO_DISJOINT:
    case ADD_MUON_TO_DISJOINT:
    case ADD_TAU_TO_DISJOINT:
    case ADD_TRACK_TO_DISJOINT:
    case ADD_PHOTON_TO_DISJOINT:
    case ADD_QGJET_TO_DISJOINT:
    case ADD_METLV_TO_DISJOINT:
    case ADD_GEN_TO_DISJOINT:
    case ADD_JET_TO_DISJOINT:
    case ADD_FJET_TO_DISJOINT:
        // Disjoint -> Disjoint
        fun->add_source_type(TYPE_DISJOINT);
        fun->add_dest_type(TYPE_DISJOINT);
        break;


    case NAME_ELEMENT_OF_DISJOINT:
    //TODO:
    assert(false);

    case ADD_PART_NAMED:
    case SUB_PART_NAMED:
    {
        // List<ParticleInstance> x List<ParticleInstance> -> List<ParticleInstance>
        auto element_type = std::make_shared<Type>(TYPE_LIST);
        // auto named_type = std::make_shared<Type>(TYPE_GENERIC);

        element_type->add_dest_type(TYPE_PARTICLEINSTANCE);

        fun->add_source_type(element_type);
        fun->add_source_type(element_type);
        // fun->add_source_type(named_type);
        fun->add_dest_type(element_type);

        // // | `a <<: ParticleInstance
        // // | `b <<: ParticleInstance
        // Constraint particlelike;
        // particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        // particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, named_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        // fun->add_constraint(particlelike);
        break;        
    }
    case ADD_PART_ELECTRON:
    case ADD_PART_MUON:
    case ADD_PART_TAU:
    case ADD_PART_TRACK:
    case ADD_PART_PHOTON:
    case ADD_PART_QGJET:
    case ADD_PART_METLV:
    case ADD_PART_GEN:
    case ADD_PART_JET:
    case ADD_PART_FJET:



    case SUB_PART_ELECTRON:
    case SUB_PART_MUON:
    case SUB_PART_TAU:
    case SUB_PART_TRACK:
    case SUB_PART_PHOTON:
    case SUB_PART_QGJET:
    case SUB_PART_METLV:
    case SUB_PART_GEN:
    case SUB_PART_JET:
    case SUB_PART_FJET:
    {
        // List<ParticleInstance> -> List<ParticleInstance> 
        auto element_type = std::make_shared<Type>(TYPE_LIST);
        element_type->add_dest_type(TYPE_PARTICLEINSTANCE);
        // auto list_type = std::make_shared<Type>(TYPE_LIST);
        // list_type->add_dest_type(element_type);

        fun->add_source_type(element_type);
        fun->add_dest_type(element_type);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);
        break;
    }

    case ADD_PART_ELECTRON_INDEXED:
    case ADD_PART_MUON_INDEXED:
    case ADD_PART_TAU_INDEXED:
    case ADD_PART_TRACK_INDEXED:
    case ADD_PART_PHOTON_INDEXED:
    case ADD_PART_QGJET_INDEXED:
    case ADD_PART_METLV_INDEXED:
    case ADD_PART_GEN_INDEXED:
    case ADD_PART_JET_INDEXED:
    case ADD_PART_FJET_INDEXED:

    case SUB_PART_ELECTRON_INDEXED:
    case SUB_PART_MUON_INDEXED:
    case SUB_PART_TAU_INDEXED:
    case SUB_PART_TRACK_INDEXED:
    case SUB_PART_PHOTON_INDEXED:
    case SUB_PART_QGJET_INDEXED:
    case SUB_PART_METLV_INDEXED:
    case SUB_PART_GEN_INDEXED:
    case SUB_PART_JET_INDEXED:
    case SUB_PART_FJET_INDEXED:
    {
        // ParticleInstance x List<ParticleInstance> x Number -> ParticleInstance
        auto source_type = std::make_shared<Type>(TYPE_LIST);
        source_type->add_dest_type(TYPE_PARTICLEINSTANCE);

        fun->add_source_type(TYPE_PARTICLEINSTANCE);
        fun->add_source_type(source_type);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(TYPE_PARTICLEINSTANCE);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);
        break;
    }

    case ADD_PART_NAMED_INDEXED:
    case SUB_PART_NAMED_INDEXED:
    {
        // `b x `a x Number -> `a where `a <<: ParticleInstance and `b <<: ParticleInstance
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_b);
        fun->add_source_type(source_type_a);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(source_type_a);

        // | `a <<: ParticleInstance
        Constraint particlelike_a;
        particlelike_a.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_a);

        // | `b <<: ParticleInstance
        Constraint particlelike_b;
        particlelike_b.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_b);
        break;
    }

    case ADD_PART_ELECTRON_RANGE:
    case ADD_PART_MUON_RANGE:
    case ADD_PART_TAU_RANGE:
    case ADD_PART_TRACK_RANGE:
    case ADD_PART_PHOTON_RANGE:
    case ADD_PART_QGJET_RANGE:
    case ADD_PART_METLV_RANGE:
    case ADD_PART_GEN_RANGE:
    case ADD_PART_JET_RANGE:
    case ADD_PART_FJET_RANGE:

    case SUB_PART_ELECTRON_RANGE:
    case SUB_PART_MUON_RANGE:
    case SUB_PART_TAU_RANGE:
    case SUB_PART_TRACK_RANGE:
    case SUB_PART_PHOTON_RANGE:
    case SUB_PART_QGJET_RANGE:
    case SUB_PART_METLV_RANGE:
    case SUB_PART_GEN_RANGE:
    case SUB_PART_JET_RANGE:
    case SUB_PART_FJET_RANGE:
    {
        // `a x Number x Number -> `a where `a <<: ParticleInstance
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(source_type);

        // | `a <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);
        break;
    }

    case ADD_PART_NAMED_RANGE:
    case SUB_PART_NAMED_RANGE:
    { 
        // `b x `a x Number x Number -> `a where `a <<: ParticleInstance and `b <<: ParticleInstance
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto source_type_b = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type_b);
        fun->add_source_type(source_type_a);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(source_type_a);

        // | `a <<: ParticleInstance
        Constraint particlelike_a;
        particlelike_a.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_a, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_a);

        // | `b <<: ParticleInstance
        Constraint particlelike_b;
        particlelike_b.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, source_type_b, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike_b);
        break;
    }


    }

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
                first_representative->print();
                std::cout  << "---";
                second_representative->print();
                std::cout << std::endl;
                assert(first_representative->get_base_type() == second_representative->get_base_type());
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

void PartialOrder::add_subtype(PType sub, PType super){
        ensure_exists(sub);
        ensure_exists(super);

        // a <: a always
        if (sub == super) return;

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
                assert(sub->get_base_type() == super->get_base_type());
            }
        }

        // one is now a subtype of the other
        supertypes[sub].insert(super);
        subtypes[super].insert(sub);
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



void Typer::equality_of_types(PType first, PType second) {

    equiv.union_of_classes(first, second);
}


void Typer::subtype_of_types(PType sub, PType super) {
    subtyping.add_subtype(sub, super);
}

void Typer::hereditary_subtype_of_types(PType sub, PType super) {
    hereditary_subtyping.add_subtype(sub, super);
}





Ternary Typer::truth_of_premise(PType lhs, PType rhs, StatementForm form) {
    switch (form) {
        case STATEMENT_EQUALITY:
            if (lhs == rhs) {
                // manifestly equal - this premise is satisfied
                return Ternary::TERN_TRUE;
            } else if (false) {
                // manifestly unequal - certainly unsatisfied
                return Ternary::TERN_FALSE;
            } else {
                // not manifestly equal - this is unknown
                return Ternary::TERN_UNKNOWN;
            }
        case STATEMENT_INEQUALITY: 
            if (lhs == rhs) {
                // if we have established their equality, then we certainly have violated this premise. This premise is manifestly unsatisfied.
                return Ternary::TERN_FALSE;
            } else if (false) {
                // manifestly unequal from prior knowledge
                return Ternary::TERN_TRUE;
            } else {
                // undeterminable
                return Ternary::TERN_UNKNOWN;
            }
        case STATEMENT_SUBTYPE:
            if (subtyping.is_subtype(lhs, rhs)) {
                return Ternary::TERN_TRUE;
            } else if (false) {
                return Ternary::TERN_FALSE;
            } else {
                return Ternary::TERN_UNKNOWN;
            }
        case STATEMENT_NONSUBTYPE:
        case STATEMENT_SUPERTYPE:
        case STATEMENT_NONSUPERTYPE:
            return Ternary::TERN_UNKNOWN;
        case STATEMENT_HEREDITARY_SUBTYPE:
            if (hereditary_subtyping.is_subtype(lhs, rhs)) {
                return Ternary::TERN_TRUE;
            } else if (false) {
                return Ternary::TERN_FALSE;
            } else {
                return Ternary::TERN_UNKNOWN;
            } break;            
        case STATEMENT_HEREDITARY_SUPERTYPE:
        case STATEMENT_EQUAL_DEPTH:
            return Ternary::TERN_UNKNOWN;
    }
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
            std::cout << "Rejecting a premise:" << std::endl;
            constraint.print();
        }

        if (has_true_premises == Ternary::TERN_UNKNOWN) {
            std::cout << "A premise is unknown:" << std::endl;
            new_running_valid_constraints.push_back(constraint);
        }

        if (has_true_premises != Ternary::TERN_TRUE) {
            continue;
        }

        for (auto conclusion : constraint.get_conclusions()) {

            auto first = conclusion.get_lhs();
            auto second = conclusion.get_rhs();

            if (conclusion.get_form() == STATEMENT_EQUALITY) {
                std::cout <<"equaling " << i <<"\n";
                equality_of_types(first, second);
            // } else if (conclusion.get_form() == STATEMENT_SUBTYPE) {
            //     subtype_of_types(first, second);
            // } else if (conclusion.get_form() == STATEMENT_HEREDITARY_SUBTYPE) {
            //     hereditary_subtype_of_types(first, second);
            } else {
                // the consequent is not something we are equipped to deal wwith yet, we add it and only it
                // remaining constraints are constraints of exclusion - not much to be done with them at this point
                Constraint consequent;
                consequent.add_conclusion(conclusion);
                new_running_valid_constraints.push_back(consequent);
            }

        }
    }

    auto equalities = equiv.resolve_all();

    std::cout << "\n Constraints resolved.\n";

    for (auto variable : order_of_variables) {
        auto type_of_var = types_of_variables[variable];

        if (used_variables.count(variable) == 0) {
            std::cout << "UNUSED ";
        }

        std::cout << variable << " : ";

        type_of_var = equalities[type_of_var];
        type_of_var->print(equalities);
        
        std::cout << "\n";
    }
    std::cout << std::endl;

    for (auto constraint : new_running_valid_constraints) {
        constraint.print(equalities);
        std::cout << "\n";
    }

}

void Typer::collect_existing_constraints() {

    std::regex reg_string;
    std::regex reg_number;

    reg_number = std::regex("-{0,1}[0-9]*\\.{0,1}[0-9]*([Ee][-+]{0,1}[0-9]+){0,1}");
    reg_string= std::regex("\"[^\"]*\"");

    while (alil->clear_to_next()) {
        AnalysisCommand command = alil->next_command();

        auto type_of_function = command_handle(command);
        type_of_function->print();

        // all the constraints this type comes with, we add to ours
        for (auto constraint : type_of_function->get_constraints()) {
            running_valid_constraints.push_back(constraint);
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
                    // TODO:error condition - variable is undefined
                    std::cerr << arg << std::endl;
                    assert(false);
                }
            } else {
                type_of_arg = types_of_variables[arg];
            }
            Constraint equality_of_input;
            if (type_of_function->get_num_of_sources() <= i) {
                std::cerr << type_of_function->get_num_of_sources();
                //TODO: real error;
                assert(false);
            } 
            equality_of_input.add_conclusion(Statement(STATEMENT_EQUALITY, type_of_arg, type_of_function->get_source_type(i)));
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

    resolve_constraints();

    std::cout << "\n\n\n" << "Second pass \n" << std::endl;

    resolve_constraints();

    // while (alil->clear_to_next()) {
    //     auto out = command_handle(alil->next_command());
    //     out->print();
    //     std::cout << std::endl;

        
    // }
}

