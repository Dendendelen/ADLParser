#include "ali_converter.hpp"
#include "coffea_converter.hpp"
#include "config.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "timber_converter.hpp"
#include <iostream>
#include <memory>
#include <utility>

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

    Config config("config.txt");

    std::unique_ptr<ALIConverter> alil = std::make_unique<ALIConverter>(config);
    alil->visitation(parser->get_root());

    if (argument == "alil") {
        alil->print_commands();
        return 0;
    }

    std::unique_ptr<ALILToFrameworkCompiler> final_state_compiler;

    if (argument == "timber") {
        final_state_compiler = std::make_unique<TimberConverter>(alil.release(), config);
    }
    
    if (argument == "coffea") {
        final_state_compiler = std::make_unique<CoffeaConverter>(alil.release(), config);    
    }

    if (!final_state_compiler) {
        std::cerr << "Error: invalid argument: " << argument << std::endl;
        return -1;
    }

    final_state_compiler->print();
    return 0;
} 
