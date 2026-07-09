#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>

#include "lexer.hpp"

#define AST_NODE_LIST(X)                                                       \
    /* AST Error: set if there is an issue but a node must be returned */      \
    X(ERROR,                    error)                                         \
                                                                               \
    /* Terminal nodes - semantically meaningful token, possibly with lexeme */ \
    X(VARYING_TERMINAL,         varying_terminal)                              \
    X(OPERATOR_TERMINAL,        operator_terminal)                             \
    X(BUILTIN_FUNC_TERMINAL,    builtin_func_terminal)                         \
                                                                               \
    /* Non-terminal: */                                                        \
    X(INPUT,                    input)                                         \
                                                                               \
    /* Input subnodes */                                                       \
    X(INFO,                     info)                                          \
    X(DEFINITION,               definition)                                    \
    X(COMPOSITE,                composite)                                     \
    X(OBJECT,                   object)                                        \
    X(TABLE_DEF,                table_def)                                     \
    X(REGION,                   region)                                        \
    X(HISTO_LIST,               histo_list)                                    \
                                                                               \
    /* Info subnodes */                                                        \
    X(INITIALIZATIONS,          initializations)                               \
    X(INITIALIZATION,           initialization)                                \
                                                                               \
    /* Definition subnodes */                                                  \
    X(EXTERN_ATTR,              extern_attr)                                   \
    X(EXTERN_FUN,               extern_fun)                                    \
    X(EXTERN_PARTICLE,          extern_particle)                               \
    X(CORRECTIONLIB,            correctionlib)                                 \
                                                                               \
    /* Composite subnodes */                                                   \
    X(COMP_TYPE,                comp_type)                                     \
    X(COMP_CRITERIA,            comp_criteria)                                 \
    X(COMPOSITE_CARTESIAN,      composite_cartesian)                           \
    X(COMPOSITE_DISJOINT,       composite_disjoint)                            \
    X(COMPOSITE_DIRECT,         composite_direct)                              \
                                                                               \
    /* Object subnodes */                                                      \
    X(OBJECT_TYPE,              object_type)                                   \
    X(OBJECT_CRITERIA,          object_criteria)                               \
    X(OBJ_UNION,                obj_union)                                     \
    X(OBJ_SORT,                 obj_sort)                                      \
    X(ASCEND,                   ascend)                                        \
    X(DESCEND,                  descend)                                       \
    X(OBJECT_SELECT,            object_select)                                 \
    X(OBJECT_REJECT,            object_reject)                                 \
                                                                               \
    /* Region subnodes */                                                      \
    X(REGION_COMMANDS,          region_commands)                               \
    X(REGION_SELECT,            region_select)                                 \
    X(REGION_REJECT,            region_reject)                                 \
    X(REGION_USE,               region_use)                                    \
    X(REGION_WEIGHT,            region_weight)                                 \
    X(REGION_BIN,               region_bin)                                    \
    X(REGION_BINS,              region_bins)                                   \
    X(HISTO_USE,                histo_use)                                     \
    X(REGION_HISTOGRAM,         region_histogram)                              \
                                                                               \
    /* Histolist subnodes */                                                   \
    X(HISTO_ENTRIES,            histo_entries)                                 \
    X(HISTOLIST_HISTOGRAM,      histolist_histogram)                           \
                                                                               \
    /* General lists */                                                        \
    X(NAMED_PARTICLE_LIST,      named_particle_list)                           \
    X(PARTICLE_LIST,            particle_list)                                 \
    X(PARTICLE_SUM,             particle_sum)                                  \
    X(VARIABLE_LIST,            variable_list)                                 \
    X(LITERAL_NUMBER_LIST,      literal_number_list)                           \
    X(STRING_LIST,              string_list)                                   \
                                                                               \
    /* Expression subnodes */                                                  \
    X(EXPRESSION,               expression)                                    \
    X(INTERVAL,                 interval)                                      \
    X(IF_STATEMENT,             if_statement)                                  \
    X(SORT_EXPRESSION,          sort_expression)                               \
    X(MIN_EXPRESSION,           min_expression)                                \
    X(MAX_EXPRESSION,           max_expression)                                \
    X(INDEX_OPERATOR,           index_operator)                                \
    X(INDEX,                    index)                                         \
    X(UNBOUNDED,                unbounded)                                     \
    X(THIS,                     this_node)                                     \
    X(TRUE,                     true_literal)                                  \
    X(FALSE,                    false_literal)                                 \
                                                                               \
    /* Unary operators */                                                      \
    X(NEGATE,                   negate)                                        \
    X(L_NOT,                    l_not)                                         \
    X(USER_FUNCTION,            user_function)

#define MAKE_ENUM(ENUM, NAME)\
    ENUM,

enum class AST_type{

    AST_NODE_LIST(MAKE_ENUM)

};

#undef MAKE_ENUM


typedef AST_type AST;

class Node {
    private:
        Node(AST_type in);
        std::vector<std::shared_ptr<Node>> children;
        std::weak_ptr<Node> m_parent;

        int line_number;
        int column_number;
        AST_type type;

        std::shared_ptr<Token> relevant_token;
        bool has_relevant_token;

        std::string associated_string;
        bool has_associated_string;

        int unique_id;

    public:
        Node(AST_type in, std::shared_ptr<Node> parent);
        Node(AST_type in, std::shared_ptr<Node> parent, std::shared_ptr<Token> tok);
        
        void set_parent(std::shared_ptr<Node> parent);
        std::weak_ptr<Node> get_parent();

        void add_child(std::shared_ptr<Node> child);
        std::vector<std::shared_ptr<Node>> &get_children();
        
        void set_token(std::shared_ptr<Token> tok);
        std::shared_ptr<Token> get_token();
        bool has_token();

        AST_type get_ast_type();
        std::string get_ast_type_as_string();

        std::string get_associated_string();
        void set_associated_string(std::string);
        
        friend class Tree;
};

typedef std::shared_ptr<Node> PNode;

#define NODE_TYPE(X) \
    class XNode : public Node {public: std::string get_ast_type_as_string() override{return "X"}; };

class ErrorNode : public Node {};

class Tree {
    private:
        std::shared_ptr<Node> root;

    public:
        Tree(AST_type in);
        std::shared_ptr<Node> get_root();

};

#endif