#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>

#include "lexer.h"

enum AST_type{

    // Set if an error has occurred in the AST
    AST_ERROR,

    // Terminal - if detected, then the parsed token matters
    TERMINAL,

    // Nonterminals
    INPUT,
    INITIALIZATIONS,
    INITIALIZATION,
    COUNT_FORMATS,
    DEFINITIONS_OBJECTS,
    OBJECTS_DEFINITIONS,
    COMMANDS,
    DESCRIPTION,
    BOOL,
    DEFINITIONS,
    OBJECTS,
    DEFINITION,
    COUNT_FORMAT,
    VARIABLE_LIST,
    NUMBER,

    // Defines a specific type of lepton
    LEPTON_TYPE,
    ERR_TYPE,
    SYST_VTYPE,

    PARTICLES,
    INDEX,
    BOXLIST,

    VALUES,
    E, // TODO: this is an expression, make sure to precedence climb this
    FUNCTION,

    LIST2, // TODO: this should not be a name
    LIST3,

    CONDITION,

    ID_LIST,
    OBJECT_BLOCS,
    OBJECT_BLOC,
    CRITERIA,
    HAMHUM,

    COUNT, //TODO: probbaly change
    A_COUNT,

    A_BOX,

    CRITERION,
    ACTION,
    
    COMMAND,
    IFSTATEMENT,
    

};

class Node {
    private:
        std::vector<std::shared_ptr<Node>> children;
        std::shared_ptr<Node> m_parent;
        int line_number;
        int column_number;
        AST_type type;

        std::shared_ptr<Token> relevant_token;

        int unique_id;

    public:
        Node(AST_type in, std::shared_ptr<Node> parent);
        
        void set_parent(std::shared_ptr<Node> parent);
        std::shared_ptr<Node> get_parent();
        

};

class Tree {
    private:
        std::shared_ptr<Node> root;

    public:
        Tree(AST_type in);
        std::shared_ptr<Node> get_root();
};

#endif