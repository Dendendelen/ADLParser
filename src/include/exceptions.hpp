#include <string>
#include <sstream>
#include "lexer.hpp"

class LexingException : public std::runtime_error {
    public:
        LexingException(const char* what) : runtime_error(what) {}
};

class ParsingException : public std::runtime_error {
    public:
        ParsingException(const char* what) : runtime_error(what) {}
};

class AnalysisLevelConversionException : public std::runtime_error {
    public:
        AnalysisLevelConversionException(const char* what) : runtime_error(what) {}
};

void raise_lexing_exception(PToken token);
void raise_parsing_exception(std::string error, PToken token);
void raise_analysis_conversion_exception(std::string error, PToken token);
void raise_non_implemented_conversion_exception(std::string inst);