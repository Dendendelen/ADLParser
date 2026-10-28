#ifndef TYPE_H
#define TYPE_H

#include "lexer.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

enum class BaseType {
    REGION,
    COND,
    MASK,
    STRING,
    NUMBER,
    PARTICLE,
    COMB,
    DISJOINT,
    HIST,
    TABLE,
    ERRORED_NUMBER,

    // Non-fundamental:
    ERROR,
    LIST,
    FUNCTION,
    GENERIC,
};
typedef BaseType TYPE;

enum class StatementForm {
    EQUALITY, // a = b
    INEQUALITY, // a =/= b
    SUBTYPE, // a <: b
    NONSUBTYPE, // a ~<: b
    SUPERTYPE, // a :> b
    NONSUPERTYPE, // a ~:> b
    HEREDITARY_SUBTYPE, // a <<: b
    HEREDITARY_SUPERTYPE, // a :>> b
    EQUAL_DEPTH, // a ~= b
    NOT_EQUAL_DEPTH,
    IS_LIST_STRUCTURED, // a is L-structured
    IS_LIST_FREE, // a is L-free
    IS_NOT_LIST_STRUCTURED, // a is not L-structured
    IS_NOT_LIST_FREE, // a is not L-free
};
typedef StatementForm STATEMENT;


class Type;
typedef std::shared_ptr<Type> PType;


class Statement {
    private:
        StatementForm form;
        PType lhs;
        PType rhs;
        std::string get_infix_string();
    public:
        Statement(StatementForm, PType, PType);
        
        PType get_lhs();
        PType get_rhs();
        StatementForm get_form();

        void print();
        void print(std::unordered_map<PType, PType> &);
};

class Constraint {
    private:
        std::vector<Statement> premises;
        std::vector<Statement> conclusions;
        std::weak_ptr<Type> originator_type;
        void internal_print(bool,std::unordered_map<PType, PType> *);
    public:
        Constraint(std::weak_ptr<Type>);
        void add_premise(Statement);
        void add_conclusion(Statement);

        std::vector<Statement> &get_premises();
        std::vector<Statement> &get_conclusions();
        std::weak_ptr<Type> get_originator_type();

        void print();
        void print(std::unordered_map<PType, PType> &);
};

class Type : public std::enable_shared_from_this<Type>{
    private:
        BaseType this_type;
        std::vector<PType> source_types;
        PType dest_type;
        std::vector<Constraint> constraints;
        bool has_dest_type;

        std::string get_name_string();

        PToken originator_token;

        static std::unordered_map<BaseType, PType> base_type_instances_map;
    public:
        static std::unordered_map<PType, int> generic_map;
        static int highest_mapped_generic;

        Type(BaseType);
        BaseType get_base_type();

        bool is_fundamental_type();

        void add_source_type(PType);
        void add_source_type(BaseType);
        void add_dest_type(PType);
        void add_dest_type(BaseType);
        void add_constraint(Constraint);

        std::vector<Constraint> &get_constraints();
        PType get_dest_type();
        PType get_source_type(size_t);
        int get_num_of_sources();

        void set_origin_token(PToken);
        PToken get_origin_token();

        void print();
        void print(std::unordered_map<PType, PType> &);
        
        static PType fundamental_type_instance(BaseType);

};

#endif