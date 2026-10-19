#include "exceptions.hpp"
#include "lexer.hpp"
#include <sstream>

static inline std::string line_column_string(PToken tok) {
    std::stringstream stream;
    int line = tok->get_line();
    int col = tok->get_column();
    stream << ", at line " << line << ", column " << col;
    return stream.str();
}

static inline std::string token_lexeme_string(PToken tok) {
    std::stringstream stream;
    stream << "\"" << tok->get_lexeme() << "\"";
    return stream.str();
}


void raise_lexing_exception(PToken token) {
    std::stringstream stream;
    stream << "Malformed token " << token_lexeme_string(token) << line_column_string(token) << std::endl;
    throw LexingException(stream.str().c_str());
}

void raise_parsing_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to parse " << token_lexeme_string(token) << line_column_string(token) << ": " << error << std::endl;
    throw ParsingException(stream.str().c_str());

}

void raise_analysis_conversion_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to convert " << token_lexeme_string(token) << line_column_string(token) << ": " << error << std::endl;

    throw AnalysisLevelConversionException(stream.str().c_str());

}

void raise_non_implemented_conversion_exception(std::string inst, std::string context) {
    std::stringstream stream;
    stream << "No implemented conversion exists for \"" << inst << "\"";
    if (context != "") {
        stream << " within the context: " << context;
    }

    throw AnalysisLevelConversionException(stream.str().c_str());
}

void raise_type_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to resolve type of " << token_lexeme_string(token) << line_column_string(token) << ":" << error << std::endl;
}