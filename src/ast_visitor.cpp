#include "ast_visitor.hpp"
#include "node.hpp"

void ASTVisitor::visit(PNode node) {
    switch (node->get_ast_type()) {
        case OBJECT:
            return visit_object(node);
        case DEFINITION:
            return visit_definition(node);
        case REGION:
            return visit_region(node);
        case CONDITION:
            return visit_condition(node);
        case IF_STATEMENT:
            return visit_if(node);
        case SORT_CMD:
            return visit_sort(node);
        case OBJECT_SELECT:
            return visit_object_select(node);
        case OBJECT_REJECT:
            return visit_object_reject(node);
        case REGION_SELECT:
            return visit_region_select(node);
        case REGION_USE:
            return visit_use(node);
        case HISTO_LIST:
            return visit_histo_list(node);
        case HISTOGRAM: case HISTOLIST_HISTOGRAM:
            return visit_histogram(node);
        case HISTO_USE:
            return visit_histo_use(node);
        case CUTFLOW_USE:
            return visit_cutflow_use(node);
        case PARTICLE_SUM:
            return visit_particle_sum(node);
        case EXPRESSION:
            return visit_expression(node);
        case TABLE_DEF:
            return visit_table_def(node);
        case BIN_CMD:
            return visit_bin(node);
        case BINS_CMD:
            return visit_bin_list(node);     
        case WEIGHT_CMD:
            return visit_weight(node);   

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