#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
    Lexer lexer = Lexer();

    lexer.read_lines("adl_test.adl");

    lexer.print();

    Parser parser = Parser(lexer);

    return 0;
} 
