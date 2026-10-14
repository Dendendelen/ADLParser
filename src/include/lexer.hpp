#ifndef LEXER_H
#define LEXER_H

#include "tokens.hpp"
#include <regex>
#include <string>
#include <memory>
#include <vector>

class Token {
    private:
        Token_type type;
        std::string lexeme;
        int line_number;
        int column_number;
        std::shared_ptr<std::string> source_line;

    public:
        Token(Token_type);
        void set_data(int line, int column, std::string actual_lexeme, std::shared_ptr<std::string> line_reference = nullptr);
        int get_line();
        int get_column();
        Token_type get_token_type();
        std::string get_token_type_as_string();
        std::string get_lexeme();
        std::string get_source_line_text();
};

class Lexer {
    private:
        Token_type identify_token(std::string &token);
        void lex_token(std::string &token, int &line_number, int &column_number, std::shared_ptr<std::string> line_reference = nullptr);
        std::vector<std::shared_ptr<Token>> tokens;
        std::vector<std::shared_ptr<Token>> non_whitespace_tokens;
        std::vector<std::shared_ptr<Token>>::iterator current_token;
        bool verbose;
        bool has_fatal_error;

        bool is_symbol (char c);
        bool is_delimiter (char c);
        std::string convert_to_uppercase(std::string &input);


        std::regex reg_int;
        std::regex reg_decimal;
        std::regex reg_scientific;
        std::regex reg_varname;
        std::regex reg_string;

        std::regex reg_whitespace;

        std::shared_ptr<Token> next();

    public: 
        Lexer();
        void reset();
        void reset_with_whitespace();

        void expect_and_consume(Token_type type, std::string error = "");

        std::shared_ptr<Token> peek(int lookahead);
        void read_line(std::string content, int line_number);
        void read_lines(std::vector<std::string> &content, bool is_verbose = false);
        bool read_file(std::string filename, bool is_verbose = false);
        void print();
        void erase_whitespace();
};

typedef std::shared_ptr<Token> PToken;


#endif