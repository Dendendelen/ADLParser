#include "lexer.h"

int main(int argc, char** argv) {
    Lexer lexer = Lexer();

    lexer.read_lines("adl_test.adl");

    lexer.print();

    return 0;
} 
