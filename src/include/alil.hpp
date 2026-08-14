#ifndef ALIL_H
#define ALIL_H

#include "lexer.hpp"

#include <ranges>
#include <span>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>

class ALILConverter;

#define ALIL_INSTRUCTION_LIST(X)                                               \
    X(CONVERSION_ERROR,                 conversion_error)                      \
                                                                               \
    X(CREATE_EMPTY_INFO_LIST,           create_empty_info_list)                \
    X(ADD_TO_INFO_LIST,                 add_to_info_list)                      \
    X(DISPLAY_INFO,                     display_info)                          \
                                                                               \
    X(CREATE_REGION,                    create_region)                         \
    X(MERGE_REGIONS,                    merge_regions)                         \
    X(CUT_REGION,                       cut_region)                            \
                                                                               \
    X(CREATE_BIN_OF_REGION,             create_bin_of_region)                  \
                                                                               \
    X(ADD_ALIAS,                        add_alias)                             \
    X(ADD_EXTERNAL,                     add_external)                          \
    X(ADD_EXTERN_ATTR,                  add_extern_attr)                       \
    X(ADD_EXTERN_PARTICLE,              add_extern_particle)                   \
    X(ADD_CORRECTIONLIB,                add_correctionlib)                     \
                                                                               \
    X(CREATE_MASK,                      create_mask)                           \
    X(LIMIT_MASK,                       limit_mask)                            \
    X(APPLY_MASK,                       apply_mask)                            \
                                                                               \
    X(CREATE_EMPTY_HIST_LIST,           create_empty_hist_list)                \
    X(ADD_HIST_TO_LIST,                 add_hist_to_list)                      \
    X(USE_HIST,                         use_hist)                              \
    X(USE_HIST_LIST,                    use_hist_list)                         \
                                                                               \
    X(HIST_1D,                          hist_1d)                               \
    X(HIST_2D,                          hist_2d)                               \
                                                                               \
    X(WEIGHT_APPLY,                     weight_apply)                          \
                                                                               \
    X(DO_CUTFLOW_ON_REGION,             do_cutflow_on_region)                  \
    X(DO_EVENTLIST_ON_REGION,           do_eventlist_on_region)                \
                                                                               \
    X(CREATE_TABLE,                     create_table)                          \
    X(CREATE_TABLE_ERRORED_VALUE,       create_table_errored_value)            \
    X(CREATE_TABLE_VALUE,               create_table_value)                    \
    X(APPEND_TO_TABLE,                  append_to_table)                       \
    X(FINISH_TABLE,                     finish_table)                          \
                                                                               \
    X(OBJ_SORT_ASCEND,                  obj_sort_ascend)                       \
    X(OBJ_SORT_DESCEND,                 obj_sort_descend)                      \
                                                                               \
    X(EXPR_RAISE,                       expr_raise)                            \
    X(EXPR_MULTIPLY,                    expr_multiply)                         \
    X(EXPR_DIVIDE,                      expr_divide)                           \
    X(EXPR_ADD,                         expr_add)                              \
    X(EXPR_SUBTRACT,                    expr_subtract)                         \
    X(EXPR_LT,                          expr_lt)                               \
    X(EXPR_LE,                          expr_le)                               \
    X(EXPR_GT,                          expr_gt)                               \
    X(EXPR_GE,                          expr_ge)                               \
    X(EXPR_EQ,                          expr_eq)                               \
    X(EXPR_NE,                          expr_ne)                               \
    X(EXPR_BITWISE_AND,                 expr_bitwise_and)                      \
    X(EXPR_BITWISE_OR,                  expr_bitwise_or)                       \
    X(EXPR_AND,                         expr_and)                              \
    X(EXPR_OR,                          expr_or)                               \
                                                                               \
    X(EXPR_WITHIN,                      expr_within)                           \
    X(EXPR_WITHIN_EXCLUSIVE,            expr_within_exclusive)                 \
    X(EXPR_WITHIN_LEFT_EXCLUSIVE,       expr_within_left_exclusive)            \
    X(EXPR_WITHIN_RIGHT_EXCLUSIVE,      expr_within_right_exclusive)           \
                                                                               \
    X(EXPR_NEGATE,                      expr_negate)                           \
    X(EXPR_LOGICAL_NOT,                 expr_logical_not)                      \
                                                                               \
    X(EXPR_IF_TERNARY,                  expr_if_ternary)                       \
                                                                               \
    X(EXPR_INDEX,                       expr_index)                            \
    X(EXPR_INDEX_RANGE,                 expr_index_range)                      \
    X(EXPR_INDEX_UNTIL,                 expr_index_until)                      \
    X(EXPR_INDEX_FROM,                  expr_index_from)                       \
                                                                               \
    X(FUNC_CHARGE,                      func_charge)                           \
    X(FUNC_PT,                          func_pt)                               \
    X(FUNC_ETA,                         func_eta)                              \
    X(FUNC_PHI,                         func_phi)                              \
    X(FUNC_MASS,                        func_mass)                             \
    X(FUNC_ENERGY,                      func_energy)                           \
                                                                               \
    X(FUNC_DISTINCT,                    func_distinct)                         \
                                                                               \
    X(FUNC_DR,                          func_dr)                               \
    X(FUNC_DPHI,                        func_dphi)                             \
    X(FUNC_DETA,                        func_deta)                             \
                                                                               \
    X(FUNC_DR_HADAMARD,                 func_dr_hadamard)                      \
    X(FUNC_DPHI_HADAMARD,               func_dphi_hadamard)                    \
    X(FUNC_DETA_HADAMARD,               func_deta_hadamard)                    \
                                                                               \
    X(FUNC_SIZE,                        func_size)                             \
                                                                               \
    X(FUNC_ANYOF,                       func_anyof)                            \
    X(FUNC_ALLOF,                       func_allof)                            \
                                                                               \
    X(FUNC_SQRT,                        func_sqrt)                             \
    X(FUNC_ABS,                         func_abs)                              \
    X(FUNC_COS,                         func_cos)                              \
    X(FUNC_SIN,                         func_sin)                              \
    X(FUNC_TAN,                         func_tan)                              \
    X(FUNC_SINH,                        func_sinh)                             \
    X(FUNC_COSH,                        func_cosh)                             \
    X(FUNC_TANH,                        func_tanh)                             \
    X(FUNC_EXP,                         func_exp)                              \
    X(FUNC_LOG,                         func_log)                              \
    X(FUNC_AVE,                         func_ave)                              \
    X(FUNC_SUM,                         func_sum)                              \
                                                                               \
    X(FUNC_MIN_OF_PAIR,                 func_min_of_pair)                      \
    X(FUNC_MAX_OF_PAIR,                 func_max_of_pair)                      \
                                                                               \
    X(FUNC_MIN_OF_LIST,                 func_min_of_list)                      \
    X(FUNC_MAX_OF_LIST,                 func_max_of_list)                      \
                                                                               \
    X(FUNC_SORT_ASCEND,                 func_sort_ascend)                      \
    X(FUNC_SORT_DESCEND,                func_sort_descend)                     \
                                                                               \
    X(FUNC_NAMED,                       func_named)                            \
                                                                               \
    X(CREATE_EMPTY_VALUE_LIST,          create_empty_value_list)               \
    X(ADD_VALUE_TO_LIST,                add_value_to_list)                     \
                                                                               \
    X(CREATE_EMPTY_UNION,               create_empty_union)                    \
    X(ADD_PART_TO_UNION,                add_part_to_union)                     \
                                                                               \
    X(CREATE_EMPTY_CARTESIAN,           create_empty_cartesian)                \
    X(CREATE_EMPTY_DISJOINT,            create_empty_disjoint)                 \
    X(CREATE_EMPTY_DIRECT,              create_empty_direct)                   \
                                                                               \
    X(ADD_PART_TO_COMPOSITE,            add_part_to_composite)                 \
    X(NAME_ELEMENT_OF_COMPOSITE,        name_element_of_composite)             \
                                                                               \
    X(CREATE_EMPTY_PARTICLE,            create_empty_particle)                 \
    X(ADD_PARTICLE,                     add_particle)                          \
    X(SUB_PARTICLE,                     sub_particle)

#define MAKE_ENUM(ENUM, NAME) \
    ENUM,

enum class AnalysisLevelInstruction {

    ALIL_INSTRUCTION_LIST(MAKE_ENUM)

};

#undef MAKE_ENUM
typedef AnalysisLevelInstruction ALIL;



class ALILCollection;


#if defined(__clang__)
    #define CONSUMABLE(state)      __attribute__((consumable(state)))
    #define MAKE_UNCONSUMED __attribute__((return_typestate(unconsumed)))
    #define MAKE_CONSUMED __attribute__((return_typestate(consumed)))
    #define CALLABLE_UNCONSUMED __attribute__((callable_when(unconsumed)))
    #define CALLABLE_CONSUMED __attribute__((callable_when(consumed)))
    #define CALLABLE_EITHER __attribute__((callable_when(unconsumed, consumed)))
    #define SET_CONSUMED           __attribute__((set_typestate(consumed)))
    #define PARAM_UNCONSUMED __attribute__((param_typestate(unconsumed))) 
    #define PARAM_CONSUMED __attribute__((param_typestate(consumed))) 
#else
    #define CONSUMABLE(state)      
    #define MAKE_UNCONSUMED 
    #define MAKE_CONSUMED
    #define CALLABLE_UNCONSUMED
    #define CALLABLE_CONSUMED
    #define CALLABLE_EITHER
    #define SET_CONSUMED
    #define PARAM_UNCONSUMED
    #define PARAM_CONSUMED
#endif


class AnalysisCommand {
    protected:
        AnalysisLevelInstruction instruction;

        std::optional<std::string> dest_argument;
        std::vector<std::string> source_arguments;

        std::optional<std::weak_ptr<Token>> source_token;

        AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token>);
        AnalysisCommand(AnalysisLevelInstruction inst);
    public:
        AnalysisCommand(const AnalysisCommand& other);

        AnalysisLevelInstruction get_instruction() const;
        // std::string get_argument(size_t pos);
        int get_num_arguments() const;

        bool has_dest_argument() const;
        const std::string get_dest_argument() const;
        int get_num_source_arguments() const;
        const std::string get_source_argument(size_t pos) const;
    
        void print_instruction();
        void print_instruction(int width_of_dest, int width_of_inst);

        std::string static instruction_to_text(AnalysisLevelInstruction inst);
};

class CONSUMABLE(unconsumed) AnalysisCommandBuilder : public AnalysisCommand{
    private:

        bool has_been_collected;
        void mark_collected() CALLABLE_UNCONSUMED SET_CONSUMED;

        bool dest_declared;
        bool source_declared;

        std::optional<std::weak_ptr<Token>> source_token;
    public:
        AnalysisCommandBuilder(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok) MAKE_UNCONSUMED;
        AnalysisCommandBuilder(AnalysisLevelInstruction inst) MAKE_UNCONSUMED;
        AnalysisCommandBuilder(const AnalysisCommandBuilder& other);
        AnalysisCommandBuilder(const AnalysisCommand& other);

        ~AnalysisCommandBuilder() CALLABLE_CONSUMED;

        void add_dest_argument(std::string arg) CALLABLE_UNCONSUMED;
        void add_source_argument(std::string arg) CALLABLE_UNCONSUMED;
        void add_empty_source() CALLABLE_UNCONSUMED;
        void add_empty_dest() CALLABLE_UNCONSUMED;
        std::string reserve_dest_arg_value(ALILConverter *) CALLABLE_UNCONSUMED;
       
        void collect_into(ALILCollection &) CALLABLE_UNCONSUMED SET_CONSUMED;
        void collect_into_reverse(ALILCollection &) CALLABLE_UNCONSUMED SET_CONSUMED;

        friend ALILCollection;
};

class ALILCollection {
    private:
        std::deque<AnalysisCommand> command_list;
        void collect_command(AnalysisCommandBuilder &in);
        void collect_command_reverse(AnalysisCommandBuilder &in);

    public:
        ALILCollection();

        const auto get_commands() const {
            return std::ranges::subrange(command_list.begin(), command_list.end());
        }       
        void print_collected_commands();

        friend AnalysisCommandBuilder;
};


#endif