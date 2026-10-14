#include "alil_converter.hpp"
#include "coffea_converter.hpp"
#include "config.hpp"
#include "exceptions.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "timber_converter.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main(int argc, char** argv) {

    std::string argument;

    bool success = true;

    if (argc < 2) {
        std::cerr << "Usage: main FILENAME.adl [genconfig]|[timber]|[coffea]|[lex]|[parse]|[alil] " << std::endl;
        return -1;
    }

    std::string filename(argv[1]);
    Config config("config.txt");

    if (argc > 2) argument = std::string(argv[2]);
    else argument = "timber";

    if (argument == "genconfig") {
        return 0;
    }

    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>();

    success = success && lexer->read_file(filename);

    if (argument == "lex") {
        lexer->print();
        return success ? 0 : 1;
    }
    
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(lexer.release());
    success = success && parser->parse();

    if (argument == "parse") {
        parser->print_parse_dot();
        return success ? 0 : 1;
    }

    std::unique_ptr<ALILConverter> alil = std::make_unique<ALILConverter>(config);
    alil->visitation(parser->get_root());

    if (argument == "alil") {
        alil->print_commands();
        return success ? 0 : 1;
    }

    if (!success) {
        std::cerr << CMDCOLOR::red << CMDCOLOR::bold <<  "Fatal error in compiler, ending here." << CMDCOLOR::reset << std::endl;
        return 1;  
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
