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
    AST_TERMINAL,

    // Nonterminals
    AST_INPUT,

    // input
    AST_INFO,
    AST_DEFINITION,
    AST_COMPOSITE,
    AST_OBJECT,
    AST_TABLE_DEF,
    AST_REGION,
    AST_HISTO_LIST,


    // info
    AST_INITIALIZATIONS,
    AST_INITIALIZATION,

    // definition
    AST_EXTERN_ATTR,
    AST_EXTERN_FUN,
    AST_EXTERN_PARTICLE,

    // composite
    AST_COMP_TYPE,
    AST_COMP_CRITERIA,


    // object
    AST_OBJECT_TYPE,
    AST_OBJECT_CRITERIA,

    AST_OBJECT_SELECT,
    AST_OBJECT_REJECT,




    
    AST_SORT_CMD,

    AST_IF_STATEMENT,

    AST_VARIABLE_LIST,

    AST_LITERAL_NUMBER_LIST,

    // region 
    AST_REGION_COMMANDS,
    
    AST_REGION_SELECT,
    AST_REGION_REJECT,
    AST_REGION_USE,
    AST_REGION_WEIGHT,
    AST_REGION_BIN,
    AST_REGION_BINS,
    AST_HISTO_USE,
    AST_REGION_HISTOGRAM,

    // histo_list
    AST_HISTOLIST_HISTOGRAM,

    AST_NAMED_PARTICLE_LIST,
    AST_PARTICLE_LIST,
    AST_PARTICLE_SUM,
    AST_INDEX,

    AST_EXPRESSION, 

    AST_INTERVAL,
    
    AST_NEGATE,

    AST_USER_FUNCTION,
    

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

class Tree {
    private:
        std::shared_ptr<Node> root;

    public:
        Tree(AST_type in);
        std::shared_ptr<Node> get_root();

};

#endif