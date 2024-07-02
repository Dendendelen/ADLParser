
#include "node.h"
#include <memory>

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent) {}

void Node::set_parent(std::shared_ptr<Node> in) {
    m_parent = in;
}

std::shared_ptr<Node> Node::get_parent() {
    return m_parent;
}

void Node::add_child(std::shared_ptr<Node> child) {
    if (child->get_ast_type() == AST_EPSILON) return;
    children.push_back((child));
}

std::vector<std::shared_ptr<Node>> Node::get_children() {
    return children;
}

void Node::set_token(std::shared_ptr<Token> tok) {
    relevant_token = tok;
}

std::shared_ptr<Token> Node::get_token() {
    return relevant_token;
}

AST_type Node::get_ast_type() {
    return type;
}

Tree::Tree(AST_type in) {
    root = std::shared_ptr<Node>(new Node(in, nullptr));
}

std::shared_ptr<Node> Tree::get_root() {
    return root;
}

