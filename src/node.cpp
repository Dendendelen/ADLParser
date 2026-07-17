
#include "node.hpp"
#include <cassert>
#include <memory>



// private constructor allows node to have no parent
Node::Node(AST_type in): type(in), has_relevant_token(false), has_associated_string(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent), has_relevant_token(false), has_associated_string(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent, std::shared_ptr<Token> tok): type(in), m_parent(parent), relevant_token(tok), has_relevant_token(true), has_associated_string(false){}


#define TYPE_TO_STRING(ENUM, NAME) \
    case AST::ENUM: return #ENUM;

std::string AST_type_to_string(AST_type type) {
    switch(type) {
        AST_NODE_LIST(TYPE_TO_STRING)
    }
}

void Node::set_parent(std::shared_ptr<Node> in) {
    m_parent = in;
}

std::weak_ptr<Node> Node::get_parent() {
    return m_parent;
}

void Node::add_child(std::shared_ptr<Node> child) {
    children.push_back((child));
}

const std::span<PNode const> Node::get_children() const {
    return children;
}

std::shared_ptr<Node> Node::get_child(int index) {
    assert(index < children.size());
    return children[index];
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

std::string Node::consume_associated_string() {
    std::string out;
    out = has_associated_string ? associated_string : "";
    has_associated_string = false;
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

