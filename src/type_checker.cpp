
#include "type_checker.hpp"
#include "ali_converter.hpp"
#include "node.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>


std::unordered_map<BaseType, std::shared_ptr<Type>> Type::base_type_instances_map;
std::unordered_map<std::shared_ptr<Type>, int> Type::generic_map;
int Type::highest_mapped_generic;


void Constraint::add_premise(Statement statement) {
    premises.push_back(statement);
}
void Constraint::add_conclusion(Statement statement) {
    conclusions.push_back(statement);
}

void Constraint::print() {
    bool has_premises = false;
    bool first = true;
    for (auto premise : premises) {
        if (first) {
            first = false;
        } else {
            std::cout << " and ";
        }
        premise.print();
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
        conclusion.print();
    }
}

Statement::Statement(StatementForm form_in, std::shared_ptr<Type> type1, std::shared_ptr<Type> type2) : form(form_in), lhs(type1), rhs(type2) {}

std::shared_ptr<Type> Type::fundamental_type_instance(BaseType bt) {
    // no non-fundamental-types should have this be called on it - lists, functions always have children, and generic is not a single type
    assert(bt != TYPE_GENERIC);
    assert(bt != TYPE_FUNCTION);
    assert(bt != TYPE_LIST);

    if (base_type_instances_map.count(bt) == 0) {
        base_type_instances_map.emplace(bt, std::make_shared<Type>(bt));
    }
    return base_type_instances_map[bt];
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

Type::Type(BaseType in) : this_type(in), has_dest_type(false) {}

BaseType Type::get_base_type() {
    return this_type;
}

void Type::add_source_type(std::shared_ptr<Type> type) {
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

void Type::add_dest_type(std::shared_ptr<Type> type) {
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

void Type::print() {

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
            it->print();
        }
        std::cout << " -> ";
        dest_type->print();
        std::cout << ")";
    } else if (this_type == TYPE_LIST) {
        std::cout << "<";
        dest_type->print();
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

std::shared_ptr<Type> Typer::command_handle(AnalysisCommand in) {

    in.print_instruction();

    std::shared_ptr<Type> fun(std::make_shared<Type>(TYPE_FUNCTION));
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
        // `a -> `b
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_dest_type(dest_type);

        // | `a = `b
        Constraint equality;
        equality.add_conclusion(Statement(STATEMENT_EQUALITY, source_type, dest_type));

        fun->add_constraint(equality);
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
        // Mask x Cond -> Mask
        fun->add_source_type(TYPE_MASK);
        fun->add_source_type(TYPE_COND);
        fun->add_dest_type(TYPE_MASK);
        break;
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
        // List<ParticleInstance> x List<Number> -> List<ParticleInstance
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
        fun->add_dest_type(dest_type);

        // | `a = `b
        Constraint equality;
        equality.add_conclusion(Statement(STATEMENT_EQUALITY, source_type, dest_type));

        fun->add_constraint(equality);

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
    case EXPR_LT:
    case EXPR_LE:
    case EXPR_GT:
    case EXPR_GE:
    case EXPR_EQ:
    case EXPR_NE:
    case EXPR_AMPERSAND:
    case EXPR_PIPE:
    case EXPR_AND:
    case EXPR_OR:
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

    case EXPR_INDEX:
    {
        // List<`a> -> `a
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
        auto source_list = std::make_shared<Type>(TYPE_LIST);
        source_list->add_dest_type(element_type);

        fun->add_source_type(source_list);
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
        // `a<`c, (`c -> `b) -> `b

    {
        // `a x (`c -> `b) -> `d
        auto source_type_a = std::make_shared<Type>(TYPE_GENERIC);
        auto type_b = std::make_shared<Type>(TYPE_GENERIC);
        auto type_c = std::make_shared<Type>(TYPE_GENERIC);
        auto dest_type_d = std::make_shared<Type>(TYPE_GENERIC);

        // (`c -> `b)
        auto func_type = std::make_shared<Type>(TYPE_FUNCTION);
        func_type->add_source_type(type_c);
        func_type->add_dest_type(type_b);

        fun->add_source_type(source_type_a);
        fun->add_source_type(func_type);
        fun->add_dest_type(dest_type_d);

        // | `a <: `c
        Constraint subtype;
        subtype.add_conclusion(Statement(STATEMENT_SUBTYPE, source_type_a, type_c));
        fun->add_constraint(subtype);

        // | `d = `b
        Constraint equality;
        equality.add_conclusion(Statement(STATEMENT_EQUALITY, dest_type_d, type_b));
        fun->add_constraint(equality);
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
        // `a x `b -> `a where `a <<: ParticleInstance and `b <<: ParticleInstance
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
        auto named_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(element_type);
        fun->add_source_type(named_type);
        fun->add_dest_type(element_type);

        // | `a <<: ParticleInstance
        // | `b <<: ParticleInstance
        Constraint particlelike;
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, element_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        particlelike.add_conclusion(Statement(STATEMENT_HEREDITARY_SUBTYPE, named_type, Type::fundamental_type_instance(TYPE_PARTICLEINSTANCE)));
        fun->add_constraint(particlelike);
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
        // `a -> `a where `a <<: ParticleInstance
        auto element_type = std::make_shared<Type>(TYPE_GENERIC);
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
    
    
    
    }

    return fun;

}



void Typer::print() {

        Type::highest_mapped_generic = 0;
        Type::generic_map.clear();

        while (alil->clear_to_next()) {
        auto out = command_handle(alil->next_command());
        out->print();
        std::cout << std::endl;

        
    }
}

