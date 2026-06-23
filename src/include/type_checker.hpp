#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H


#include "ali_converter.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>



enum BaseType {
    TYPE_REGION,
    TYPE_COND,
    TYPE_MASK,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_NUMBER,
    TYPE_PARTICLEINSTANCE,
    TYPE_UNION,
    TYPE_COMB,
    TYPE_DISJOINT,
    TYPE_HIST,
    TYPE_ERROR,
    TYPE_FUNCTION,
    TYPE_GENERIC,
    // Region,
    // Cond,
    // Mask
    // String
    // List<>
    // Number < Numeric, 
    // NumberList = List<Number> < Numeric,
    // NumberMatrix = List<List<Number>> < Numeric
    // ParticleInstance < Particlelike
    // Particle = List<ParticleInstance> < Particlelike
    // Error
};

class Statement;
class Constraint;
class Type;

typedef std::shared_ptr<Type> PType;


enum StatementForm {
    STATEMENT_EQUALITY, // a = b
    STATEMENT_INEQUALITY, // a =/= b
    STATEMENT_SUBTYPE, // a <: b
    STATEMENT_NONSUBTYPE, // a ~<: b
    STATEMENT_SUPERTYPE, // a :> b
    STATEMENT_NONSUPERTYPE, // a ~:> b
    STATEMENT_HEREDITARY_SUBTYPE, // a <<: b
    STATEMENT_HEREDITARY_SUPERTYPE, // a :>> b
    STATEMENT_EQUAL_DEPTH, // a ~=~ b
};


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

};

class Constraint {
    private:
        std::vector<Statement> premises;
        std::vector<Statement> conclusions;
    public:
        void add_premise(Statement);
        void add_conclusion(Statement);

        std::vector<Statement> &get_premises();
        std::vector<Statement> &get_conclusions();

        void print();
};


class Type : public std::enable_shared_from_this<Type>{

    
    private:
        BaseType this_type;
        std::vector<PType> source_types;
        PType dest_type;
        std::vector<Constraint> constraints;
        bool has_dest_type;

        std::string get_name_string();

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
        PType get_source_type(int);
        int get_num_of_sources();

        void print();
        void print(std::unordered_map<PType, PType> &);
        
        static PType fundamental_type_instance(BaseType);

};

class Typer : public ALILToFrameworkCompiler {
    private:

        PType command_handle(AnalysisCommand);
        void equality_of_types(std::unordered_map<PType, PType> &, PType, PType);


        std::vector<Constraint> running_valid_constraints;
        std::unordered_map<std::string, PType> types_of_variables;
        std::vector<std::string> order_of_variables;

        std::unordered_set<std::string> used_variables;

    public:
        using ALILToFrameworkCompiler::ALILToFrameworkCompiler;

    
        void collect_existing_constraints();
        void resolve_constraints();

        void print() override;
};

#endif