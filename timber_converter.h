#ifndef TIMBER_CONVERTER_H
#define TIMBER_CONVERTER_H

#include "ast_visitor.h"
#include <string>
#include <vector>


enum AnalysisLevelInstruction {
    BEGIN_REGION_BLOCK,
    END_REGION_BLOCK,

    CREATE_REGION,
    MERGE_REGIONS,
    CUT_REGION,

    RUN_REGION,

    ADD_ALIAS,

    ADD_OBJECT,
    CREATE_MASK,
    LIMIT_MASK,
    APPLY_MASK,

    BEGIN_EXPRESSION,
    END_EXPRESSION,
    BEGIN_IF,
    END_IF,

    EXPR_RAISE,
    EXPR_MULTIPLY,
    EXPR_DIVIDE,
    EXPR_ADD,
    EXPR_SUBTRACT,
    EXPR_LT,
    EXPR_LE,
    EXPR_GT,
    EXPR_GE,
    EXPR_EQ,
    EXPR_NE,
    EXPR_AMPERSAND,
    EXPR_PIPE,
    EXPR_AND,
    EXPR_OR,

    EXPR_WITHIN,
    EXPR_OUTSIDE,

    EXPR_NEGATE,
    EXPR_LOGICAL_NOT,

    FUNC_BTAG,
    FUNC_PT,
    FUNC_ETA,
    FUNC_PHI,
    FUNC_M,
    FUNC_E,

    MAKE_EMPTY_PARTICLE,
    CREATE_PARTICLE_VARIABLE,

    MAKE_EMPTY_UNION,
    ADD_NAMED_TO_UNION,
    ADD_ELECTRON_TO_UNION,
    ADD_MUON_TO_UNION,
    ADD_TAU_TO_UNION,

    ADD_PART_ELECTRON,
    ADD_PART_MUON,
    ADD_PART_TAU,
    ADD_PART_TRACK,
    ADD_PART_LEPTON,
    ADD_PART_PHOTON,
    ADD_PART_BJET,
    ADD_PART_QGJET,
    ADD_PART_NUMET, 
    ADD_PART_METLV,
    ADD_PART_GEN,
    ADD_PART_JET,
    ADD_PART_FJET,
    ADD_PART_NAMED,

    SUB_PART_ELECTRON,
    SUB_PART_MUON,
    SUB_PART_TAU,
    SUB_PART_TRACK,
    SUB_PART_LEPTON,
    SUB_PART_PHOTON,
    SUB_PART_BJET,
    SUB_PART_QGJET,
    SUB_PART_NUMET, 
    SUB_PART_METLV,
    SUB_PART_GEN,
    SUB_PART_JET,
    SUB_PART_FJET,
    SUB_PART_NAMED,

    FUNC_HSTEP,
    FUNC_DELTA, 
    FUNC_ANYOF, 
    FUNC_ALLOF, 
    FUNC_SQRT, 
    FUNC_ABS, 
    FUNC_COS,  
    FUNC_SIN, 
    FUNC_TAN, 
    FUNC_SINH, 
    FUNC_COSH, 
    FUNC_TANH, 
    FUNC_EXP, 
    FUNC_LOG, 
    FUNC_AVE, 
    FUNC_SUM, 

    FUNC_NAMED,

};

class AnalysisCommand {
    private:
        AnalysisLevelInstruction instruction;
        std::vector<std::string> arguments;
    public:
        AnalysisCommand(AnalysisLevelInstruction inst);
        void add_argument(std::string arg);
        void convert_instruction();
        void print_instruction();
};

class ALIConverter : ASTVisitor {
    private:
        std::vector<AnalysisCommand> command_list;
        std::vector<AnalysisCommand> post_list;

        std::string visit_expression(PNode node);

        std::string if_operator(PNode node);

        std::string handle_particle_list(PNode node);
        std::string handle_particle(PNode node, std::string last_part);
        std::string particle_list_function(PNode node);

        std::string expression_function(PNode node);

        std::string union_list(PNode node, std::string prev);


        std::string unary_operator(PNode node);
        std::string binary_operator(PNode node);
        std::string interval_operator(PNode node);
        std::string literal_value(PNode node);
        std::string keyword_value(PNode node);

        std::string reserve_scoped_value_name();
        std::string reserve_scoped_limit_name();
        std::string reserve_scoped_region_name();


        void visit_union_type(PNode node); 

        std::string last_condition_name;

        std::string current_scope_name;
        int highest_var_val;

    protected:
        void visit_object(PNode node) override;
        void visit_if(PNode node) override;
        void visit_object_select(PNode node) override;
        void visit_object_reject(PNode node) override;
        void visit_region_select(PNode node) override;
        void visit_condition(PNode node) override;
        void visit_region(PNode node) override;
        void visit_criteria(PNode node) override;
        void visit_use(PNode node) override;

    public:
        ALIConverter();

        void vistitation(PNode root);
        void print_commands();
};


#endif