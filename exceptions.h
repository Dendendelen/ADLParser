#include <string>
#include <sstream>
#include "lexer.h"

class LexingException : public std::runtime_error {
    public:
        LexingException(const char* what) : runtime_error(what) {}
};

class ParsingException : public std::runtime_error {
    public:
        ParsingException(const char* what) : runtime_error(what) {}
};

void raise_lexing_exception(PToken token);
void raise_parsing_exception(std::string error, PToken token);