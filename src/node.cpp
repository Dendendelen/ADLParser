
#include "node.hpp"
#include <cassert>
#include <memory>



// private constructor allows node to have no parent
Node::Node(AST_type in): type(in), has_relevant_token(false), has_associated_string(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent), has_relevant_token(false), has_associated_string(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent, std::shared_ptr<Token> tok): type(in), m_parent(parent), relevant_token(tok), has_relevant_token(true), has_associated_string(false){}


std::string AST_type_to_string(AST_type type) {
    switch(type) {
        case AST_ERROR: return "AST_ERROR";
        case AST_EPSILON: return "AST_EPSILON";
        case AST_TERMINAL: return "TERMINAL";
        case AST_INPUT: return "INPUT";
        case AST_INFO: return "INFO";
        case AST_OBJECT: return "OBJECT";
        case AST_DEFINITION: return "DEFINITION";
        case AST_TABLE_DEF: return "TABLE_DEF";
        case AST_REGION: return "REGION";
        case AST_HISTO_LIST: return "HISTO_LIST";
        case AST_OBJECT_SELECT: return "OBJECT_SELECT";
        case AST_OBJECT_REJECT: return "OBJECT_REJECT";
        case AST_REGION_SELECT: return "REGION_SELECT";
        case AST_REGION_REJECT: return "REGION_REJECT";
        case AST_REGION_USE: return "REGION_USE";
        case AST_IF_STATEMENT: return "IF_STATEMENT";
        case AST_VARIABLE_LIST: return "VARIABLE_LIST";
        case AST_REGION_WEIGHT: return "WEIGHT_CMD";
        case AST_REGION_BINS: return "BINS_CMD";
        case AST_REGION_BIN: return "BIN_CMD";
        case AST_REGION_HISTOGRAM: return "HISTOGRAM";
        case AST_HISTOLIST_HISTOGRAM: return "HISTOLIST_HISTOGRAM";
        case AST_HISTO_USE: return "HISTO_USE";
        case AST_PARTICLE_LIST: return "PARTICLE_LIST";
        case AST_PARTICLE_SUM: return "PARTICLE_LIST";
        case AST_INDEX: return "INDEX";
        case AST_EXPRESSION: return "EXPRESSION";
        case AST_INTERVAL: return "INTERVAL";
        case AST_NEGATE: return "NEGATE";
        case AST_USER_FUNCTION: return "USER_FUNCTION";
        case AST_COMPOSITE: return "COMPOSITE";
        case AST_SORT_CMD: return "SORT_CMD";
        case AST_NAMED_PARTICLE_LIST: return "NAMED_PARTICLE_LIST";
        }
}

void Node::set_parent(std::shared_ptr<Node> in) {
    m_parent = in;
}

std::weak_ptr<Node> Node::get_parent() {
    return m_parent;
}

void Node::add_child(std::shared_ptr<Node> child) {
    if (child->get_ast_type() == AST_EPSILON) return;
    children.push_back((child));
}

std::vector<std::shared_ptr<Node>> &Node::get_children() {
    return children;
}

void Node::set_token(std::shared_ptr<Token> tok) {
    relevant_token = tok;
    has_relevant_token = true;
}

bool Node::has_token() {
    return has_relevant_token;
}

std::shared_ptr<Token> Node::get_token() {
    return relevant_token;
}

AST_type Node::get_ast_type() {
    return type;
}

std::string Node::get_ast_type_as_string() {
    return AST_type_to_string(type);
}

std::string Node::get_associated_string() {
    if (has_associated_string) return associated_string;
    return "";
} 

void Node::set_associated_string(std::string str_in) {
    assert(!has_associated_string);
    associated_string = str_in;
    has_associated_string = true;
}

Tree::Tree(AST_type in) {
    // create a node with no parent - the only node that is allowed to have this property
    root = std::shared_ptr<Node>(new Node(in));
}

std::shared_ptr<Node> Tree::get_root() {
    return root;
}

