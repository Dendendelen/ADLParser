#include "ast_visitor.hpp"
#include "node.hpp"


#define VISIT_DISPATCH(ENUM, NAME) \
    case AST::ENUM: visit_##NAME(node); return; \


void ASTVisitor::visit(PNode node) {
    switch (node->get_ast_type()) {
        AST_NODE_LIST(VISIT_DISPATCH)
    }
}

#undef VISIT_DISPATCH

void ASTVisitor::visit_children(PNode node) {
    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {
        visit(*it);
    }
}

void ASTVisitor::visit_children_before_index(PNode node, int index) {
    int i = 0;
    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {
        if (i < index) {
            visit(*it);
        }
        i++;
    }  
}


void ASTVisitor::visit_children_after_index(PNode node, int index) {
    int i = 0;
    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {
        if (i > index) {
            visit(*it);
        }
        i++;
    }
}