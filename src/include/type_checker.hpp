#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H


#include "ali_converter.hpp"
#include <memory>
#include <unordered_map>
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
        std::shared_ptr<Type> lhs;
        std::shared_ptr<Type> rhs;
        std::string get_infix_string();
    public:
        Statement(StatementForm, std::shared_ptr<Type>, std::shared_ptr<Type>);
        
        void print();

};

class Constraint {
    private:
        std::vector<Statement> premises;
        std::vector<Statement> conclusions;
    public:
        void add_premise(Statement);
        void add_conclusion(Statement);
        void print();
};


class Type : public std::enable_shared_from_this<Type>{
    private:
        BaseType this_type;
        std::vector<std::shared_ptr<Type>> source_types;
        std::shared_ptr<Type> dest_type;
        std::vector<Constraint> constraints;
        bool has_dest_type;

        std::string get_name_string();

        static std::unordered_map<BaseType, std::shared_ptr<Type>> base_type_instances_map;
    public:
        static std::unordered_map<std::shared_ptr<Type>, int> generic_map;
        static int highest_mapped_generic;

        Type(BaseType);
        BaseType get_base_type();
        void add_source_type(std::shared_ptr<Type>);
        void add_source_type(BaseType);
        void add_dest_type(std::shared_ptr<Type>);
        void add_dest_type(BaseType);
        void add_constraint(Constraint);

        void print();
        
        static std::shared_ptr<Type> fundamental_type_instance(BaseType);

};


class Typer : public ALILToFrameworkCompiler {
    private:

        std::shared_ptr<Type> command_handle(AnalysisCommand);
    public:
        using ALILToFrameworkCompiler::ALILToFrameworkCompiler;
        void print() override;
};

#endif