
#include "type_checker.hpp"
#include "ali_converter.hpp"
#include "node.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
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


    if (substitutions.count(shared_from_this()) != 0) {
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

        // // | `a <: `c
        // Constraint subtype;
        // subtype.add_conclusion(Statement(STATEMENT_SUBTYPE, source_type_a, type_c));
        // fun->add_constraint(subtype);

        Constraint subtype_fake_as_equality; //TODO: once subtyping is implemented, change
        subtype_fake_as_equality.add_conclusion(Statement(STATEMENT_EQUALITY, source_type_a, type_c));
        fun->add_constraint(subtype_fake_as_equality);

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
        // `a x Number -> `a where `a <<: ParticleInstance
        auto source_type = std::make_shared<Type>(TYPE_GENERIC);

        fun->add_source_type(source_type);
        fun->add_source_type(TYPE_NUMBER);
        fun->add_dest_type(source_type);

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





void Typer::equality_of_types(std::unordered_map<PType, PType> &equalities, PType first, PType second) {

    if (first->get_base_type() != TYPE_GENERIC && second->get_base_type() != TYPE_GENERIC) {
        if (first->get_base_type() != second->get_base_type()) {
            assert(false);
        } else if (first->get_base_type() == TYPE_FUNCTION) {
            for (int i = 0; i < first->get_num_of_sources(); i++) {
                equality_of_types(equalities, first->get_source_type(i), second->get_source_type(i));
            }
        }  

        if (first->get_base_type() == TYPE_FUNCTION || first->get_base_type() == TYPE_LIST) {
            equality_of_types(equalities, first->get_dest_type(), second->get_dest_type());
        }

    } else if (first->get_base_type() != TYPE_GENERIC) {
            equalities.emplace(second, first);
    } else if (second->get_base_type() != TYPE_GENERIC) {
            equalities.emplace(first, second);
    } else {
        if (equalities.count(second) != 0) {
            equalities.emplace(first, equalities[second]);
        } else if (equalities.count(first) != 0) {
            equalities.emplace(second, equalities[first]);
        } else {
            equalities.emplace(second, first);
        }

    }
}


// create sets of equivalence classes - any one equal to any other will have their equivalence classes merged
void transitive_closure_equality() {
    
}

// using Warshall's algorithm, we take any types that are natively subtyped from each other, and compute whether any node i is upstrean of node j (i.e. i <: j)
void transitive_closure_hereditary_subtype() {

}

// we similarly compute for hereditary subtyping, using the fact that a <: b ==> a <<: b




void Typer::resolve_constraints() {
    std::unordered_map<PType, PType> equalities;

    std::vector<Constraint> new_running_valid_constraints;

    for (auto constraint : running_valid_constraints) {
        bool has_true_premises = true;
        for (auto premise : constraint.get_premises()) {
            auto first = premise.get_lhs();
            auto second = premise.get_rhs();

            auto true_first = equalities.count(first) != 0 ? equalities[first] : first;
            auto true_second = equalities.count(second) != 0 ? equalities[second] : second;
 
            if (true_first->is_fundamental_type() && true_second->is_fundamental_type()) {
                if (true_first->get_base_type() != true_second->get_base_type()) has_true_premises = false;
            } else if (true_first != true_second) {
            // this has failed for a reason that is not fully determined - add it back to the constraints list
                has_true_premises = false;
                new_running_valid_constraints.push_back(constraint);
            }
            

        }

        if (!has_true_premises) continue;

        for (auto conclusion : constraint.get_conclusions()) {

            auto first = conclusion.get_lhs();
            auto second = conclusion.get_rhs();

            if (conclusion.get_form() == STATEMENT_EQUALITY) {
                equality_of_types(equalities, first, second);
            } else {
                new_running_valid_constraints.push_back(constraint);
            }

        }
    }

    for (auto variable : order_of_variables) {
        auto type_of_var = types_of_variables[variable];

        if (used_variables.count(variable) == 0) {
            std::cout << "UNUSED ";
        }

        std::cout << variable << " : ";

        while (equalities.count(type_of_var) != 0) {
            type_of_var = equalities[type_of_var];
            // equalities[type_of_var]->print();    
        } 
            type_of_var->print(equalities);
        
        std::cout << "\n";
    }
    std::cout << std::endl;

    for (auto constraint : new_running_valid_constraints) {
        constraint.print();
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

    for (auto constraint : running_valid_constraints) {
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
    // resolve_constraints();

    // while (alil->clear_to_next()) {
    //     auto out = command_handle(alil->next_command());
    //     out->print();
    //     std::cout << std::endl;

        
    // }
}

