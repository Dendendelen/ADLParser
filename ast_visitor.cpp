#include "ast_visitor.h"
#include "node.h"

void ASTVisitor::visit(PNode node) {
    switch (node->get_ast_type()) {
        case OBJECT:
            return visit_object(node);
        case REGION:
            return visit_region(node);
        case CONDITION:
            return visit_condition(node);
        case IF:
            return visit_if(node);
        case OBJECT_SELECT:
            return visit_object_select(node);
        case OBJECT_REJECT:
            return visit_object_reject(node);
        case REGION_SELECT:
            return visit_region_select(node);
        case REGION_USE:
            return visit_use(node);
        default:
            return visit_children(node);
    }
}

void ASTVisitor::visit_children(PNode node) {
    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {
        visit(*it);
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