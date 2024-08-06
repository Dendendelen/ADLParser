#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "node.h"
#include <vector>
class ASTVisitor {
    protected:
        virtual void visit_object(PNode node) = 0;
        virtual void visit_region(PNode node) = 0;

        virtual void visit_criteria(PNode node) = 0;

        virtual void visit_object_select(PNode node) = 0;
        virtual void visit_object_reject(PNode node) = 0;

        virtual void visit_region_select(PNode node) = 0;
        virtual void visit_use(PNode node) = 0;


        virtual void visit_if(PNode node) = 0;
        virtual void visit_condition(PNode node) = 0;

    public:
        void visit(PNode node);
        


        void visit_children(PNode node);
        void visit_children_after_index(PNode node, int index);


};

#endif