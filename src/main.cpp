#include "lexer.h"
#include "parser.h"
#include "timber_converter.h"
#include <iostream>

int main(int argc, char** argv) {

    std::string argument;

    if (argc < 2) {
        std::cerr << "Usage: main FILENAME.adl [timber]|[lex]|[parse]|[alil] " << std::endl;
        return -1;
    }

    std::string filename(argv[1]);

    if (argc > 2) argument = std::string(argv[2]);
    else argument = "timber";

    std::unique_ptr<Lexer> lexer(new Lexer());

    lexer->read_lines(filename);
    // lexer->read_lines("adl_test_2.adl");

    if (argument == "lex") {
        lexer->print();
        return 0;
    }
    
    std::unique_ptr<Parser> parser(new Parser(lexer.release()));
    parser->parse();

    if (argument == "parse") {
        parser->print_parse_dot();
        return 0;
    }

    std::unique_ptr<ALIConverter> alil(new ALIConverter());
    alil->visitation(parser->get_root());

    if (argument == "alil") {
        alil->print_commands();
        return 0;
    }

    if (argument == "timber") {
        std::unique_ptr<TimberConverter> timber(new TimberConverter(alil.release()));
        timber->print_timber();
        return 0;
    }

    std::cerr << "Error: invalid argument: " << argument << std::endl;
    return -1;
} 
