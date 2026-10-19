#include "exceptions.hpp"
#include <cstddef>
#include <iostream>
#include <sstream>

std::string line_indicator(PToken token) {
    std::string source = token->get_source_line_text();

    size_t first_non_whitespace = source.length() > 0 ? source.find_first_not_of(" \t\n\r\v\f") : 0;

    std::stringstream stream;

    stream << "\t╷ \n\t│ ";

    for (int i = first_non_whitespace; i < token->get_column()-1 && i < source.size(); i++) {
        stream << source[i];
    }

    stream << CMDCOLOR::underline;

    size_t start_of_token = token->get_column()-1;
    size_t end_of_token = start_of_token + token->get_lexeme().size();

    for (size_t i = start_of_token; i < end_of_token; i++) {
        stream << source[i];
    }

    stream << CMDCOLOR::reset;

    for (size_t i = end_of_token; i < source.size(); i++) {
        stream << source[i];
    }

    stream << CMDCOLOR::reset << "\n\t│ ";

    // stream << "\n\n" << source << "\n";
    
    for (int i = first_non_whitespace; i < token->get_column()-1; i++) {
        stream << " ";
    }

    stream << "^" << "\n";

    return stream.str();
}

void raise_lexing_exception(PToken token) {
    std::stringstream stream;
    stream << "Malformed token \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << "\n";

    stream << line_indicator(token);

    throw LexingException(stream.str().c_str(), token);
}

void raise_parsing_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to parse \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << ": " << error << "\n";

    stream << line_indicator(token);

    throw ParsingException(stream.str().c_str(), token);

}

void raise_analysis_conversion_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to convert \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << ": " << error << "\n";

    stream << line_indicator(token);

    throw AnalysisLevelConversionException(stream.str().c_str(), token);

}

void raise_non_implemented_conversion_exception(std::string inst, std::string context) {
    std::stringstream stream;
    stream << "No implemented conversion exists for \"" << inst << "\"";
    if (context != "") {
        stream << " within the context: " << context;
    }

    throw AnalysisLevelConversionException(stream.str().c_str());
}

void report_error(const std::runtime_error &e, std::string err_type) {        
    std::cerr << CMDCOLOR::red << "ERROR: " << CMDCOLOR::reset << err_type << ": \n\t" << e.what() << std::endl;
}

ADLErrorLogger::ADLErrorLogger(): has_fatal_error(false) {}

void ADLErrorLogger::error_handler() {
    if (auto exception = std::current_exception()) {
        try {
            std::rethrow_exception(exception);
        } catch (const ADLCompilerException &e) {
            errors.push_back(e);
            has_fatal_error = true;
            report_error(e);
        } catch (const std::runtime_error &e) {
            has_fatal_error = true;
            report_error(e);
        }
    }
}

std::vector<ADLCompilerException> &ADLErrorLogger::get_errors() {
    return errors;
} 
