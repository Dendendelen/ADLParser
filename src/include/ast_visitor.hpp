#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "node.hpp"


#define VISIT_DISPATCH_DECLARE(ENUM, NAME) \
    virtual void visit_##NAME(PNode node) { visit_children(node); };


class ASTVisitor {
    protected:

        AST_NODE_LIST(VISIT_DISPATCH_DECLARE)

    public:
        void visit(PNode node);

        virtual void visit_children(PNode node);
        virtual void visit_children_before_index(PNode node, int index);
        virtual void visit_children_after_index(PNode node, int index);


};

#undef VISIT_DISPATCH_DECLARE
#endif