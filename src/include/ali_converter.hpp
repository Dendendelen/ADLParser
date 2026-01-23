#ifndef ALI_CONVERTER_H
#define ALI_CONVERTER_H

#include "ast_visitor.hpp"
#include "lexer.hpp"
#include "tokens.hpp"
#include <memory>
#include <string>
#include <vector>


enum AnalysisLevelInstruction {
    CREATE_REGION,
    MERGE_REGIONS,
    CUT_REGION,

    ADD_ALIAS,
    ADD_EXTERNAL,
    ADD_CORRECTIONLIB,

    CREATE_MASK,
    LIMIT_MASK,
    APPLY_MASK,

    CREATE_HIST_LIST,
    ADD_HIST_TO_LIST,
    USE_HIST,
    USE_HIST_LIST,

    HIST_1D,
    HIST_2D,

    WEIGHT_APPLY,

    DO_CUTFLOW_ON_REGION,

    CREATE_BIN,

    CREATE_TABLE,
    CREATE_TABLE_VALUE,
    CREATE_TABLE_LOWER_BOUNDS,
    CREATE_TABLE_UPPER_BOUNDS,
    APPEND_TO_TABLE,
    FINISH_TABLE,

    BEGIN_EXPRESSION,
    END_EXPRESSION,

    BEGIN_IF,
    END_IF,

    SORT_ASCEND,
    SORT_DESCEND,

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

    FUNC_THETA,

    FUNC_ABS_ISO,
    FUNC_MINI_ISO,

    FUNC_DR,
    FUNC_DPHI,
    FUNC_DETA,

    FUNC_SIZE,

    FUNC_FMT2,
    FUNC_TAUTAU,
    FUNC_HT,
    FUNC_SPHERICITY,
    FUNC_APLANARITY,

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
    FUNC_MIN,
    FUNC_MAX,

    FUNC_MAX_LIST,
    FUNC_MIN_LIST,

    FUNC_ANYOCCURRENCES,
    FUNC_FIRST,
    FUNC_SECOND,
    FUNC_SORT_ASCEND,
    FUNC_SORT_DESCEND,

    FUNC_FLAVOR,
    FUNC_CONSTITUENTS,
    FUNC_PDG_ID,
    FUNC_JET_ID,
    FUNC_IDX,
    FUNC_TAUTAG,
    FUNC_CTAG,
    FUNC_DXY,
    FUNC_DZ,

    FUNC_IS_TIGHT,
    FUNC_IS_MEDIUM,
    FUNC_IS_LOOSE,

    FUNC_NAMED,

    MAKE_EMPTY_PARTICLE,

    MAKE_EMPTY_UNION,
    ADD_NAMED_TO_UNION,
    ADD_ELECTRON_TO_UNION,
    ADD_MUON_TO_UNION,
    ADD_TAU_TO_UNION,
    ADD_TRACK_TO_UNION,
    ADD_LEPTON_TO_UNION,
    ADD_PHOTON_TO_UNION,
    ADD_BJET_TO_UNION,
    ADD_QGJET_TO_UNION,
    ADD_NUMET_TO_UNION,
    ADD_METLV_TO_UNION,
    ADD_GEN_TO_UNION,
    ADD_JET_TO_UNION,
    ADD_FJET_TO_UNION,

    MAKE_EMPTY_COMB,
    ADD_NAMED_TO_COMB,
    ADD_ELECTRON_TO_COMB,
    ADD_MUON_TO_COMB,
    ADD_TAU_TO_COMB,
    ADD_TRACK_TO_COMB,
    ADD_LEPTON_TO_COMB,
    ADD_PHOTON_TO_COMB,
    ADD_BJET_TO_COMB,
    ADD_QGJET_TO_COMB,
    ADD_NUMET_TO_COMB,
    ADD_METLV_TO_COMB,
    ADD_GEN_TO_COMB,
    ADD_JET_TO_COMB,
    ADD_FJET_TO_COMB,

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
    SUB_PART_NAMED

};

class AnalysisCommand {
    private:
        AnalysisLevelInstruction instruction;

        std::string dest_argument;
        std::vector<std::string> source_arguments;
        bool has_dest_argument_yet;

        std::weak_ptr<Token> source_token;
        bool has_source_token;
    public:
        AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok);
        AnalysisCommand(AnalysisLevelInstruction inst);

        void add_dest_argument(std::string arg);
        void add_source_argument(std::string arg);

        AnalysisLevelInstruction get_instruction();
        std::string get_argument(int pos);
        int get_num_arguments();

        bool has_dest_argument();
        std::string get_dest_argument();
        std::string get_source_argument(int pos);
    
        void print_instruction();
        void print_instruction(int width_of_dest, int width_of_inst);
        std::string static instruction_to_text(AnalysisLevelInstruction inst);
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
        std::string comb_list(PNode node, std::string prev);

        std::string unary_operator(PNode node);
        std::string binary_operator(PNode node);
        std::string interval_operator(PNode node);
        std::string literal_value(PNode node);
        std::string keyword_value(PNode node);

        std::string reserve_scoped_value_name();
        std::string reserve_scoped_limit_name();
        std::string reserve_scoped_region_name();

        void visit_object_first_second(PNode node);
        void visit_union_type(PNode node); 
        void visit_comb_type(PNode node); 

        std::string last_condition_name;

        std::string last_value_name;

        std::string current_limit;

        std::string current_scope_name;

        Token_type current_object_token;
        std::string current_object_particle_if_named;

        std::string current_region;
        int highest_var_val;

        int iter_command;

    protected:
        void visit_object(PNode node) override;
        void visit_if(PNode node) override;
        void visit_sort(PNode node) override;
        void visit_object_select(PNode node) override;
        void visit_object_reject(PNode node) override;
        void visit_region_select(PNode node) override;
        void visit_region_reject(PNode node) override;
        void visit_condition(PNode node) override;
        void visit_region(PNode node) override;
        void visit_definition(PNode node) override;
        void visit_criteria(PNode node) override;
        void visit_use(PNode node) override;
        void visit_histogram(PNode node) override;
        void visit_histo_list(PNode node) override;
        void visit_histo_use(PNode node) override;
        void visit_particle_sum(PNode node) override;
        void visit_expression(PNode node) override;
        void visit_bin(PNode node) override;
        void visit_bin_list(PNode node) override;
        void visit_table_def(PNode node) override;
        void visit_weight(PNode node) override;
        void visit_cutflow_use(PNode node) override;


    public:
        ALIConverter();

        void visitation(PNode root);
        void print_commands();

        AnalysisCommand next_command();
        bool clear_to_next();
};


#endif