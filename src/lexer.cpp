#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <regex>
#include <vector>

#include "exceptions.hpp"

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
Token_type Token::get_token() {
    return type;
}
std::string Token::get_lexeme() {
    return lexeme;
}

// Is the character an inherently delimiting symbol?
bool Lexer::is_symbol (char c) {
    if (c == '=' || c == '!' || c == '!' || c == '~' || c == '<' || c == '>' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == '&' || c == '|' || c == '+' || c == '-' || c == '*' || c == '/' || c == '?' || c == '^' || c == ',') return true;
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
    reg_scientific = std::regex("-{0,1}[0-9]*\\.{0,1}[0-9]*[Ee][-+]{0,1}[0-9]+");
    reg_varname = std::regex("[A-Za-z][A-Za-z0-9_]*");
    reg_string= std::regex("\"[^\"]*\"");

    reg_whitespace = std::regex("\\s");

}

Token_type Lexer::identify_token(std::string &token) {
    std::string uppercase_token = convert_to_uppercase(token);

    if (verbose) std::cout << "Lexing " << token << std::endl;
    
    // If we start with a hash, then this is instantly a comment
    if (token.front() == '#') return LEXER_COMMENT;
    if (std::regex_match(token, std::regex("\\s+"))) return LEXER_SPACE;

    // Top level ADL syntax
    if (uppercase_token == "DEF" || uppercase_token == "DEFINE") return DEF;
    if (uppercase_token == "ALGORITHM" || uppercase_token == "ALGO" || uppercase_token == "REGION") return ALGO;
    if (uppercase_token == "HISTOLIST") return HISTOLIST;
    if (uppercase_token == "INFO") return ADLINFO;
    if (uppercase_token == "OBJ" || uppercase_token == "OBJECT") return OBJ;


    if (uppercase_token == "CMD" || uppercase_token == "CUT" || uppercase_token == "SELECT") return SELECT;
    if (uppercase_token == "REJECT") return REJEC;


    if (token == "TRGe") return TRGE;
    if (token == "TRGm") return TRGM;

    // Header info tags
    if (token == "experiment") return PAP_EXPERIMENT;
    if (token == "id") return PAP_ID;
    if (uppercase_token == "TITLE") return PAP_TITLE;
    if (token == "publication") return PAP_PUBLICATION;
    if (token == "sqrtS") return PAP_SQRTS;
    if (token == "lumi" ) return PAP_LUMI;
    if (token == "arXiv") return PAP_ARXIV;
    if (token == "hepdata") return PAP_HEPDATA;
    if (token == "doi"   ) return PAP_DOI;

    if (token == "counts" ) return COUNTS;
    if (token == "countsformat") return COUNTSFORMAT;
    if (token == "stat") return ERR_STAT;
    if (token == "syst") return ERR_SYST;
    if (token == "process") return PROCESS;

    
    if (uppercase_token == "PARTICLE") return PARTICLE_KEYWORD; // keyword that allows definitions to be of particles and not functions
    if (uppercase_token == "EXTERN" || uppercase_token  == "EXTERNAL") return EXTERNAL; // keyword that allows arbitrary external functions to be included

    if (token == "systematic") return SYSTEMATIC;
    if (token == "ttree") return SYST_TTREE;
    if (token == "weightMc") return SYST_WEIGHT_MC;
    if (token == "weightPileup") return SYST_WEIGHT_PILEUP;
    if (token == "weightJvt") return SYST_WEIGHT_JVT;
    if (token == "weightLeptonSF") return SYST_WEIGHT_LEPTON_SF;
    if (token == "weightBTagSF") return SYST_WEIGHT_BTAG_SF;
    if (token == "RunYear") return RUNYEAR;
    if (token == "mcChannelNumber") return MC_CHANNEL_NUMBER;
    if (uppercase_token == "EVENTNO") return EVENT_NO;
    if (uppercase_token == "RUNNO") return RUN_NO;
    if (uppercase_token == "LBNO") return LB_NO;
    if (token == "OME") return OME;

    if (uppercase_token == "USE") return USE;
    if (uppercase_token == "PRINT") return PRINT;
    if (uppercase_token == "IF") return IF;
    if (uppercase_token == "THEN") return THEN;
    if (uppercase_token == "ELSE") return ELSE;
    if (uppercase_token == "DO") return DO;
    if (uppercase_token == "ON" || uppercase_token == "TRUE") return TRUE; 
    if (uppercase_token == "OFF" || uppercase_token == "FALSE") return FALSE; 
    if (uppercase_token == "NVARS") return NVARS;
    if (uppercase_token == "ERRORS") return ERRORS;
    if (uppercase_token == "TABLETYPE") return TABLETYPE;
    if (uppercase_token == "TAKE"  || uppercase_token == "USING") return TAKE;
    if (uppercase_token == "HISTO") return HISTO;
    if (uppercase_token == "WEIGHT") return WEIGHT;
    if (uppercase_token == "TABLE") return TABLE;
    if (uppercase_token == "SKIPHISTOS") return SKIP_HISTO;
    if (uppercase_token == "SKIPEFS") return SKIP_EFFS;
    if (uppercase_token == "GEN") return GEN;
    if (uppercase_token == "ELE"|| uppercase_token == "ELECTRON"|| token == "electron") return ELECTRON;//particles
    if (uppercase_token == "MUO" || uppercase_token == "MUON"| token == "muon") return MUON;
    if (uppercase_token == "TAU") return TAU;
    if (uppercase_token == "TRK") return TRACK;
    if (uppercase_token == "PHO" || uppercase_token == "PHOTON") return PHOTON;
    if (uppercase_token == "JET") return JET;
    if (uppercase_token == "FJET"|| uppercase_token == "FATJET") return FJET;
    if (uppercase_token == "QGJET") return QGJET;
    if (uppercase_token == "BIN") return BIN;
    if (uppercase_token == "BINS") return BINS;
    if (token == "daughters" || token == "constituents") return CONSTITUENTS;
    if (token == "NUMET") return NUMET;
    if (token == "METLV") return METLV;
    if (token == "MET") return MET;

    if (token == "LEP") return LEPTON;
    if (uppercase_token == "HLT") return HLT;
    if (token == "BJET") return BJET;
    if (uppercase_token == "INDEX") return IDX;

    if (uppercase_token == "METSIG") return METSIGNIF;
    if (token == "applyHM") return APPLY_HM;
    if (token == "applyPTF" || token == "scalePT" ) return APPLY_PTF;
    if (token == "applyEF" || token == "scaleE" ) return APPLY_EF;
    if (token == "genPartIdx") return GENPART_IDX;

    if (uppercase_token == "UNION") return UNION;
    if (uppercase_token == "ALIAS") return ALIAS;

    if (uppercase_token == "BTAG") return IS_BTAG;
    if (uppercase_token == "CTAG") return IS_CTAG;
    if (uppercase_token == "TAUTAG") return IS_TAUTAG;

    if (uppercase_token == "PDGID" || uppercase_token == "PDG_ID") return PDG_ID;
    if (uppercase_token == "JETID") return JET_ID;
    if (uppercase_token == "STATUSFLAGS") return STATUS_FLAGS;
    if (uppercase_token == "FLAVOR" | uppercase_token == "BTAGGER") return FLAVOR;
    if (uppercase_token == "PTCONE") return PTCONE;
    if (uppercase_token == "ETCONE") return ETCONE;

    if (uppercase_token == "VERTEXT") return VERT;
    if (uppercase_token == "VERTEXX") return VERX;
    if (uppercase_token == "VERTEXY") return VERY;
    if (uppercase_token == "VERTEXZ") return VERZ;
    if (uppercase_token == "VERTEXTR") return VERTR;

    if (uppercase_token == "ISTIGHT" ) return IS_TIGHT;
    if (uppercase_token == "ISMEDIUM") return IS_MEDIUM;
    if (uppercase_token == "ISLOOSE" ) return IS_LOOSE;
    if (token == "fmegajets") return FMEGAJETS;
    if (token == "fhemisphere") return FHEMISPHERE;
    if (token == "fMR") return FMR;
    if (token == "fMTR") return FMTR;
    if (token == "fMT2") return FMT2;
    if (token == "fMTauTau") return FMTAUTAU;
    if (uppercase_token == "MINIISO") return MINI_ISO;
    if (uppercase_token == "ABSISO") return ABS_ISO;

    if (token == "dxy"||uppercase_token == "D0") return DXY;
    if (token == "edxy" || uppercase_token == "ED0") return EDXY;
    if (token == "dz") return DZ;
    if (token == "edz") return EDZ;

    if (token == "m_HF_Classification") return HF_CLASSIFICATION; // generalize event variables.
    if (token == "fTTrr") return TTBAR_NNLOREC;

    if (uppercase_token == "PHI") return PHI;//functions
    if (uppercase_token == "ETA") return ETA;
    if (uppercase_token == "RAP") return RAPIDITY;
    if (uppercase_token == "ABSETA") return ABS_ETA;

    if (uppercase_token == "MSOFTDROP") return MSOFTDROP;

    if (uppercase_token == "THETA") return THETA;
    if (uppercase_token == "PT") return PT;
    if (uppercase_token == "PZ") return PZ;
    if (uppercase_token == "DR" || uppercase_token == "DELTAR") return DR;
    if (uppercase_token == "DPHI" || uppercase_token == "DELTAPHI") return DPHI;
    if (uppercase_token == "DETA" || uppercase_token == "DELTAETA") return DETA;
    if (uppercase_token == "SIZE" || uppercase_token == "COUNT" || uppercase_token == "NUMOF") return NUMOF;//no arg funcs 
    if (uppercase_token == "NBJ") return NBF;
    if (uppercase_token == "FHT") return HT; // attention
    if (token == "fAplanarity") return APLANARITY;
    if (token == "fSphericity") return SPHERICITY;
    if (token == "LEPsf") return LEP_SF;
    if (token == "bTagSF") return BTAGS_SF;
    if (token == "XSLumiCorrSF") return XSLUMICORR_SF;

    if (uppercase_token == "ANYOF") return ANYOF;
    if (uppercase_token == "ALLOF") return ALLOF;
    if (uppercase_token == "ALL") return ALL;
    if (uppercase_token == "NONE") return NONE;
    if (token == "=="|| uppercase_token == "EQ") return EQ;//comparison operators
    if (token == "!="|| uppercase_token == "NE") return NE;
    if (token == "~!") return MAXIMIZE;
    if (token == "~=") return MINIMIZE;
    if (token == "<="|| uppercase_token == "LE") return LE;
    if (token == ">="|| uppercase_token == "GE") return GE;
    if (token == "<"|| uppercase_token == "LT") return LT;
    if (token == ">"|| uppercase_token == "GT") return GT;
    if (token == "[]") return IRG;
    if (token == "][") return ERG;
    if (uppercase_token == "AND" || token == "&&") return AND;//logical ops
    if (uppercase_token == "OR" || token == "||") return OR;
    if (uppercase_token == "NOT") return NOT;
    if (uppercase_token == "WITHIN" || uppercase_token == "IN") return WITHIN;
    if (uppercase_token == "OUTSIDE") return OUTSIDE;

    if (token == "-") return MINUS;
    if (token == "+") return PLUS;
    if (token == "*") return MULTIPLY;
    if (token == "/") return DIVIDE;

    if (token == "&") return AMPERSAND;
    if (token == "|") return PIPE;
    if (token == ":") return COLON;
    if (token == "^") return RAISED_TO_POWER;


    if (token == "(") return OPEN_PAREN;
    if (token == ")") return CLOSE_PAREN;
    if (token == "[") return OPEN_SQUARE_BRACE;
    if (token == "]") return CLOSE_SQUARE_BRACE;
    if (token == "{") return OPEN_CURLY_BRACE;
    if (token == "}") return CLOSE_CURLY_BRACE;
    if (token == "?") return QUESTION;
    if (token == "=") return ASSIGN;
    if (token == "_") return UNDERSCORE;

    if (uppercase_token == "AVE") return AVE;
    if (uppercase_token == "SUM") return SUM;
    if (uppercase_token == "ADD") return ADD;
    if (uppercase_token == "SAVE") return SAVE;
    if (uppercase_token == "CSV") return CSV;
    if (uppercase_token == "ASCEND") return ASCEND;
    if (uppercase_token == "DESCEND") return DESCEND;
    if (uppercase_token == "TAN") return TAN;
    if (uppercase_token == "SIN") return SIN;
    if (uppercase_token == "COS") return COS;
    if (uppercase_token == "SINH") return SINH;
    if (uppercase_token == "COSH") return COSH;
    if (uppercase_token == "TANH") return TANH;
    if (uppercase_token == "EXP") return EXP;
    if (uppercase_token == "LOG") return LOG;
    if (uppercase_token == "HSTEP") return HSTEP;
    if (uppercase_token == "DELTA") return DELTA;
    if (uppercase_token == "ABS") return ABS;
    if (uppercase_token == "SQRT") return SQRT;
    if (uppercase_token == "SORT") return SORT;
    if (uppercase_token == "COMB") return COMB;
    if (uppercase_token == "PERM") return PERM;
    if (uppercase_token == "MIN") return MIN;
    if (uppercase_token == "MAX") return MAX;
    if (token == "+-"|| token == "-+") return PM;
    if (token == ">>") return BWR;
    if (token == "<<") return BWL;

    if (token == ",") return COMMA;

    // these letters are keywords in ADL, and so are their own tokens
    if (uppercase_token == "Q") return LETTER_Q; // charge
    if (uppercase_token == "E") return LETTER_E; // energy
    if (uppercase_token == "P") return LETTER_P; // momentum
    if (uppercase_token == "M") return LETTER_M; // mass

    // We have as of yet failed to lex this - if this is a number, we lex it
    if (std::regex_match(token, reg_int)) return INTEGER;
    if (std::regex_match(token, reg_decimal)) return DECIMAL;
    if (std::regex_match(token, reg_scientific)) return SCIENTIFIC;

    // It definintely isn't a number - maybe it's a variable name format
    if (std::regex_match(token, reg_varname)) return VARNAME;

    // Not that either - maybe it's just a valid string
    if (std::regex_match(token, reg_string)) return STRING;

    // Not any sort of valid object, so far as this can tell. Assume this is invalid text, and end our tokenization.
    return LEXER_ERROR;
}

void Lexer::lex_token(std::string &token, int &line_number, int &column_number) {

    // Do not lex an empty token
    if (token.size() == 0) return;

    auto tok = std::shared_ptr<Token>(new Token(identify_token(token)));
    tok->set_data(line_number, column_number, token);

    if (tok->get_token() == LEXER_ERROR) {
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

        int column = 0;
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

            if (current_char == '(' || current_char == ')' || current_char == ',' || current_char == '_' || current_char == '{' || current_char == '}' || current_char == '[' || current_char == ']') {
                // Exception: commas and parenthesis must be allowed to be stacked adjacent to whatever, and that must unequivocably be its own token - there is never a situation in which this should not be the case
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

            // if the running token is of the same type as the current char, then we add it to the string and keep going
            if (delimiter != previous_delimiter || symbol != previous_symbol) {
                if (!(prev_char == '-' && current_char >= '0' && current_char <= '9')) {
                    // Otherwise, we do not have compatible symbols, so this clearly must be the end of a running token
                    lex_token(running_token, line, column);
                }
            } 
            running_token = running_token + current_char;
        }
        // The line is over, we lex the remainder
        lex_token(running_token, line, column);

        auto endline = std::shared_ptr<Token>(new Token(LEXER_NEWLINE));
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
        if (tok->get_token() == LEXER_NEWLINE) {
            std::cout << std::endl;
            continue;
        }
        std::cout << tok->get_token() << ": " << tok->get_lexeme() << "," << " ";
    }
}

void Lexer::erase_whitespace() {

    non_whitespace_tokens.clear();

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {

        auto tok = *it;
        switch (tok->get_token()) {
            // If this is a whitespace or comment, don't even consider it in parsing
            case LEXER_ERROR: case LEXER_NEWLINE: case LEXER_COMMENT: case LEXER_SPACE:
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
    if (current_token == non_whitespace_tokens.end()) return std::shared_ptr<Token>(new Token(LEXER_END_OF_FILE));

    auto tok = *current_token;
    ++current_token;

    return tok;
}

void Lexer::expect_and_consume(Token_type type, std::string error) {
    auto tok = next();
    if (tok->get_token() != type) {
        raise_parsing_exception(error, tok);
    }
}

void Lexer::expect_and_consume(Token_type type) {
    expect_and_consume(type, "Unexpected token");
}


std::shared_ptr<Token> Lexer::peek(int lookahead) {
    auto ahead_tok_it = current_token;

    while (lookahead > 0) {
        ++ahead_tok_it;
        lookahead--;
        if (ahead_tok_it == non_whitespace_tokens.end()) return std::shared_ptr<Token>(new Token(LEXER_END_OF_FILE));
    }

    if (ahead_tok_it == non_whitespace_tokens.end()) return std::shared_ptr<Token>(new Token(LEXER_END_OF_FILE));

    return *ahead_tok_it;
}