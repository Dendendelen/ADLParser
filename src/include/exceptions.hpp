#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "tokens.hpp"
#include <stdexcept>
#include <string>
#include <vector>


class ADLCompilerException : public std::runtime_error {
    protected:
        PToken err_tok;
    public:
        ADLCompilerException(const char* what, PToken tok = nullptr) : std::runtime_error(what), err_tok(tok) {}
        PToken get_token() {
            return err_tok;
        }
};

class LexingException : public ADLCompilerException {
    public:
        LexingException(const char* what, PToken tok = nullptr) : ADLCompilerException(what, tok) {}
};
    
class ParsingException : public ADLCompilerException {
    private:
        PToken err_tok;
    public:
        ParsingException(const char* what, PToken tok = nullptr) : ADLCompilerException(what, tok) {}
};

class AnalysisLevelConversionException : public ADLCompilerException {
    private:
        PToken err_tok;
    public:
        AnalysisLevelConversionException(const char* what, PToken tok = nullptr) : ADLCompilerException(what, tok) {}
};

void raise_lexing_exception(PToken token);
void raise_parsing_exception(std::string error, PToken token);
void raise_analysis_conversion_exception(std::string error, PToken token);
void raise_non_implemented_conversion_exception(std::string inst, std::string context="");
void report_error(const std::runtime_error &e, std::string err_type);     

class ADLErrorLogger {
    protected:
        std::vector<ADLCompilerException> errors;
        bool has_fatal_error;

    public:
        ADLErrorLogger();
        std::vector<ADLCompilerException> &get_errors(); 
        virtual void error_handler();
        virtual void report_error(const std::runtime_error &e) = 0;
};

namespace CMDCOLOR {
    inline constexpr std::string red = "\033[31m";
    inline constexpr std::string reset = "\033[0m";
    inline constexpr std::string underline = "\033[4:3m";
    inline constexpr std::string bold =    "\033[1m";
}

#endif