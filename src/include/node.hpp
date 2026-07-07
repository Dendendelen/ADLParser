#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>

#include "lexer.hpp"

enum AST_type{

    // Set if an error has occurred in the AST
    AST_ERROR,

    // AST node for an epsilon expression - empty, and detected by parser as a key to remove this node from consideration
    AST_EPSILON,

    // Terminal - if detected, then the parsed token matters
    TERMINAL,

    // Nonterminals
    INPUT,

    INFO,
    OBJECT,
    DEFINITION,
    COMPOSITE,
    TABLE_DEF,
    REGION,
    HISTO_LIST,

    OBJECT_SELECT,
    OBJECT_REJECT,

    REGION_SELECT,
    REGION_REJECT,
    REGION_USE,

    IF_STATEMENT,

    VARIABLE_LIST,

    WEIGHT_CMD,
    HISTO_CMD,
    BINS_CMD,
    BIN_CMD,
    SORT_CMD,

    HISTOGRAM,
    HISTOLIST_HISTOGRAM,
    HISTO_USE,

    NAMED_PARTICLE_LIST,
    PARTICLE_LIST,
    PARTICLE_SUM,
    INDEX,

    EXPRESSION, 

    INTERVAL,
    
    NEGATE,

    USER_FUNCTION,
    

};

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
        
        friend class Tree;
};

typedef std::shared_ptr<Node> PNode;

class Tree {
    private:
        std::shared_ptr<Node> root;

    public:
        Tree(AST_type in);
        std::shared_ptr<Node> get_root();

};

#endif