#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
    std::unique_ptr<Lexer> lexer(new Lexer());

    lexer->read_lines("adl_test_2.adl");

    // lexer->print();

    Parser parser = Parser(lexer.release());
    parser.parse();

    parser.print_parse_dot();

    return 0;
} 
