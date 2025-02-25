#ifndef LEXER_H
#define LEXER_H

#include "tokens.hpp"
#include <exception>
#include <regex>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>

class Token {
    private:
        Token_type type;
        std::string lexeme;
        int line_number;
        int column_number;
    public:
        Token(Token_type);
        void set_data(int line, int column, std::string actual_lexeme);
        int get_line();
        int get_column();
        Token_type get_token();
        std::string get_lexeme();
};

class Lexer {
    private:
        Token_type identify_token(std::string &token);
        void lex_token(std::string &token, int &line_number, int &column_number);
        std::vector<std::shared_ptr<Token>> tokens;
        std::vector<std::shared_ptr<Token>> non_whitespace_tokens;
        std::vector<std::shared_ptr<Token>>::iterator current_token;
        bool verbose;
        bool is_symbol (char c);
        bool is_delimiter (char c);
        std::string convert_to_uppercase(std::string &input);


        std::regex reg_int;
        std::regex reg_decimal;
        std::regex reg_scientific;
        std::regex reg_varname;
        std::regex reg_string;

        std::regex reg_whitespace;

    public: 
        Lexer();
        void reset();
        std::shared_ptr<Token> next();

        void expect_and_consume(Token_type type);
        void expect_and_consume(Token_type type, std::string error);

        std::shared_ptr<Token> peek(int lookahead);
        void read_lines(std::string filename, bool is_verbose = false);
        void print();
        void erase_whitespace();
};

typedef std::shared_ptr<Token> PToken;


#endif