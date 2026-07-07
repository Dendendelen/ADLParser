#include "lexer.hpp"

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
    if (token.front() == '#') return TOK_COMMENT;
    if (std::regex_match(token, std::regex("\\s+"))) return TOK_SPACE;

    // Top level ADL syntax
    if (uppercase_token == "DEF" || uppercase_token == "DEFINE") return TOK_DEF;
    if (uppercase_token == "ALGORITHM" || uppercase_token == "ALGO" || uppercase_token == "REGION") return TOK_REG;
    if (uppercase_token == "HISTOLIST") return TOK_HISTOLIST;
    if (uppercase_token == "INFO") return TOK_ADLINFO;
    if (uppercase_token == "OBJ" || uppercase_token == "OBJECT") return TOK_OBJ;
    if (uppercase_token == "COMP" || uppercase_token == "COMPOSITE") return TOK_COMP;


    if (uppercase_token == "CMD" || uppercase_token == "CUT" || uppercase_token == "SELECT") return TOK_SELECT;
    if (uppercase_token == "REJECT") return TOK_REJEC;
    
    if (uppercase_token == "PARTICLE" || uppercase_token == "CANDIDATE") return TOK_PARTICLE_KEYWORD; // keyword that allows definitions to be of particles and not functions
    if (uppercase_token == "EXTERN" || uppercase_token  == "EXTERNAL") return TOK_EXTERNAL; // keyword that allows arbitrary external functions to be included
    if (uppercase_token == "ATTR" || uppercase_token  == "ATTRIBUTE") return TOK_ATTRIBUTE; // keyword that allows external particle attributes to be included
    if (uppercase_token == "CORRECTIONLIB") return TOK_CORRECTIONLIB;

    if (token == "systematic") return SYSTEMATIC;

    if (uppercase_token == "ON" || uppercase_token == "TRUE") return TOK_TRUE; 
    if (uppercase_token == "OFF" || uppercase_token == "FALSE") return TOK_FALSE; 
    if (uppercase_token == "NVARS") return TOK_NVARS;
    if (uppercase_token == "ERRORS") return TOK_ERRORS;
    if (uppercase_token == "TABLETYPE") return TOK_TABLETYPE;
    if (uppercase_token == "TAKE"  || uppercase_token == "USING") return TOK_TAKE;
    if (uppercase_token == "HISTO" || uppercase_token == "HIST") return TOK_HISTO;
    if (uppercase_token == "WEIGHT") return TOK_WEIGHT;
    if (uppercase_token == "TABLE") return TOK_TABLE;

    // Within-object block helper
    if (uppercase_token == "THIS") return TOK_THIS;

    

    if (uppercase_token == "BIN") return TOK_BIN;
    if (uppercase_token == "BINS") return TOK_BINS;



    if (uppercase_token == "UNION") return TOK_UNION;
    if (uppercase_token == "ALIAS") return TOK_ALIAS;

    if (uppercase_token == "PHI") return TOK_PHI;//functions
    if (uppercase_token == "ETA") return TOK_ETA;
    if (uppercase_token == "CHARGE") return TOK_CHARGE;
    if (uppercase_token == "MASS") return TOK_MASS;

    if (uppercase_token == "DR" || uppercase_token == "DELTAR") return TOK_DR;
    if (uppercase_token == "DPHI" || uppercase_token == "DELTAPHI") return TOK_DPHI;
    if (uppercase_token == "DETA" || uppercase_token == "DELTAETA") return TOK_DETA;

    if (uppercase_token == "DISTINCT") return TOK_DISTINCT;

    if (uppercase_token == "DRHADAMARD" || uppercase_token == "DELTARHADAMARD") return TOK_DR_HADAMARD;
    if (uppercase_token == "DETAHADAMARD" || uppercase_token == "DELTAETAHADAMARD") return TOK_DETA_HADAMARD;
    if (uppercase_token == "DPHIHADAMARD" || uppercase_token == "DELTAPHIHADAMARD") return TOK_DPHI_HADAMARD;

    if (uppercase_token == "SIZE" || uppercase_token == "COUNT" || uppercase_token == "NUMOF") return TOK_NUMOF;//no arg funcs 

    // Global analysis tokens
    if (uppercase_token == "ALL") return TOK_ALL;
    if (uppercase_token == "NONE") return TOK_NONE;

    // Comparison operators
    if (token == "=="|| uppercase_token == "EQ") return TOK_EQ;
    if (token == "!="|| uppercase_token == "NE") return TOK_NE;
    if (token == "<="|| uppercase_token == "LE") return TOK_LE;
    if (token == ">="|| uppercase_token == "GE") return TOK_GE;
    if (token == "<"|| uppercase_token == "LT") return TOK_LT;
    if (token == ">"|| uppercase_token == "GT") return TOK_GT;

    // Logical operators
    if (uppercase_token == "AND" || token == "&&") return TOK_AND;
    if (uppercase_token == "OR" || token == "||") return TOK_OR;
    if (uppercase_token == "NOT") return TOK_NOT;
    if (uppercase_token == "WITHIN" || uppercase_token == "IN") return TOK_WITHIN;
    if (uppercase_token == "OUTSIDE") return TOK_OUTSIDE;

    
    if (token == "-") return TOK_MINUS;
    if (token == "+") return TOK_PLUS;
    if (token == "*") return TOK_MULTIPLY;
    if (token == "/") return TOK_DIVIDE;

    if (token == "&") return TOK_AMPERSAND;
    if (token == "|") return TOK_PIPE;
    if (token == ":") return TOK_COLON;
    if (token == "^") return TOK_RAISED_TO_POWER;

    //  A dot, likely used to index an attribute e.g. particle.m
    if (token == ".") return TOK_DOT_INDEX;
    if (token == "->") return TOK_ARROW_INDEX;

    if (token == "(") return TOK_OPEN_PAREN;
    if (token == ")") return TOK_CLOSE_PAREN;
    if (token == "[") return TOK_OPEN_SQUARE_BRACE;
    if (token == "]") return TOK_CLOSE_SQUARE_BRACE;
    if (token == "{") return TOK_OPEN_CURLY_BRACE;
    if (token == "}") return TOK_CLOSE_CURLY_BRACE;
    if (token == "?") return TOK_QUESTION;
    if (token == "=") return TOK_ASSIGN;
    if (token == "_") return TOK_UNDERSCORE;

    if (uppercase_token == "DESCEND" || uppercase_token == "DESCENDING" || uppercase_token == "DECREASING") return TOK_DESCEND;
    if (uppercase_token == "ASCEND" || uppercase_token == "ASCENDING" || uppercase_token == "INCREASING") return TOK_ASCEND;

    // Purely mathematical functions
    if (uppercase_token == "TAN") return TOK_TAN;
    if (uppercase_token == "SIN") return TOK_SIN;
    if (uppercase_token == "COS") return TOK_COS;
    if (uppercase_token == "SINH") return TOK_SINH;
    if (uppercase_token == "COSH") return TOK_COSH;
    if (uppercase_token == "TANH") return TOK_TANH;
    if (uppercase_token == "EXP") return TOK_EXP;
    if (uppercase_token == "LOG") return TOK_LOG;
    if (uppercase_token == "ABS") return TOK_ABS;
    if (uppercase_token == "SQRT") return TOK_SQRT;

    // Functions on variable lists
    if (uppercase_token == "AVE") return TOK_AVE;
    if (uppercase_token == "SUM") return TOK_SUM;
    if (uppercase_token == "ADD") return ADD;
    if (uppercase_token == "ANY" || uppercase_token == "ANYOF") return TOK_ANYOF;
    if (uppercase_token == "ALLOF" || uppercase_token ==  "ALL") return TOK_ALLOF;

     
    if (uppercase_token == "SORT") return TOK_SORT;

    // Combinators - ways to combine different lists of particles
    if (uppercase_token == "COMB" || uppercase_token=="CARTESIAN") return TOK_COMB;
    if (uppercase_token == "DISJOINT") return TOK_DISJOINT;
    if (uppercase_token == "DIRECT") return TOK_DIRECT;

    if (uppercase_token == "MIN") return TOK_MIN;
    if (uppercase_token == "MAX") return TOK_MAX;

    if (token == "+-"|| token == "-+") return PM;

    if (token == ",") return TOK_COMMA;

    // these letters are keywords which indicate a corresponding function 
    if (uppercase_token == "Q") return TOK_LETTER_Q; // charge
    if (uppercase_token == "E") return TOK_LETTER_E; // energy
    if (uppercase_token == "P") return TOK_LETTER_P; // momentum
    if (uppercase_token == "M") return TOK_LETTER_M; // mass

    // We have as of yet failed to lex this token, which implies it isn't something that exact matching has worked with
    // We check if it is some kind of number
    if (std::regex_match(token, reg_int)) return TOK_INTEGER;
    if (std::regex_match(token, reg_decimal)) return TOK_DECIMAL;
    if (std::regex_match(token, reg_scientific)) return TOK_SCIENTIFIC;

    // It definintely isn't a number - maybe it's in a variable name format
    if (std::regex_match(token, reg_varname)) return TOK_VARNAME;

    // Not that either - maybe it's just a valid string
    if (std::regex_match(token, reg_string)) return TOK_STRING;

    // Not any sort of valid object, so far as this can tell. Assume this is invalid text, and end our tokenization.
    return TOK_LEXER_ERROR;
}

std::string token_type_to_string(Token_type type) {
    switch(type) {
        case TOK_LEXER_ERROR: return "LEXER_ERROR";
        case TOK_COMMENT: return "LEXER_COMMENT";
        case TOK_SPACE: return "LEXER_SPACE";
        case TOK_NEWLINE: return "LEXER_NEWLINE";
        case TOK_END_OF_FILE: return "LEXER_END_OF_FILE";

        case TOK_DECIMAL: return "DECIMAL";
        case TOK_SCIENTIFIC: return "SCIENTIFIC";

        case TOK_STRING: return "STRING";
        case TOK_INTEGER: return "INTEGER";
        case TOK_VARNAME: return "VARNAME";

        case TOK_DEF: return "DEF";
        case TOK_SELECT: return "SELECT";
        case TOK_REJEC: return "REJEC";
        case TOK_OBJ: return "OBJ";
        case TOK_REG: return "REG";

        case TOK_HISTOLIST: return "HISTOLIST";

        case TOK_ADLINFO: return "ADLINFO";

        case TOK_PARTICLE_KEYWORD: return "PARTICLE_KEYWORD";
        case TOK_EXTERNAL: return "EXTERNAL";
        case TOK_ATTRIBUTE: return "ATTRIBUTE";
        case TOK_CORRECTIONLIB: return "CORRECTIONLIB";

        case SYSTEMATIC: return "SYSTEMATIC";

        case TOK_TRUE: return "TRUE";
        case TOK_FALSE: return "FALSE";
        case TOK_NVARS: return "NVARS";
        case TOK_ERRORS: return "ERRORS";
        case TOK_TABLETYPE: return "TABLETYPE";
        case TOK_TAKE: return "TAKE";
        case TOK_HISTO: return "HISTO";
        case TOK_WEIGHT: return "WEIGHT";
        case TOK_TABLE: return "TABLE";


        case TOK_BIN: return "BIN";
        case TOK_BINS: return "BINS";

        case TOK_UNION: return "UNION";
        case TOK_ALIAS: return "ALIAS";


        case TOK_PHI: return "PHI";
        case TOK_ETA: return "ETA";

        case TOK_CHARGE: return "CHARGE";
        case TOK_MASS: return "MASS";

        case TOK_PT: return "PT";

        case TOK_DR: return "DR";
        case TOK_DPHI: return "DPHI";
        case TOK_DETA: return "DETA";

        case TOK_DR_HADAMARD: return "DR_HADAMARD";
        case TOK_DPHI_HADAMARD: return "DPHI_HADAMARD";
        case TOK_DETA_HADAMARD: return "DETA_HADAMARD";

        case TOK_NUMOF: return "NUMOF";

        case TOK_ANYOF: return "ANYOF";
        case TOK_ALLOF: return "ALLOF";
        case TOK_ALL: return "ALL";
        case TOK_NONE: return "NONE";
        case TOK_THIS: return "THIS";


        case TOK_EQ: return "EQ";
        case TOK_NE: return "NE";
        case TOK_LE: return "LE";
        case TOK_GE: return "GE";
        case TOK_LT: return "LT";
        case TOK_GT: return "GT";
        case TOK_AND: return "AND";
        case TOK_OR: return "OR";
        case TOK_NOT: return "NOT";
        case TOK_WITHIN: return "WITHIN";
        case TOK_OUTSIDE: return "OUTSIDE";

        case TOK_MINUS: return "MINUS";
        case TOK_PLUS: return "PLUS";
        case TOK_MULTIPLY: return "MULTIPLY";
        case TOK_DIVIDE: return "DIVIDE";

        case TOK_DOT_INDEX: return "DOT_INDEX";
        case TOK_ARROW_INDEX: return "ARROW_INDEX";

        case TOK_AMPERSAND: return "AMPERSAND";
        case TOK_PIPE: return "PIPE";
        case TOK_COLON: return "COLON";
        case TOK_RAISED_TO_POWER: return "RAISED_TO_POWER";

        case TOK_OPEN_PAREN: return "OPEN_PAREN";
        case TOK_CLOSE_PAREN: return "CLOSE_PAREN";
        case TOK_OPEN_SQUARE_BRACE: return "OPEN_SQUARE_BRACE";
        case TOK_CLOSE_SQUARE_BRACE: return "CLOSE_SQUARE_BRACE";
        case TOK_OPEN_CURLY_BRACE: return "OPEN_CURLY_BRACE";
        case TOK_CLOSE_CURLY_BRACE: return "CLOSE_CURLY_BRACE";
        case TOK_QUESTION: return "QUESTION";
        case TOK_ASSIGN: return "ASSIGN";

        case TOK_AVE: return "AVE";
        case TOK_SUM: return "SUM";
        case ADD: return "ADD";
        case TOK_ASCEND: return "ASCEND";
        case TOK_DESCEND: return "DESCEND";
        case TOK_TAN: return "TAN";
        case TOK_SIN: return "SIN";
        case TOK_COS: return "COS";
        case TOK_SINH: return "SINH";
        case TOK_COSH: return "COSH";
        case TOK_TANH: return "TANH";
        case TOK_EXP: return "EXP";
        case TOK_LOG: return "LOG";
        case TOK_ABS: return "ABS";
        case TOK_SQRT: return "SQRT";
        case TOK_SORT: return "SORT";
        case TOK_COMB: return "COMB";
        case TOK_DISJOINT: return "DISJOINT";
        case TOK_DIRECT: return "DIRECT";
        case TOK_MIN: return "MIN";
        case TOK_MAX: return "MAX";
        case PM: return "PM";

        case TOK_COMMA: return "COMMA";
        case TOK_UNDERSCORE: return "UNDERSCORE";

        case TOK_LETTER_M: return "LETTER_M";
        case TOK_LETTER_Q: return "LETTER_Q";
        case TOK_LETTER_P: return "LETTER_P";
        case TOK_LETTER_E: return "LETTER_E";
        case TOK_COMP: return "COMP";
        case TOK_DISTINCT: return "DISTINCT";
          break;
        }
}

std::string Token::get_token_type_as_string() {
    return token_type_to_string(type);
}

void Lexer::lex_token(std::string &token, int &line_number, int &column_number) {

    // Do not lex an empty token
    if (token.size() == 0) return;

    auto tok = std::make_shared<Token>(identify_token(token));
    tok->set_data(line_number, column_number, token);

    if (tok->get_token_type() == TOK_LEXER_ERROR) {
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

        auto endline = std::make_shared<Token>(TOK_NEWLINE);
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
        if (tok->get_token_type() == TOK_NEWLINE) {
            std::cout << std::endl;
            continue;
        }
        std::cout << tok->get_token_type() << ": " << tok->get_lexeme() << "," << " ";
    }
}

void Lexer::erase_whitespace() {

    non_whitespace_tokens.clear();

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {

        auto tok = *it;
        switch (tok->get_token_type()) {
            // If this is a whitespace or comment, don't even consider it in parsing
            case TOK_LEXER_ERROR: case TOK_NEWLINE: case TOK_COMMENT: case TOK_SPACE:
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
    std::make_shared<Token>(TOK_END_OF_FILE);

    auto tok = *current_token;
    ++current_token;

    return tok;
}

void Lexer::expect_and_consume(Token_type type, std::string error) {
    auto tok = next();
    if (tok->get_token_type() != type) {
        raise_parsing_exception(error, tok);
    }
}

void Lexer::expect_and_consume(Token_type type) {
    auto tok = next();
    if (tok->get_token_type() != type) {
        std::stringstream error_ss;
        error_ss << "Unexpected token, expected a token of type " << token_type_to_string(type) << ", got token of type " << token_type_to_string(tok->get_token_type());
        raise_parsing_exception(error_ss.str(), tok);
    }
    
}


std::shared_ptr<Token> Lexer::peek(int lookahead) {
    auto ahead_tok_it = current_token;

    while (lookahead > 0) {
        ++ahead_tok_it;
        lookahead--;
        if (ahead_tok_it == non_whitespace_tokens.end()) return 
        std::make_shared<Token>(TOK_END_OF_FILE);
    }

    if (ahead_tok_it == non_whitespace_tokens.end()) return 
    std::make_shared<Token>(TOK_END_OF_FILE);

    return *ahead_tok_it;
}
