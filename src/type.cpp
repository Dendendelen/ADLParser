#include "type.hpp"
#include <cassert>
#include <iostream>


// Maps for allowing us to easily compare and display types
std::unordered_map<BaseType, PType> Type::base_type_instances_map;
std::unordered_map<PType, int> Type::generic_map;
int Type::highest_mapped_generic;

// Constraint definitions:

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

void Constraint::print() {
    internal_print(false, nullptr);
}

void Constraint::print(std::unordered_map<PType, PType> &substitutions) {
    internal_print(true, &substitutions);
}


// Statement definitions:

Statement::Statement(StatementForm form_in, PType type1, PType type2) : form(form_in), lhs(type1), rhs(type2) {}


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

    case STATEMENT::EQUALITY:
        return " = ";
    case STATEMENT::INEQUALITY:
        return " =/= ";
    case STATEMENT::SUBTYPE:
        return " <: ";
    case STATEMENT::NONSUBTYPE:
        return " </:";
    case STATEMENT::SUPERTYPE:
        return " :> ";
    case STATEMENT::NONSUPERTYPE:
        return ":/>";
    case STATEMENT::HEREDITARY_SUBTYPE:
        return " <<: ";
    case STATEMENT::HEREDITARY_SUPERTYPE:
        return " :>> ";
    case STATEMENT::EQUAL_DEPTH:
        return " ~= ";
    case StatementForm::NOT_EQUAL_DEPTH:
        return "~/=";
      break;
    case StatementForm::IS_LIST_STRUCTURED:
        return " is L-structured ";
    case StatementForm::IS_LIST_FREE:
        return " is L-free ";
    case StatementForm::IS_NOT_LIST_STRUCTURED:
        return  " NOT is L-structured";
    case StatementForm::IS_NOT_LIST_FREE:
        return  " NOT is L-free";
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

// Type definitions:

Type::Type(BaseType in) : this_type(in), has_dest_type(false) {}

BaseType Type::get_base_type() {
    return this_type;
}

bool Type::is_fundamental_type() {
    return (this_type != TYPE::FUNCTION && this_type != TYPE::GENERIC && this_type != TYPE::LIST);
}

PType Type::fundamental_type_instance(BaseType bt) {
    // no non-fundamental-types should have this be called on it - lists, functions always have children, and generic is not a single type
    assert(bt != TYPE::GENERIC);
    assert(bt != TYPE::FUNCTION);
    assert(bt != TYPE::LIST);

    if (base_type_instances_map.count(bt) == 0) {
        base_type_instances_map.emplace(bt, std::make_shared<Type>(bt));
    }
    return base_type_instances_map[bt];
}

void Type::add_source_type(PType type) {
    assert(this_type == TYPE::FUNCTION);
    source_types.push_back(type);
}

void Type::add_source_type(BaseType type) {
    if (type != TYPE::GENERIC) {
        add_source_type(fundamental_type_instance(type));
    } else {
        // all generics are unique
        add_source_type(std::make_shared<Type>(TYPE::GENERIC));
    }


}

void Type::add_dest_type(PType type) {
    assert(this_type == TYPE::FUNCTION || this_type == TYPE::LIST);
    assert(!has_dest_type);

    dest_type = type;
    has_dest_type = true;
}

void Type::add_dest_type(BaseType type) {
    if (type != TYPE::GENERIC) {
        add_dest_type(fundamental_type_instance(type));
    } else {
        // all generics are unique
        add_dest_type(std::make_shared<Type>(TYPE::GENERIC));
    }
}

void Type::add_constraint(Constraint con) {
    constraints.push_back(con);
}

std::vector<Constraint> &Type::get_constraints() {
    return constraints;
}

int Type::get_num_of_sources() {
    assert(this_type == TYPE::FUNCTION);
    return source_types.size();
}

PType Type::get_source_type(size_t index) {
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
    if (this_type == TYPE::LIST) {
        dest_type->set_origin_token(tok_in);
    } else if (this_type == TYPE::FUNCTION) {
        if (has_dest_type) dest_type->set_origin_token(tok_in);
        for (auto src : source_types) {
            src->set_origin_token(tok_in);
        }
    }
}

std::string Type::get_name_string() {

    switch (this_type) {
        case TYPE::REGION:
            return "region";
        case TYPE::COND:
            return "cond";
        case TYPE::MASK:
            return "mask";
        case TYPE::STRING:
            return "string";
        case TYPE::LIST:
            return "list";
        case TYPE::NUMBER:
            return "number";
        case TYPE::PARTICLE:
            return "particle";
        case TYPE::COMB:
            return "comb";
        case TYPE::DISJOINT:
            return "disjoint";
        case TYPE::HIST:
            return "hist";
        case TYPE::TABLE:
            return "table";
        case TYPE::ERRORED_NUMBER:
            return "errored_number";
        case TYPE::ERROR:
            return "error";
        case TYPE::FUNCTION:
            return "function";
        case TYPE::GENERIC:
            return "generic";
    }
    
}


void Type::print(std::unordered_map<PType, PType> &substitutions) {


    if (substitutions.count(shared_from_this()) != 0 && substitutions[shared_from_this()] != shared_from_this()) {
        substitutions[shared_from_this()]->print(substitutions);
        return;
    }

    if (this_type != TYPE::GENERIC) {
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
    if (this_type == TYPE::FUNCTION) {
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
    } else if (this_type == TYPE::LIST) {
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

