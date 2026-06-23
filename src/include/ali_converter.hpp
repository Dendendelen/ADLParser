#ifndef ALI_CONVERTER_H
#define ALI_CONVERTER_H

#include "ast_visitor.hpp"
#include "config.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>


enum AnalysisLevelInstruction {

    // Types:
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

    CREATE_REGION,  // () -> Region
    MERGE_REGIONS,  // Region, Region -> Region
    CUT_REGION,     // Region, Cond -> Region

    ADD_ALIAS,      // `a -> `a
    ADD_EXTERNAL,   // String -> `a
    ADD_EXTERN_ATTR, // String -> (`b <<: ParticleInstance -> `a)
    ADD_CORRECTIONLIB, //TODO:

    CREATE_MASK, // Particle -> Mask
    LIMIT_MASK, // Mask, Cond -> Mask
    APPLY_MASK, // Mask, Particle -> Particle

    CREATE_HIST_LIST, 
    ADD_HIST_TO_LIST,
    USE_HIST,
    USE_HIST_LIST,

    HIST_1D, // Non-typed
    HIST_2D, // Non-typed

    WEIGHT_APPLY, // Region, Number -> Region

    DO_CUTFLOW_ON_REGION, // Non-typed
    DO_EVENTLIST_ON_REGION, // Non-typed

    CREATE_BIN,

    CREATE_TABLE,
    CREATE_TABLE_VALUE,
    CREATE_TABLE_LOWER_BOUNDS,
    CREATE_TABLE_UPPER_BOUNDS,
    APPEND_TO_TABLE,
    FINISH_TABLE,

    BEGIN_EXPRESSION, // () -> `a
    END_EXPRESSION, // `a -> `a

    BEGIN_IF, 
    END_IF,

    SORT_ASCEND,    // Particle, NumberList -> Particle
    SORT_DESCEND,   // Particle, NumberList -> Particle

    EXPR_RAISE, // `a <<: Number, Number -> `a
    EXPR_MULTIPLY, // `a <<: Number, `b <<: Number -> {if `a = `b then `a else if `a = Number then `b else if `b = Number then `a else Error}
    EXPR_DIVIDE,
    EXPR_ADD,
    EXPR_SUBTRACT,
    EXPR_LT, // 
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

    EXPR_INDEX, // NumberList, Number -> Number

    FUNC_GEN_PART_IDX,

    FUNC_CHARGE,

    FUNC_BTAG,
    FUNC_PT,    // `a <<: ParticleInstance -> {if `a < ParticleInstance then Number else NumberList}
    FUNC_ETA,   
    FUNC_RAPIDITY,
    FUNC_PHI,
    FUNC_MASS,
    FUNC_ENERGY,
    FUNC_MSOFTDROP,

    FUNC_THETA,

    FUNC_ABS_ISO,
    FUNC_MINI_ISO,

    FUNC_DISTINCT,  // `a Particlelike, `b ParticleLike -> {}

    FUNC_DR,    // `a Particlelike, `b Particlelike -> {If `a < ParticleInstance then `b else if `b < ParticleInstance then `a else if `a < Particle and `b < Particle then ParticleMatrix else Error}
    FUNC_DPHI,
    FUNC_DETA,

    FUNC_DR_HADAMARD,   // Particle, Particle -> NumberList
    FUNC_DPHI_HADAMARD,
    FUNC_DETA_HADAMARD,

    FUNC_SIZE,  // Particle -> Number

    FUNC_ANYOF, 
    FUNC_ALLOF, 

    FUNC_SQRT, // `a <<: Number -> `a
    FUNC_ABS, // `a Numeric -> `a Numeric
    FUNC_COS,  
    FUNC_SIN, 
    FUNC_TAN, 
    FUNC_SINH, 
    FUNC_COSH, 
    FUNC_TANH, 
    FUNC_EXP, 
    FUNC_LOG, 

    FUNC_AVE,  // List<`a <<: Number > -> `a
    FUNC_SUM, 
    FUNC_MIN,
    FUNC_MAX,

    FUNC_MAX_LIST,  // `a <<: Number, `a -> `a
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

    FUNC_TAUTAG,

    FUNC_CTAG,
    FUNC_DXY,
    FUNC_DZ,

    FUNC_IS_TIGHT,
    FUNC_IS_MEDIUM,
    FUNC_IS_LOOSE,

    FUNC_NAMED, // `a<`c, (`c -> `b) -> `b

    MAKE_EMPTY_PARTICLE,    // () -> Particle

    MAKE_EMPTY_UNION,   // () -> Union
    ADD_NAMED_TO_UNION, // Union, Particle -> Union
    ADD_ELECTRON_TO_UNION,  // Union -> Union
    ADD_MUON_TO_UNION,
    ADD_TAU_TO_UNION,
    ADD_TRACK_TO_UNION,
    ADD_PHOTON_TO_UNION,
    ADD_QGJET_TO_UNION,
    ADD_METLV_TO_UNION,
    ADD_GEN_TO_UNION,
    ADD_JET_TO_UNION,
    ADD_FJET_TO_UNION,

    MAKE_EMPTY_COMB,    // () -> Comb
    ADD_NAMED_TO_COMB,  // Comb, Particle -> Comb
    ADD_ELECTRON_TO_COMB, // Comb -> Comb
    ADD_MUON_TO_COMB,
    ADD_TAU_TO_COMB,
    ADD_TRACK_TO_COMB,
    ADD_PHOTON_TO_COMB,
    ADD_QGJET_TO_COMB,
    ADD_METLV_TO_COMB,
    ADD_GEN_TO_COMB,
    ADD_JET_TO_COMB,
    ADD_FJET_TO_COMB,

    NAME_ELEMENT_OF_COMB,   // Comb, Number -> Particle

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

    ADD_PART_ELECTRON_INDEXED,
    ADD_PART_MUON_INDEXED,
    ADD_PART_TAU_INDEXED,
    ADD_PART_TRACK_INDEXED,
    ADD_PART_PHOTON_INDEXED,
    ADD_PART_QGJET_INDEXED,
    ADD_PART_METLV_INDEXED,
    ADD_PART_GEN_INDEXED,
    ADD_PART_JET_INDEXED,
    ADD_PART_FJET_INDEXED,
    ADD_PART_NAMED_INDEXED,

    ADD_PART_ELECTRON_RANGE,
    ADD_PART_MUON_RANGE,
    ADD_PART_TAU_RANGE,
    ADD_PART_TRACK_RANGE,
    ADD_PART_PHOTON_RANGE,
    ADD_PART_QGJET_RANGE,
    ADD_PART_METLV_RANGE,
    ADD_PART_GEN_RANGE,
    ADD_PART_JET_RANGE,
    ADD_PART_FJET_RANGE,
    ADD_PART_NAMED_RANGE,

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
    SUB_PART_NAMED,

    SUB_PART_ELECTRON_INDEXED,
    SUB_PART_MUON_INDEXED,
    SUB_PART_TAU_INDEXED,
    SUB_PART_TRACK_INDEXED,
    SUB_PART_PHOTON_INDEXED,
    SUB_PART_QGJET_INDEXED,
    SUB_PART_METLV_INDEXED,
    SUB_PART_GEN_INDEXED,
    SUB_PART_JET_INDEXED,
    SUB_PART_FJET_INDEXED,
    SUB_PART_NAMED_INDEXED,

    SUB_PART_ELECTRON_RANGE,
    SUB_PART_MUON_RANGE,
    SUB_PART_TAU_RANGE,
    SUB_PART_TRACK_RANGE,
    SUB_PART_PHOTON_RANGE,
    SUB_PART_QGJET_RANGE,
    SUB_PART_METLV_RANGE,
    SUB_PART_GEN_RANGE,
    SUB_PART_JET_RANGE,
    SUB_PART_FJET_RANGE,
    SUB_PART_NAMED_RANGE,

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

class ALILEmitter {
    protected:
        std::vector<AnalysisCommand> command_list;  
        int iter_command;
    public:
        ALILEmitter();
        virtual ~ALILEmitter() = default;

        void print_commands();
        AnalysisCommand next_command();
        bool clear_to_next();
};

class ALILConverter : ASTVisitor, public ALILEmitter {
    private:

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

        void visit_object_first_second(PNode node);
        void visit_sort(PNode node);
        void visit_union_type(PNode node); 
        void visit_comb_type(PNode node); 
        void visit_direct_combiner(PNode node);

        std::string last_condition_name;
        std::string last_value_name;
        std::string current_limit;
        std::string current_scope_name;

        Token_type current_object_token;
        std::string current_object_particle_if_named;

        std::string current_region;

        std::vector<std::string> current_defined_variables_within_comb;

        int highest_var_val;

        Config &config;

    protected:
        void visit_object(PNode node) override;
        void visit_if(PNode node) override;
        void visit_object_select(PNode node) override;
        void visit_object_reject(PNode node) override;
        void visit_region_select(PNode node) override;
        void visit_region_reject(PNode node) override;
        void visit_composite(PNode node) override;
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


    public:
        ALILConverter(Config &conf);
        void clean_command_list();
        void visitation(PNode root);
};


class ALILToFrameworkCompiler {
    protected:
        std::unique_ptr<ALILEmitter> alil;
        Config &config;

    public:
        ALILToFrameworkCompiler(ALILEmitter *alil_in, Config &conf);
        virtual ~ALILToFrameworkCompiler() = default;
        virtual void print() = 0;
};


class ALILToALILCompiler : public ALILEmitter {
    protected:
        std::unique_ptr<ALILEmitter> alil;
        Config &config;

    public:
        ALILToALILCompiler(ALILEmitter *alil_in, Config &conf);
        virtual ~ALILToALILCompiler() = default;
};


class RedundancyEliminator : public ALILToALILCompiler {
    private:
        std::unordered_set<std::string> needed;
    public:
        using ALILToALILCompiler::ALILToALILCompiler;
        void eliminate();
};


#endif