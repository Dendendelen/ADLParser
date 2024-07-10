#include "exceptions.h"

void raise_lexing_exception(PToken token) {
    std::stringstream stream;
    stream << "Malformed token \"" << token << "\", at line " << token->get_line() << ", column " << token->get_column() << std::endl;

    throw LexingException(stream.str().c_str());
}

void raise_parsing_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to parse \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << ": " << error << std::endl;

    throw ParsingException(stream.str().c_str());

}


