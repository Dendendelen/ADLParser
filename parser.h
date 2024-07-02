#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

class Parser {
    private:
        Lexer &lexer;

    public:
        Parser(Lexer &lex);
        
        void parse();
        
        
};

#endif