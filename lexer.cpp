#include "lexer.h"
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <vector>

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
    reg_varname = std::regex("[A-Za-z][A-Za-z0-9]*");
    reg_string= std::regex("\"[^\"]*\"");

    reg_whitespace = std::regex("\\s");

}

Token Lexer::identify_token(std::string &token) {
    std::string uppercase_token = convert_to_uppercase(token);

    if (verbose) std::cout << "Lexing " << token << std::endl;
    
    // If we start with a hash, then this is instantly a comment
    if (token.front() == '#') return Token(LEXER_COMMENT);
    if (std::regex_match(token, std::regex("\\s+"))) return Token(LEXER_SPACE);

    if (uppercase_token == "DEF" || uppercase_token == "DEFINE") return Token(DEF);
    if (uppercase_token == "CMD" || uppercase_token == "CUT" || uppercase_token == "SELECT") return Token(CMD);
    if (uppercase_token == "REJECT") return Token(REJEC);
    if (uppercase_token == "OBJ" || uppercase_token == "OBJECT") return Token(OBJ);

    if (uppercase_token == "ALGORITHM" || uppercase_token == "ALGO" || uppercase_token == "REGION") return Token(ALGO);
    if (token == "TRGe") return Token(TRGE);
    if (token == "TRGm") return Token(TRGM);
    if (uppercase_token == "INFO") return Token(ADLINFO);

    if (token == "experiment") return Token(PAP_EXPERIMENT);
    if (token == "id") return Token(PAP_ID);
    if (uppercase_token == "TITLE") return Token(PAP_TITLE);
    if (token == "publication") return Token(PAP_PUBLICATION);
    if (token == "sqrtS") return Token(PAP_SQRTS);
    if (token == "lumi" ) return Token(PAP_LUMI);
    if (token == "arXiv") return Token(PAP_ARXIV);
    if (token == "hepdata") return Token(PAP_HEPDATA);
    if (token == "doi"   ) return Token(PAP_DOI);

    if (token == "counts" ) return Token(COUNTS);
    if (token == "countsformat") return Token(COUNTSFORMAT);
    if (token == "stat") return Token(ERR_STAT);
    if (token == "syst") return Token(ERR_SYST);
    if (token == "process") return Token(PROCESS);

    if (token == "systematic") return Token(SYSTEMATIC);
    if (token == "ttree") return Token(SYST_TTREE);
    if (token == "weight_mc") return Token(SYST_WEIGHT_MC);
    if (token == "weight_pileup") return Token(SYST_WEIGHT_PILEUP);
    if (token == "weight_jvt") return Token(SYST_WEIGHT_JVT);
    if (token == "weight_leptonSF") return Token(SYST_WEIGHT_LEPTON_SF);
    if (token == "weight_BTagSF") return Token(SYST_WEIGHT_BTAG_SF);
    if (token == "RunYear") return Token(RUNYEAR);
    if (token == "mcChannelNumber") return Token(MC_CHANNEL_NUMBER);
    if (uppercase_token == "EVENTNO") return Token(EVENT_NO);
    if (uppercase_token == "RUNNO") return Token(RUN_NO);
    if (uppercase_token == "LBNO") return Token(LB_NO);
    if (token == "OME") return Token(OME);

    if (uppercase_token == "PRINT") return Token(PRINT);
    if (uppercase_token == "ON" || uppercase_token == "TRUE") return Token(TRUE); 
    if (uppercase_token == "OFF" || uppercase_token == "FALSE") return Token(FALSE); 
    if (uppercase_token == "NVARS") return Token(NVARS);
    if (uppercase_token == "ERRORS") return Token(ERRORS);
    if (uppercase_token == "TABLETYPE") return Token(TABLETYPE);
    if (uppercase_token == "TAKE" || uppercase_token == "USING") return Token(TAKE);
    if (uppercase_token == "HISTO") return Token(HISTO);
    if (uppercase_token == "WEIGHT") return Token(WEIGHT);
    if (uppercase_token == "TABLE") return Token(TABLE);
    if (uppercase_token == "SKIPHISTOS") return Token(SKPH);
    if (uppercase_token == "SKIPEFS") return Token(SKPE);
    if (uppercase_token == "GEN") return Token(GEN);
    if (uppercase_token == "ELE"|| token == "Electron"|| token == "electron") return Token(ELECTRON);//particles
    if (uppercase_token == "MUO" || token == "MUON"| token == "muon") return Token(MUON);
    if (uppercase_token == "TAU") return Token(TAU);
    if (uppercase_token == "TRK") return Token(TRACK);
    if (uppercase_token == "PHO" || uppercase_token == "Photon") return Token(PHOTON);
    if (uppercase_token == "JET") return Token(JET);
    if (uppercase_token == "FJET"|| uppercase_token == "FatJet") return Token(FJET);
    if (uppercase_token == "QGJET") return Token(QGJET);
    if (uppercase_token == "BIN") return Token(BINS);
    if (token == "daughters" || token == "constituents") return Token(CONSTITUENTS);
    if (token == "NUMET") return Token(NUMET);
    if (token == "METLV") return Token(METLV);
    if (token == "LEP") return Token(LEPTON);
    if (uppercase_token == "HLT") return Token(HLT);
    if (token == "BJET") return Token(BJET);
    if (uppercase_token == "INDEX") return Token(IDX);

    if (uppercase_token == "METSIG") return Token(METSIGNIF);
    if (token == "applyHM") return Token(APPLY_HM);
    if (token == "applyPTF" || token == "scalePT" ) return Token(APPLY_PTF);
    if (token == "applyEF" || token == "scaleE" ) return Token(APPLY_EF);
    if (token == "genPartIdx") return Token(GENPART_IDX);

    if (uppercase_token == "UNION") return Token(UNION);
    if (uppercase_token == "ALIAS") return Token(ALIAS);

    if (uppercase_token == "BTAG") return Token(IS_BTAG);
    if (uppercase_token == "CTAG") return Token(IS_CTAG);
    if (uppercase_token == "TAUTAG") return Token(IS_TAUTAG);

    if (uppercase_token == "PDGID") return Token(PDG_ID);
    if (uppercase_token == "FLAVOR" | uppercase_token == "BTAGGER") return Token(FLAVOR);
    if (uppercase_token == "PTCONE") return Token(PTCONE);
    if (uppercase_token == "ETCONE") return Token(ETCONE);

    if (uppercase_token == "VERTEXT") return Token(VERT);
    if (uppercase_token == "VERTEXX") return Token(VERX);
    if (uppercase_token == "VERTEXY") return Token(VERY);
    if (uppercase_token == "VERTEXZ") return Token(VERZ);
    if (uppercase_token == "VERTEXTR") return Token(VERTR);

    if (uppercase_token == "ISTIGHT") return Token(IS_TIGHT);
    if (uppercase_token == "MEDIUM") return Token(IS_MEDIUM);
    if (uppercase_token == "LOOSE") return Token(IS_LOOSE);
    if (token == "fmegajets") return Token(FMEGAJETS);
    if (token == "fhemisphere") return Token(FHEMISPHERE);
    if (token == "fMR") return Token(FMR);
    if (token == "fMTR") return Token(FMTR);
    if (token == "fMT2") return Token(FMT2);
    if (token == "fMTauTau") return Token(FMTAUTAU);
    if (uppercase_token == "MINIISO") return Token(MINI_ISO);
    if (uppercase_token == "ABSISO") return Token(ABS_ISO);

    if (token == "dxy"||uppercase_token == "D0") return Token(DXY);
    if (token == "edxy" || uppercase_token == "ED0") return Token(EDXY);
    if (token == "dz") return Token(DZ);
    if (token == "edz") return Token(EDZ);

    if (token == "m_HF_Classification") return Token(HF_CLASSIFICATION); // generalize event variables.
    if (token == "fTTrr") return Token(TTBAR_NNLOREC);

    if (uppercase_token == "PHI") return Token(PHI);//functions
    if (uppercase_token == "ETA") return Token(ETA);
    if (uppercase_token == "RAP") return Token(RAPIDITY);
    if (uppercase_token == "EBSETA") return Token(ABS_ETA);
    if (uppercase_token == "THETA") return Token(THETA);
    if (uppercase_token == "PT") return Token(PT);
    if (uppercase_token == "PZ") return Token(PZ);
    if (uppercase_token == "DR") return Token(DR);
    if (uppercase_token == "DPHI") return Token(DPHI);
    if (uppercase_token == "DETA") return Token(DETA);
    if (uppercase_token == "SIZE" || uppercase_token == "COUNT" || uppercase_token == "NUMOF") return Token(NUMOF);//no arg funcs 
    if (uppercase_token == "NBJ") return Token(NBF);
    if (uppercase_token == "FHT") return Token(HT); // attention
    if (token == "MET") return Token(MET);
    if (token == "fAplanarity") return Token(APLANARITY);
    if (token == "fSphericity") return Token(SPHERICITY);
    if (token == "LEPsf") return Token(LEP_SF);
    if (token == "bTagSF") return Token(BTAGS_SF);
    if (token == "XSLumiCorrSF") return Token(XSLUMICORR_SF);

    if (uppercase_token == "ANYOF") return Token(ANYOF);
    if (uppercase_token == "ALLOF") return Token(ALLOF);
    if (uppercase_token == "ALL") return Token(ALL);
    if (uppercase_token == "NONE") return Token(NONE);
    if (token == "=="|| uppercase_token == "EQ") return Token(EQ);//comparison operators
    if (token == "!="|| uppercase_token == "NE") return Token(NE);
    if (token == "~!") return Token(MAXIMIZE);
    if (token == "~=") return Token(MINIMIZE);
    if (token == "<="|| uppercase_token == "LE") return Token(LE);
    if (token == ">="|| uppercase_token == "GE") return Token(GE);
    if (token == "<"|| uppercase_token == "LT") return Token(LT);
    if (token == ">"|| uppercase_token == "GT") return Token(GT);
    if (token == "[]") return Token(IRG);
    if (token == "][") return Token(ERG);
    if (uppercase_token == "AND" || token == "&&") return Token(AND);//logical ops
    if (uppercase_token == "OR" || token == "||") return Token(OR);
    if (uppercase_token == "NOT") return Token(NOT);

    if (token == "-") return Token(MINUS);
    if (token == "+") return Token(PLUS);
    if (token == "*") return Token(MULTIPLY);
    if (token == "/") return Token(DIVIDE);

    if (token == "&") return Token(AMPERSAND);
    if (token == "|") return Token(PIPE);
    if (token == ":") return Token(COLON);
    if (token == "^") return Token(RAISED_TO_POWER);


    if (token == "(") return Token(OPEN_PAREN);
    if (token == ")") return Token(CLOSE_PAREN);
    if (token == "[") return Token(OPEN_SQUARE_BRACE);
    if (token == "]") return Token(CLOSE_SQUARE_BRACE);
    if (token == "{") return Token(OPEN_CURLY_BRACE);
    if (token == "}") return Token(CLOSE_CURLY_BRACE);
    if (token == "?") return Token(QUESTION);
    if (token == "=") return Token(ASSIGN);

    if (uppercase_token == "AVE") return Token(AVE);
    if (uppercase_token == "SUM") return Token(SUM);
    if (uppercase_token == "ADD") return Token(ADD);
    if (uppercase_token == "SAVE") return Token(SAVE);
    if (uppercase_token == "CSV") return Token(CSV);
    if (uppercase_token == "ASCEND") return Token(ASCEND);
    if (uppercase_token == "DESCEND") return Token(DESCEND);
    if (uppercase_token == "TAN") return Token(TAN);
    if (uppercase_token == "SIN") return Token(SIN);
    if (uppercase_token == "COS") return Token(COS);
    if (uppercase_token == "SINH") return Token(SINH);
    if (uppercase_token == "COSH") return Token(COSH);
    if (uppercase_token == "TANH") return Token(TANH);
    if (uppercase_token == "EXP") return Token(EXP);
    if (uppercase_token == "LOG") return Token(LOG);
    if (uppercase_token == "HSTEP") return Token(HSTEP);
    if (uppercase_token == "DELTA") return Token(DELTA);
    if (uppercase_token == "ABS") return Token(ABS);
    if (uppercase_token == "SQRT") return Token(SQRT);
    if (uppercase_token == "SORT") return Token(SORT);
    if (uppercase_token == "COMB") return Token(COMB);
    if (uppercase_token == "PERM") return Token(PERM);
    if (uppercase_token == "MIN") return Token(MIN);
    if (uppercase_token == "MAX") return Token(MAX);
    if (token == "+-"|| token == "-+") return Token(PM);
    if (token == ">>") return Token(BWR);
    if (token == "<<") return Token(BWL);

    if (token == ",") return Token(COMMA);

    // We have as of yet failed to lex this - if this is a number, we lex it
    if (std::regex_match(token, reg_int)) return Token(INTEGER);
    if (std::regex_match(token, reg_decimal)) return Token(DECIMAL);
    if (std::regex_match(token, reg_scientific)) return Token(SCIENTIFIC);

    // It definintely isn't a number - maybe it's a variable name format?
    if (std::regex_match(token, reg_varname)) return Token(VARNAME);

    // Not that either - maybe it's just a valid string
    if (std::regex_match(token, reg_string)) return Token(STRING);

    // Not any sort of valid object, so far as this can tell. Assume this is invalid text, and end our tokenization.
    return Token(LEXER_ERROR);
}

void Lexer::lex_token(std::string &token, int &line_number, int &column_number) {

    // Do not lex an empty token
    if (token.size() == 0) return;

    Token tok = identify_token(token);

    if (tok.get_token() == LEXER_ERROR) {

        std::stringstream stream;
        stream << "Malformed token \"" << token << "\", at line " << line_number << ", column " << column_number << std::endl;

        throw LexingException(stream.str().c_str());
    }

    tok.set_data(line_number, column_number, token);
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

            } else if (is_commented_out) {
                // If this line is commented out, this is all just one comment
                running_token += current_char;
                continue;
            }

            // if we have a quote character, and we are in a quote scope, then we have ceased to be so.
            if (is_quoted_out && current_char == '"') {
                running_token += current_char;
                lex_token(running_token, line, column);
                continue;
            }

            bool delimiter = is_delimiter(current_char);
            bool symbol = is_symbol(current_char);

            if (current_char == '(' || current_char == ')' || current_char == ',') {
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
                // Otherwise, we do not have compatible symbols, so this clearly must be the end of a running token
                lex_token(running_token, line, column);
            } 
            running_token = running_token + current_char;
        }
        // The line is over, we lex the remainder
        lex_token(running_token, line, column);

        Token endline(LEXER_NEWLINE);
        endline.set_data(line, column, "\n");
        tokens.push_back(endline);

        // Return to the start of the next line
    }

    read_file.close();
}

void Lexer::print() {
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->get_token() == LEXER_NEWLINE) {
            std::cout << std::endl;
            continue;
        }
        std::cout << it->get_token() << ": " << it->get_lexeme() << "," << " ";
    }
}

Token &Lexer::next() {
    Token &tok = *current_token;
    ++current_token;

    return tok;
}

Token &Lexer::peek(int lookahead) {

    Token &ahead_tok = *(current_token + lookahead);

    return ahead_tok;
}