#ifndef ALI_CONVERTER_H
#define ALI_CONVERTER_H

#include "ast_visitor.hpp"
#include <string>
#include <vector>


enum AnalysisLevelInstruction {
    CREATE_REGION,
    MERGE_REGIONS,
    CUT_REGION,

    RUN_REGION,

    ADD_ALIAS,
    ADD_EXTERNAL,

    ADD_OBJECT,
    CREATE_MASK,
    LIMIT_MASK,
    APPLY_MASK,

    CREATE_HIST_LIST,
    ADD_HIST_TO_LIST,
    USE_HIST,
    USE_HIST_LIST,

    HIST_1D,
    HIST_2D,

    CREATE_BIN,

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

    FUNC_VER_TR,
    FUNC_VER_Z,
    FUNC_VER_Y,
    FUNC_VER_X,
    FUNC_VER_T,
    FUNC_GEN_PART_IDX,

    FUNC_CHARGE,

    FUNC_BTAG,
    FUNC_PT,
    FUNC_ETA,
    FUNC_RAPIDITY,
    FUNC_PHI,
    FUNC_MASS,
    FUNC_ENERGY,
    FUNC_MSOFTDROP,

    FUNC_ABS_ETA,
    FUNC_THETA,
    FUNC_PT_CONE,
    FUNC_ET_CONE,
    FUNC_ABS_ISO,
    FUNC_MINI_ISO,

    FUNC_PZ,
    FUNC_NBF,
    FUNC_DR,
    FUNC_DPHI,
    FUNC_DETA,
    FUNC_SIZE,

    FUNC_FMT2,
    FUNC_TAUTAU,
    FUNC_HT,
    FUNC_SPHERICITY,
    FUNC_APLANARITY,

    MAKE_EMPTY_PARTICLE,

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

    FUNC_FLAVOR,
    FUNC_CONSTITUENTS,
    FUNC_PDG_ID,
    FUNC_JET_ID,
    FUNC_IDX,
    FUNC_TAUTAG,
    FUNC_CTAG,
    FUNC_DXY,
    FUNC_EDXY,
    FUNC_EDZ,
    FUNC_DZ,

    FUNC_IS_TIGHT,
    FUNC_IS_MEDIUM,
    FUNC_IS_LOOSE,

    FUNC_NAMED,

};

class AnalysisCommand {
    private:
        AnalysisLevelInstruction instruction;
        std::vector<std::string> arguments;
    public:
        AnalysisCommand(AnalysisLevelInstruction inst);
        void add_argument(std::string arg);

        AnalysisLevelInstruction get_instruction();
        std::string get_argument(int pos);
        int get_num_arguments();
    
        void print_instruction();
};

class ALIConverter : ASTVisitor {
    private:
        std::vector<AnalysisCommand> command_list;
        std::vector<AnalysisCommand> post_list;

        std::string handle_expression(PNode node);

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

        std::string last_value_name;

        std::string current_limit;

        std::string current_scope_name;

        std::string current_region;
        int highest_var_val;

        int iter_command;

    protected:
        void visit_object(PNode node) override;
        void visit_if(PNode node) override;
        void visit_object_select(PNode node) override;
        void visit_object_reject(PNode node) override;
        void visit_region_select(PNode node) override;
        void visit_condition(PNode node) override;
        void visit_region(PNode node) override;
        void visit_definition(PNode node) override;
        void visit_criteria(PNode node) override;
        void visit_use(PNode node) override;
        void visit_histogram(PNode node) override;
        void visit_histo_list(PNode node) override;
        void visit_histo_use(PNode node) override;
        void visit_particle_list(PNode node) override;
        void visit_expression(PNode node) override;
        void visit_bin(PNode node) override;
        void visit_bin_list(PNode node) override;

    public:
        ALIConverter();

        void visitation(PNode root);
        void print_commands();

        AnalysisCommand next_command();
        bool clear_to_next();
};


#endif