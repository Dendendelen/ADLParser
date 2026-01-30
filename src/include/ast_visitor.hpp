#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "node.hpp"
#include <vector>
class ASTVisitor {
    protected:
        virtual void visit_object(PNode node) = 0;
        virtual void visit_region(PNode node) = 0;
        virtual void visit_definition(PNode node) = 0;

        virtual void visit_criteria(PNode node) = 0;

        virtual void visit_object_select(PNode node) = 0;
        virtual void visit_object_reject(PNode node) = 0;

        virtual void visit_region_select(PNode node) = 0;
        virtual void visit_region_reject(PNode node) = 0;

        virtual void visit_use(PNode node) = 0;

        virtual void visit_expression(PNode node) = 0;

        virtual void visit_if(PNode node) = 0;
        virtual void visit_condition(PNode node) = 0;

        virtual void visit_sort(PNode node) = 0;

        virtual void visit_histo_use(PNode node) = 0;
        virtual void visit_histogram(PNode node) = 0;
        virtual void visit_histo_list(PNode node) = 0;

        virtual void visit_particle_sum(PNode node) = 0;

        virtual void visit_table_def(PNode node) = 0;

        virtual void visit_bin(PNode node) = 0;
        virtual void visit_bin_list(PNode node) = 0;

        virtual void visit_weight(PNode node) = 0;

    public:
        void visit(PNode node);
        


        void visit_children(PNode node);
        void visit_children_after_index(PNode node, int index);


};

#endif