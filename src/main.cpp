#include "coffea_converter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "timber_converter.hpp"
#include <iostream>
#include <memory>

int main(int argc, char** argv) {

    std::string argument;

    if (argc < 2) {
        std::cerr << "Usage: main FILENAME.adl [timber]|[coffea]|[lex]|[parse]|[alil] " << std::endl;
        return -1;
    }

    std::string filename(argv[1]);

    if (argc > 2) argument = std::string(argv[2]);
    else argument = "timber";

    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>();

    lexer->read_lines(filename);
    // lexer->read_lines("adl_test_2.adl");

    if (argument == "lex") {
        lexer->print();
        return 0;
    }
    
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(lexer.release());
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
        std::unique_ptr<TimberConverter> timber = std::make_unique<TimberConverter>(alil.release());
        timber->print_timber();
        return 0;
    }
    
    if (argument == "coffea") {
        std::unique_ptr<CoffeaConverter> coffea = std::make_unique<CoffeaConverter>(alil.release());
        coffea->print_coffea();
        return 0;
    }

    std::cerr << "Error: invalid argument: " << argument << std::endl;
    return -1;
} 
