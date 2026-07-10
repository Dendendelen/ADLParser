#ifndef ALI_CONVERTER_H
#define ALI_CONVERTER_H

#include "ast_visitor.hpp"
#include "config.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>


enum class AnalysisLevelInstruction {

    CREATE_EMPTY_INFO_LIST,
    ADD_TO_INFO_LIST,
    DISPLAY_INFO,



    CREATE_REGION,
    MERGE_REGIONS,
    CUT_REGION,

    CREATE_BIN_OF_REGION,

    ADD_ALIAS,
    ADD_EXTERNAL,
    ADD_EXTERN_ATTR,
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
    DO_EVENTLIST_ON_REGION,

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

    OBJ_SORT_ASCEND,
    OBJ_SORT_DESCEND,

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
    EXPR_WITHIN_EXCLUSIVE,
    EXPR_WITHIN_LEFT_EXCLUSIVE,
    EXPR_WITHIN_RIGHT_EXCLUSIVE,

    EXPR_OUTSIDE,

    EXPR_NEGATE,
    EXPR_LOGICAL_NOT,

    EXPR_INDEX,

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

    FUNC_DISTINCT,

    FUNC_DR,
    FUNC_DPHI,
    FUNC_DETA,

    FUNC_DR_HADAMARD,
    FUNC_DPHI_HADAMARD,
    FUNC_DETA_HADAMARD,

    FUNC_SIZE,

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

    FUNC_SORT_ASCEND,
    FUNC_SORT_DESCEND,

    FUNC_NAMED,

    MAKE_EMPTY_PARTICLE,

    MAKE_EMPTY_UNION,
    ADD_PART_TO_UNION,
    
    MAKE_EMPTY_COMB,
    ADD_NAMED_TO_COMB,
    ADD_ELECTRON_TO_COMB,
    ADD_MUON_TO_COMB,
    ADD_TAU_TO_COMB,
    ADD_TRACK_TO_COMB,
    ADD_PHOTON_TO_COMB,
    ADD_QGJET_TO_COMB,
    ADD_METLV_TO_COMB,
    ADD_GEN_TO_COMB,
    ADD_JET_TO_COMB,
    ADD_FJET_TO_COMB,

    NAME_ELEMENT_OF_COMB,

    MAKE_EMPTY_DISJOINT,
    ADD_NAMED_TO_DISJOINT,
    ADD_ELECTRON_TO_DISJOINT,
    ADD_MUON_TO_DISJOINT,
    ADD_TAU_TO_DISJOINT,
    ADD_TRACK_TO_DISJOINT,
    ADD_PHOTON_TO_DISJOINT,
    ADD_QGJET_TO_DISJOINT,
    ADD_METLV_TO_DISJOINT,
    ADD_GEN_TO_DISJOINT,
    ADD_JET_TO_DISJOINT,
    ADD_FJET_TO_DISJOINT,

    NAME_ELEMENT_OF_DISJOINT,

    ADD_PART_ELECTRON,
    ADD_PART_MUON,
    ADD_PART_TAU,
    ADD_PART_TRACK,
    ADD_PART_PHOTON,
    ADD_PART_QGJET,
    ADD_PART_METLV,
    ADD_PART_GEN,
    ADD_PART_JET,
    ADD_PART_FJET,
    ADD_PART_NAMED,

    SUB_PART_ELECTRON,
    SUB_PART_MUON,
    SUB_PART_TAU,
    SUB_PART_TRACK,
    SUB_PART_PHOTON,
    SUB_PART_QGJET,
    SUB_PART_METLV,
    SUB_PART_GEN,
    SUB_PART_JET,
    SUB_PART_FJET,
    SUB_PART_NAMED

};

typedef AnalysisLevelInstruction ALIL;



#define MAKE_UNCONSUMED __attribute__((return_typestate(unconsumed)))
static_assert(true, "Macro check");

#define CALLABLE_UNCONSUMED __attribute__((callable_when(unconsumed)))
#define CALLABLE_CONSUMED __attribute__((callable_when(consumed)))
#define CALLABLE_EITHER __attribute__((callable_when(unconsumed, consumed)))

#define PARAM_UNCONSUMED __attribute__((param_typestate(unconsumed))) 

class ALILCollection;
class ALILConverter;

class __attribute__((consumable(unconsumed))) AnalysisCommand {
    private:
        AnalysisLevelInstruction instruction;

        bool has_been_collected;
        void mark_collected() CALLABLE_UNCONSUMED __attribute__((set_typestate(consumed)));

        std::optional<std::string> dest_argument;
        std::vector<std::string> source_arguments;

        std::optional<std::weak_ptr<Token>> source_token;
    public:
        AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok) MAKE_UNCONSUMED;
        AnalysisCommand(AnalysisLevelInstruction inst) MAKE_UNCONSUMED;
        AnalysisCommand(const AnalysisCommand& other) MAKE_UNCONSUMED;
        AnalysisCommand(AnalysisCommand&& other) MAKE_UNCONSUMED;

        ~AnalysisCommand() CALLABLE_CONSUMED;

        void add_dest_argument(std::string arg) CALLABLE_UNCONSUMED;
        void add_source_argument(std::string arg) CALLABLE_UNCONSUMED;

        std::string reserve_dest_arg_value(ALILConverter *) CALLABLE_UNCONSUMED;

        AnalysisLevelInstruction get_instruction() CALLABLE_CONSUMED;
        std::string get_argument(int pos) CALLABLE_CONSUMED;
        int get_num_arguments() CALLABLE_CONSUMED;

        bool has_dest_argument() CALLABLE_CONSUMED;
        std::string get_dest_argument() CALLABLE_CONSUMED;
        std::string get_source_argument(int pos) CALLABLE_CONSUMED;
    
        void print_instruction() CALLABLE_CONSUMED;
        void print_instruction(int width_of_dest, int width_of_inst) CALLABLE_CONSUMED;
        std::string static instruction_to_text(AnalysisLevelInstruction inst);

        void collect_into(ALILCollection &) CALLABLE_UNCONSUMED;

        friend ALILCollection;
};

class ALILCollection {
    private:
        std::vector<AnalysisCommand> command_list;
        void collect_command(AnalysisCommand in);

    public:
        ALILCollection();
        std::vector<AnalysisCommand> emit_collected_commands;

        friend AnalysisCommand;
};

class ALILConverter : ASTVisitor {
    private:

        ALILCollection commands;

        void clean_command_list();

        std::string handle_expression(PNode node);

        std::string if_operator(PNode node);

        std::string empty_particle_create();

        std::string handle_particle_list(PNode node);
        std::string handle_particle(PNode node, std::string last_part);

        std::string function_handler(PNode node);
        std::string particle_list_function(PNode node);

        std::string expression_function(PNode node);
        std::string union_list(PNode node, std::string prev);
        std::string comb_list(PNode node, std::string prev, bool is_comb);

        std::string unary_operator(PNode node);
        std::string binary_operator(PNode node);
        std::string comparison_operator(PNode node);
        std::string interval_operator(PNode node);
        std::string literal_value(PNode node);
        std::string keyword_value(PNode node);

        std::string reserve_scoped_value_name();
        std::string reserve_scoped_limit_name();
        std::string reserve_scoped_region_name();

        class NameScope {
            private:
                std::string old_name;
                ALILConverter *this_converter;
            public:
                NameScope(std::string, ALILConverter *);
                NameScope(std::string type_name, PNode id_node, ALILConverter *);
                ~NameScope();
        };        
        std::string current_scope_name;
        std::string what_object_is_this;
        std::string what_region_are_we_in;

        void visit_object_first_second(PNode node);
        void visit_sort(PNode node);
        void visit_union_type(PNode node); 
        void visit_comb_type(PNode node); 
        void visit_direct_combiner(PNode node);

        std::string last_condition_name;
        std::string last_value_name;
        std::string current_limit;

        Token_type current_object_token;
        std::string current_object_particle_if_named;

        std::string current_region;

        std::vector<std::string> current_defined_variables_within_comb;

        int highest_var_val;

        int iter_command;

        Config &config;

    protected:
        void visit_info(PNode node) override;
        void visit_definition(PNode node) override;
        void visit_composite(PNode node) override;
        void visit_object(PNode node) override;

        void visit_initializations(PNode node) override;

        void visit_object_criteria(PNode node) override;
        void visit_obj_union(PNode node) override;
        void visit_obj_sort(PNode node) override;
        void visit_object_select(PNode node) override;
        void visit_object_reject(PNode node) override;
    
        void visit_region_commands(PNode node) override;
        void visit_region_select(PNode node) override;
        void visit_region_reject(PNode node) override;
        


    public:
        ALILConverter(Config &conf);

        void visitation(PNode root);
        void print_commands();

        AnalysisCommand next_command();
        bool clear_to_next();

        friend AnalysisCommand;
};


class ALILToFrameworkCompiler {
    protected:
        std::unique_ptr<ALILConverter> alil;
        Config &config;

    public:
        ALILToFrameworkCompiler(ALILConverter *alil_in, Config &conf);
        virtual ~ALILToFrameworkCompiler() = default;
        virtual void print() = 0;
};

#endif