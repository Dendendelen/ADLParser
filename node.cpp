
#include "node.h"
#include <memory>

// private constructor allows node to have no parent
Node::Node(AST_type in): type(in), has_relevant_token(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent), has_relevant_token(false) {}

Node::Node(AST_type in, std::shared_ptr<Node> parent, std::shared_ptr<Token> tok): type(in), m_parent(parent), relevant_token(tok), has_relevant_token(true){}


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

Tree::Tree(AST_type in) {
    // create a node with no parent - the only node that is allowed to have this property
    root = std::shared_ptr<Node>(new Node(in));
}

std::shared_ptr<Node> Tree::get_root() {
    return root;
}

