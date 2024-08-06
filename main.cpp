#include "lexer.h"
#include "parser.h"
#include "timber_converter.h"

int main(int argc, char** argv) {
    std::unique_ptr<Lexer> lexer(new Lexer());

    lexer->read_lines("ex01.adl");

    // lexer->print();

    std::unique_ptr<Parser> parser(new Parser(lexer.release()));
    parser->parse();

    parser->print_parse_dot();

    std::unique_ptr<ALIConverter> alil(new ALIConverter());
    alil->vistitation(parser->get_root());
    alil->print_commands();

    std::unique_ptr<TimberConverter> timber(new TimberConverter());

    return 0;
} 
