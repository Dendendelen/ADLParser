
#include "node.h"
#include <memory>

Node::Node(AST_type in, std::shared_ptr<Node> parent): type(in), m_parent(parent) {}

void Node::set_parent(std::shared_ptr<Node> in) {
    m_parent = in;
}

std::shared_ptr<Node> Node::get_parent() {
    return m_parent;
}

Tree::Tree(AST_type in) {
    root = std::shared_ptr<Node>(new Node(in, nullptr));
}

std::shared_ptr<Node> Tree::get_root() {
    return root;
}
