
#include "node.hpp"
#include <memory>



// private constructor allows node to have no parent
Node::Node(AST_type in): type(in), has_relevant_token(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent), has_relevant_token(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent, std::shared_ptr<Token> tok): type(in), m_parent(parent), relevant_token(tok), has_relevant_token(true){}


std::string AST_type_to_string(AST_type type) {
    switch(type) {
        case AST_ERROR: return "AST_ERROR";
        case AST_EPSILON: return "AST_EPSILON";
        case TERMINAL: return "TERMINAL";
        case INPUT: return "INPUT";
        case INFO: return "INFO";
        case OBJECT: return "OBJECT";
        case DEFINITION: return "DEFINITION";
        case TABLE_DEF: return "TABLE_DEF";
        case REGION: return "REGION";
        case HISTO_LIST: return "HISTO_LIST";
        case OBJECT_SELECT: return "OBJECT_SELECT";
        case OBJECT_REJECT: return "OBJECT_REJECT";
        case REGION_SELECT: return "REGION_SELECT";
        case REGION_REJECT: return "REGION_REJECT";
        case REGION_USE: return "REGION_USE";
        case IF_STATEMENT: return "IF_STATEMENT";
        case VARIABLE_LIST: return "VARIABLE_LIST";
        case WEIGHT_CMD: return "WEIGHT_CMD";
        case HISTO_CMD: return "HISTO_CMD";
        case BINS_CMD: return "BINS_CMD";
        case BIN_CMD: return "BIN_CMD";
        case HISTOGRAM: return "HISTOGRAM";
        case HISTOLIST_HISTOGRAM: return "HISTOLIST_HISTOGRAM";
        case HISTO_USE: return "HISTO_USE";
        case PARTICLE_LIST: return "PARTICLE_LIST";
        case PARTICLE_SUM: return "PARTICLE_LIST";
        case INDEX: return "INDEX";
        case EXPRESSION: return "EXPRESSION";
        case INTERVAL: return "INTERVAL";
        case NEGATE: return "NEGATE";
        case USER_FUNCTION: return "USER_FUNCTION";
        case COMPOSITE: return "COMPOSITE";
        case SORT_CMD: return "SORT_CMD";
        case NAMED_PARTICLE_LIST: return "NAMED_PARTICLE_LIST";
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

Tree::Tree(AST_type in) {
    // create a node with no parent - the only node that is allowed to have this property
    root = std::shared_ptr<Node>(new Node(in));
}

std::shared_ptr<Node> Tree::get_root() {
    return root;
}

