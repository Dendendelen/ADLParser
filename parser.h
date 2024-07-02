#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "node.h"

class Parser {
    private:
        Lexer &lexer;
        Tree tree;

    public:
        Parser(Lexer &lex);
        
        void parse();

        void parse_input();
        PNode parse_initializations(PNode parent);
        PNode parse_count_formats(PNode parent);
        PNode parse_definitions_and_objects(PNode parent);
        PNode parse_commands(PNode parent);

        PNode parse_initialization(PNode parent);
        PNode parse_bool(PNode parent);
        PNode parse_description(PNode parent);

};

class ParsingException : public std::runtime_error {
    public:
        ParsingException(const char* what) : runtime_error(what) {}
};

#endif