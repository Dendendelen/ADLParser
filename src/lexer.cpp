#include "lexer.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <regex>
#include <sstream>
#include <vector>

#include "exceptions.hpp"
#include "tokens.hpp"

Token::Token(Token_type in): type(in) {}

void Token::set_data(int line, int column, std::string actual_lexeme) {
    line_number = line;
    column_number = column;
    lexeme = actual_lexeme;
}

int Token::get_line() {
    return line_number;
}
int Token::get_column() {
    return column_number;
}
Token_type Token::get_token_type() {
    return type;
}
std::string Token::get_lexeme() {
    return lexeme;
}

// Is the character an inherently delimiting symbol?
bool Lexer::is_symbol (char c) {
    if (c == '=' || c == '!' || c == '!' || c == '~' || c == '<' || c == '>' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == '&' || c == '|' || c == '+' || c == '-' || c == '*' || c == '/' || c == '?' || c == '^' || c == ',' || c == '.') return true;
    return false;
}

// Is the character a delimiting space?
bool Lexer::is_delimiter (char c) {
    std::string s(1, c);
    if (std::regex_match(s, reg_whitespace)) return true;

    return false;
}

std::string Lexer::convert_to_uppercase(std::string &input) {
    std::string uppercase;
    for (auto it = input.begin(); it != input.end(); ++it) {
            char c = *it;

            // If lowercase ASCII letter, make it uppercase
            if (c >= 'a' && c <= 'z') c -= 32;

            uppercase += c;
        }
    return uppercase;
}

Lexer::Lexer() {
    current_token = tokens.begin();

    reg_int = std::regex("-{0,1}[0-9]+");
    reg_decimal = std::regex("-{0,1}[0-9]*\\.[0-9]+");
    reg_scientific = std::regex("-{0,1}[0-9]+\\.{0,1}[0-9]*[Ee][-+]{0,1}[0-9]+");
    reg_varname = std::regex("[A-Za-z][A-Za-z0-9_]*");
    reg_string= std::regex("\"[^\"]*\"");

    reg_whitespace = std::regex("\\s");

}

Token_type Lexer::identify_token(std::string &token) {
    std::string uppercase_token = convert_to_uppercase(token);

    if (verbose) std::cout << "Lexing " << token << std::endl;
    
    // If we start with a hash, then this is instantly a comment
    if (token.front() == '#') return TOK::COMMENT;
    if (std::regex_match(token, std::regex("\\s+"))) return TOK::SPACE;

    // Top level ADL syntax
    if (uppercase_token == "DEF" || uppercase_token == "DEFINE") return TOK::DEF;
    if (uppercase_token == "ALGORITHM" || uppercase_token == "ALGO" || uppercase_token == "REGION") return TOK::REG;
    if (uppercase_token == "HISTOLIST") return TOK::HISTOLIST;
    if (uppercase_token == "INFO") return TOK::ADLINFO;
    if (uppercase_token == "OBJ" || uppercase_token == "OBJECT") return TOK::OBJ;
    if (uppercase_token == "COMP" || uppercase_token == "COMPOSITE") return TOK::COMP;


    if (uppercase_token == "CMD" || uppercase_token == "CUT" || uppercase_token == "SELECT") return TOK::SELECT;
    if (uppercase_token == "REJECT") return TOK::REJEC;
    
    if (uppercase_token == "PARTICLE" || uppercase_token == "CANDIDATE") return TOK::PARTICLE_KEYWORD; // keyword that allows definitions to be of particles and not functions
    if (uppercase_token == "EXTERN" || uppercase_token  == "EXTERNAL") return TOK::EXTERNAL; // keyword that allows arbitrary external functions to be included
    if (uppercase_token == "ATTR" || uppercase_token  == "ATTRIBUTE") return TOK::ATTRIBUTE; // keyword that allows external particle attributes to be included
    if (uppercase_token == "CORRECTIONLIB") return TOK::CORRECTIONLIB;

    if (uppercase_token == "ON" || uppercase_token == "TRUE") return TOK::TRUE; 
    if (uppercase_token == "OFF" || uppercase_token == "FALSE") return TOK::FALSE; 
    if (uppercase_token == "NVARS") return TOK::NVARS;
    if (uppercase_token == "ERRORS") return TOK::ERRORS;
    if (uppercase_token == "TABLETYPE") return TOK::TABLETYPE;
    if (uppercase_token == "TAKE"  || uppercase_token == "USING") return TOK::TAKE;
    if (uppercase_token == "HISTO" || uppercase_token == "HIST") return TOK::HISTO;
    if (uppercase_token == "WEIGHT") return TOK::WEIGHT;
    if (uppercase_token == "TABLE") return TOK::TABLE;

    // Within-object block helper
    if (uppercase_token == "THIS") return TOK::THIS;

    
    if (uppercase_token == "NAMED") return TOK::NAMED;
    if (uppercase_token == "BIN") return TOK::BIN;
    if (uppercase_token == "BINS") return TOK::BINS;



    if (uppercase_token == "UNION") return TOK::UNION;
    if (uppercase_token == "ALIAS") return TOK::ALIAS;

    if (uppercase_token == "PHI") return TOK::PHI;//functions
    if (uppercase_token == "ETA") return TOK::ETA;
    if (uppercase_token == "CHARGE") return TOK::CHARGE;
    if (uppercase_token == "MASS") return TOK::MASS;

    if (uppercase_token == "DR" || uppercase_token == "DELTAR") return TOK::DR;
    if (uppercase_token == "DPHI" || uppercase_token == "DELTAPHI") return TOK::DPHI;
    if (uppercase_token == "DETA" || uppercase_token == "DELTAETA") return TOK::DETA;

    if (uppercase_token == "DISTINCT") return TOK::DISTINCT;

    if (uppercase_token == "DRHADAMARD" || uppercase_token == "DELTARHADAMARD") return TOK::DR_HADAMARD;
    if (uppercase_token == "DETAHADAMARD" || uppercase_token == "DELTAETAHADAMARD") return TOK::DETA_HADAMARD;
    if (uppercase_token == "DPHIHADAMARD" || uppercase_token == "DELTAPHIHADAMARD") return TOK::DPHI_HADAMARD;

    if (uppercase_token == "SIZE" || uppercase_token == "COUNT" || uppercase_token == "NUMOF") return TOK::NUMOF;//no arg funcs 

    // Comparison operators
    if (token == "=="|| uppercase_token == "EQ") return TOK::EQ;
    if (token == "!="|| uppercase_token == "NE") return TOK::NE;
    if (token == "<="|| uppercase_token == "LE") return TOK::LE;
    if (token == ">="|| uppercase_token == "GE") return TOK::GE;
    if (token == "<"|| uppercase_token == "LT") return TOK::LT;
    if (token == ">"|| uppercase_token == "GT") return TOK::GT;

    // Logical operators
    if (uppercase_token == "AND" || token == "&&") return TOK::AND;
    if (uppercase_token == "OR" || token == "||") return TOK::OR;
    if (uppercase_token == "NOT") return TOK::NOT;
    if (uppercase_token == "WITHIN" || uppercase_token == "IN") return TOK::WITHIN;
    if (uppercase_token == "OUTSIDE") return TOK::OUTSIDE;

    
    if (token == "-") return TOK::MINUS;
    if (token == "+") return TOK::PLUS;
    if (token == "*") return TOK::MULTIPLY;
    if (token == "/") return TOK::DIVIDE;

    if (token == "&") return TOK::AMPERSAND;
    if (token == "|") return TOK::PIPE;
    if (token == ":") return TOK::COLON;
    if (token == "^") return TOK::RAISED_TO_POWER;

    //  A dot, likely used to index an attribute e.g. particle.m
    if (token == ".") return TOK::DOT_INDEX;
    if (token == "->") return TOK::ARROW_INDEX;

    if (token == "(") return TOK::OPEN_PAREN;
    if (token == ")") return TOK::CLOSE_PAREN;
    if (token == "[") return TOK::OPEN_SQUARE_BRACE;
    if (token == "]") return TOK::CLOSE_SQUARE_BRACE;
    if (token == "{") return TOK::OPEN_CURLY_BRACE;
    if (token == "}") return TOK::CLOSE_CURLY_BRACE;
    if (token == "?") return TOK::QUESTION;
    if (token == "=") return TOK::ASSIGN;
    if (token == "_") return TOK::UNDERSCORE;

    if (uppercase_token == "DESCEND" || uppercase_token == "DESCENDING" || uppercase_token == "DECREASING") return TOK::DESCEND;
    if (uppercase_token == "ASCEND" || uppercase_token == "ASCENDING" || uppercase_token == "INCREASING") return TOK::ASCEND;

    // Purely mathematical functions
    if (uppercase_token == "TAN") return TOK::TAN;
    if (uppercase_token == "SIN") return TOK::SIN;
    if (uppercase_token == "COS") return TOK::COS;
    if (uppercase_token == "SINH") return TOK::SINH;
    if (uppercase_token == "COSH") return TOK::COSH;
    if (uppercase_token == "TANH") return TOK::TANH;
    if (uppercase_token == "EXP") return TOK::EXP;
    if (uppercase_token == "LOG") return TOK::LOG;
    if (uppercase_token == "ABS") return TOK::ABS;
    if (uppercase_token == "SQRT") return TOK::SQRT;

    // Functions on variable lists
    if (uppercase_token == "AVE") return TOK::AVE;
    if (uppercase_token == "SUM") return TOK::SUM;
    if (uppercase_token == "ADD") return TOK::ADD;
    if (uppercase_token == "ANY" || uppercase_token == "ANYOF") return TOK::ANYOF;
    if (uppercase_token == "ALLOF" || uppercase_token ==  "ALL") return TOK::ALLOF;

     
    if (uppercase_token == "SORT") return TOK::SORT;

    // Combinators - ways to combine different lists of particles
    if (uppercase_token == "COMB" || uppercase_token=="CARTESIAN") return TOK::COMB;
    if (uppercase_token == "DISJOINT") return TOK::DISJOINT;
    if (uppercase_token == "DIRECT") return TOK::DIRECT;

    if (uppercase_token == "MIN") return TOK::MIN;
    if (uppercase_token == "MAX") return TOK::MAX;

    if (token == ",") return TOK::COMMA;

    // these letters are keywords which indicate a corresponding function 
    if (uppercase_token == "Q") return TOK::LETTER_Q; // charge
    if (uppercase_token == "E") return TOK::LETTER_E; // energy
    if (uppercase_token == "P") return TOK::LETTER_P; // momentum
    if (uppercase_token == "M") return TOK::LETTER_M; // mass

    // We have as of yet failed to lex this token, which implies it isn't something that exact matching has worked with
    // We check if it is some kind of number
    if (std::regex_match(token, reg_int)) return TOK::INTEGER;
    if (std::regex_match(token, reg_decimal)) return TOK::DECIMAL;
    if (std::regex_match(token, reg_scientific)) return TOK::SCIENTIFIC;

    // It definintely isn't a number - maybe it's in a variable name format
    if (std::regex_match(token, reg_varname)) return TOK::VARNAME;

    // Not that either - maybe it's just a valid string
    if (std::regex_match(token, reg_string)) return TOK::STRING;

    // Not any sort of valid object, so far as this can tell. Assume this is invalid text, and end our tokenization.
    return TOK::LEXER_ERROR;
}

std::string token_type_to_string(Token_type type) {
    switch(type) {
        case TOK::LEXER_ERROR: return "LEXER_ERROR";
        case TOK::COMMENT: return "LEXER_COMMENT";
        case TOK::SPACE: return "LEXER_SPACE";
        case TOK::NEWLINE: return "LEXER_NEWLINE";
        case TOK::END_OF_FILE: return "LEXER_END_OF_FILE";

        case TOK::DECIMAL: return "DECIMAL";
        case TOK::SCIENTIFIC: return "SCIENTIFIC";

        case TOK::STRING: return "STRING";
        case TOK::INTEGER: return "INTEGER";
        case TOK::VARNAME: return "VARNAME";

        case TOK::DEF: return "DEF";
        case TOK::SELECT: return "SELECT";
        case TOK::REJEC: return "REJEC";
        case TOK::OBJ: return "OBJ";
        case TOK::REG: return "REG";

        case TOK::HISTOLIST: return "HISTOLIST";

        case TOK::ADLINFO: return "ADLINFO";

        case TOK::PARTICLE_KEYWORD: return "PARTICLE_KEYWORD";
        case TOK::EXTERNAL: return "EXTERNAL";
        case TOK::ATTRIBUTE: return "ATTRIBUTE";
        case TOK::CORRECTIONLIB: return "CORRECTIONLIB";

        case TOK::TRUE: return "TRUE";
        case TOK::FALSE: return "FALSE";
        case TOK::NVARS: return "NVARS";
        case TOK::ERRORS: return "ERRORS";
        case TOK::TABLETYPE: return "TABLETYPE";
        case TOK::TAKE: return "TAKE";
        case TOK::HISTO: return "HISTO";
        case TOK::WEIGHT: return "WEIGHT";
        case TOK::TABLE: return "TABLE";

        case TOK::NAMED: return "NAMED";
        case TOK::BIN: return "BIN";
        case TOK::BINS: return "BINS";

        case TOK::UNION: return "UNION";
        case TOK::ALIAS: return "ALIAS";


        case TOK::PHI: return "PHI";
        case TOK::ETA: return "ETA";

        case TOK::CHARGE: return "CHARGE";
        case TOK::MASS: return "MASS";

        case TOK::PT: return "PT";

        case TOK::DR: return "DR";
        case TOK::DPHI: return "DPHI";
        case TOK::DETA: return "DETA";

        case TOK::DR_HADAMARD: return "DR_HADAMARD";
        case TOK::DPHI_HADAMARD: return "DPHI_HADAMARD";
        case TOK::DETA_HADAMARD: return "DETA_HADAMARD";

        case TOK::NUMOF: return "NUMOF";

        case TOK::ANYOF: return "ANYOF";
        case TOK::ALLOF: return "ALLOF";
        case TOK::THIS: return "THIS";


        case TOK::EQ: return "EQ";
        case TOK::NE: return "NE";
        case TOK::LE: return "LE";
        case TOK::GE: return "GE";
        case TOK::LT: return "LT";
        case TOK::GT: return "GT";
        case TOK::AND: return "AND";
        case TOK::OR: return "OR";
        case TOK::NOT: return "NOT";
        case TOK::WITHIN: return "WITHIN";
        case TOK::OUTSIDE: return "OUTSIDE";

        case TOK::MINUS: return "MINUS";
        case TOK::PLUS: return "PLUS";
        case TOK::MULTIPLY: return "MULTIPLY";
        case TOK::DIVIDE: return "DIVIDE";

        case TOK::DOT_INDEX: return "DOT_INDEX";
        case TOK::ARROW_INDEX: return "ARROW_INDEX";

        case TOK::AMPERSAND: return "AMPERSAND";
        case TOK::PIPE: return "PIPE";
        case TOK::COLON: return "COLON";
        case TOK::RAISED_TO_POWER: return "RAISED_TO_POWER";

        case TOK::OPEN_PAREN: return "OPEN_PAREN";
        case TOK::CLOSE_PAREN: return "CLOSE_PAREN";
        case TOK::OPEN_SQUARE_BRACE: return "OPEN_SQUARE_BRACE";
        case TOK::CLOSE_SQUARE_BRACE: return "CLOSE_SQUARE_BRACE";
        case TOK::OPEN_CURLY_BRACE: return "OPEN_CURLY_BRACE";
        case TOK::CLOSE_CURLY_BRACE: return "CLOSE_CURLY_BRACE";
        case TOK::QUESTION: return "QUESTION";
        case TOK::ASSIGN: return "ASSIGN";

        case TOK::AVE: return "AVE";
        case TOK::SUM: return "SUM";
        case TOK::ADD: return "ADD";
        case TOK::ASCEND: return "ASCEND";
        case TOK::DESCEND: return "DESCEND";
        case TOK::TAN: return "TAN";
        case TOK::SIN: return "SIN";
        case TOK::COS: return "COS";
        case TOK::SINH: return "SINH";
        case TOK::COSH: return "COSH";
        case TOK::TANH: return "TANH";
        case TOK::EXP: return "EXP";
        case TOK::LOG: return "LOG";
        case TOK::ABS: return "ABS";
        case TOK::SQRT: return "SQRT";
        case TOK::SORT: return "SORT";
        case TOK::COMB: return "COMB";
        case TOK::DISJOINT: return "DISJOINT";
        case TOK::DIRECT: return "DIRECT";
        case TOK::MIN: return "MIN";
        case TOK::MAX: return "MAX";

        case TOK::COMMA: return "COMMA";
        case TOK::UNDERSCORE: return "UNDERSCORE";

        case TOK::LETTER_M: return "LETTER_M";
        case TOK::LETTER_Q: return "LETTER_Q";
        case TOK::LETTER_P: return "LETTER_P";
        case TOK::LETTER_E: return "LETTER_E";
        case TOK::COMP: return "COMP";
        case TOK::DISTINCT: return "DISTINCT";
          break;
        }

    assert(false);
    return "";

}

std::string Token::get_token_type_as_string() {
    return token_type_to_string(type);
}

void Lexer::lex_token(std::string &token, int &line_number, int &column_number) {

    // Do not lex an empty token
    if (token.size() == 0) return;

    auto tok = std::make_shared<Token>(identify_token(token));
    tok->set_data(line_number, column_number, token);

    if (tok->get_token_type() == TOK::LEXER_ERROR) {
        raise_lexing_exception(tok);
    }

    tokens.push_back(tok);

    // move the lexer along
    column_number += token.length();
    token.clear();
}

void Lexer::read_lines(std::string filename, bool is_verbose) {

    verbose = is_verbose;

    std::ifstream read_file(filename);

    std::string content;

    int line = 0;
    while (std::getline(read_file, content)) {
        line++;

        std::string running_token;

        bool is_commented_out = false;
        bool is_quoted_out = false;

        int column = 1;
        for (auto it = content.begin(); it != content.end(); ++it) {
            char current_char = *it;

            // If we have a comment start symbol, this entire line from here on must be one comment token
            if (current_char == '#' && !is_commented_out) {
                lex_token(running_token, line, column);
                is_commented_out = true;
                running_token += current_char;
                continue;

            } else if (is_commented_out) {
                // If this line is commented out, this is all just one comment
                running_token += current_char;
                continue;
            }

            // if we have a quote character, and we are in a quote scope, then we have ceased to be so.
            if (is_quoted_out && current_char == '"') {
                running_token += current_char;
                lex_token(running_token, line, column);
                is_quoted_out = false;
                continue;
            } else if (current_char == '"') {
                // We are now beginning a quoted scope, within which we want to always add characters to this same token
                lex_token(running_token, line, column);
                is_quoted_out = true;
                running_token += current_char;
                continue;
            } else if(is_quoted_out) {
                // If we are in a quote, we simply add until the quote is over
                running_token += current_char;
                continue;
            }

            bool delimiter = is_delimiter(current_char);
            bool symbol = is_symbol(current_char);

            if (current_char == '(' || current_char == ')' || current_char == ','|| current_char == '{' || current_char == '}' || current_char == '[' || current_char == ']') {
                // Exception: commas and brackets of all kinds must be allowed to be stacked adjacent to whatever, and that must unequivocably be its own token - there is never a situation in which this should not be the case
                lex_token(running_token, line, column);
                running_token += current_char;
                lex_token(running_token, line, column);
                continue;
            }

            // if the running string is empty, we add this character to the running string and await further input
            if (running_token.size() <= 0) {
                running_token += current_char;
                continue;
            }

            char prev_char = running_token.back();

            bool previous_delimiter = is_delimiter(prev_char);
            bool previous_symbol = is_symbol(prev_char);

            char next_char = std::next(it) != content.end() ? *std::next(it) : ' ';

            // if the running token is of the same type as the current char, then we add it to the string and keep going
            if (delimiter != previous_delimiter || symbol != previous_symbol) {

                bool was_negative_sign_for_number = prev_char == '-' && current_char >= '0' && current_char <= '9';
                bool was_decimal_point_for_number = prev_char == '.' && current_char >= '0' && current_char <= '9';
                bool is_decimal_point_in_number = current_char == '.' && next_char <= '9' && next_char >= '0';

                if (!was_negative_sign_for_number && !was_decimal_point_for_number && !is_decimal_point_in_number) {
                    // Otherwise, we do not have compatible symbols, so this clearly must be the end of a running token
                    lex_token(running_token, line, column);
                }
            } 
            running_token = running_token + current_char;
        }
        // The line is over, we lex the remainder
        lex_token(running_token, line, column);

        auto endline = std::make_shared<Token>(TOK::NEWLINE);
        endline->set_data(line, column, "\n");
        tokens.push_back(endline);

        // Return to the start of the next line
    }

    read_file.close();
}

void Lexer::print() {
    // erase_whitespace();

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        std::shared_ptr tok = *it;
        if (tok->get_token_type() == TOK::NEWLINE) {
            std::cout << std::endl;
            continue;
        }
        std::cout << token_type_to_string(tok->get_token_type()) << ": " << tok->get_lexeme() << "," << " ";
    }
}

void Lexer::erase_whitespace() {

    non_whitespace_tokens.clear();

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {

        auto tok = *it;
        switch (tok->get_token_type()) {
            // If this is a whitespace or comment, don't even consider it in parsing
            case TOK::LEXER_ERROR: case TOK::NEWLINE: case TOK::COMMENT: case TOK::SPACE:
                break;
            default:
                non_whitespace_tokens.push_back(tok);
                break;
        }
    }

}

void Lexer::reset() {
    erase_whitespace();
    current_token = non_whitespace_tokens.begin();
}

std::shared_ptr<Token> Lexer::next() {
    if (current_token == non_whitespace_tokens.end()) return 
    std::make_shared<Token>(TOK::END_OF_FILE);

    auto tok = *current_token;
    ++current_token;

    return tok;
}

void Lexer::expect_and_consume(Token_type type, std::string error) {
    auto tok = next();
    if (tok->get_token_type() != type) {
        if (error == "") {
            std::stringstream error_ss;
            error_ss << "Unexpected token, expected a token of type " << token_type_to_string(type) << ", got token of type " << token_type_to_string(tok->get_token_type());
            raise_parsing_exception(error_ss.str(), tok);
        } else {
            raise_parsing_exception(error, tok);
        }
    }
}


std::shared_ptr<Token> Lexer::peek(int lookahead) {
    auto ahead_tok_it = current_token;

    while (lookahead > 0) {
        ++ahead_tok_it;
        lookahead--;
        if (ahead_tok_it == non_whitespace_tokens.end()) return 
        std::make_shared<Token>(TOK::END_OF_FILE);
    }

    if (ahead_tok_it == non_whitespace_tokens.end()) return 
    std::make_shared<Token>(TOK::END_OF_FILE);

    return *ahead_tok_it;
}
