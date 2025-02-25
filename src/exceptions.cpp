#include "exceptions.hpp"

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

void raise_analysis_conversion_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to convert \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << ": " << error << std::endl;

    throw AnalysisLevelConversionException(stream.str().c_str());

}

void raise_non_implemented_conversion_exception(std::string inst) {
    std::stringstream stream;
    stream << "No implemented conversion exists for \"" << inst << "\"";

    throw AnalysisLevelConversionException(stream.str().c_str());
}