#ifndef TOKENS_H
#define TOKENS_H

#define TOKEN_LIST(X)                                                        \
    /* Lexer tokens */                                          \
    X(LEXER_ERROR)    /* signifies that a lexing error has occurred */        \
    X(COMMENT)                                                               \
    X(SPACE)                                                                 \
    X(NEWLINE)                                                               \
    X(END_OF_FILE)                                                           \
                                                                             \
    /* Literals / identifiers */                                              \
    X(DECIMAL)       /* decimal/floating-point-like */                        \
    X(SCIENTIFIC)    /* scientific notation e.g. 2E4 */                       \
    X(INTEGER)       /* standard integer */                                   \
    X(STRING)                                                               \
    X(VARNAME)       /* variable name format, name_LikeThis or Variable42 */  \
                                                                             \
    /* Definitions / declarations */                                          \
    X(DEF)                                                                   \
    X(OBJ)                                                                   \
    X(REG)                                                                   \
    X(COMP)                                                                  \
    X(HISTOLIST)                                                             \
    X(ADLINFO)                                                              \
    X(PARTICLE_KEYWORD)                                                     \
    X(ADD)                                                                  \
    X(EXTERNAL)                                                             \
    X(ATTRIBUTE)                                                            \
    X(CORRECTIONLIB) /* TODO: check implementation */                         \
                                                                             \
    /* Selection / booleans */                                                \
    X(SELECT)                                                               \
    X(REJEC)                                                                \
    X(TRUE)                                                                 \
    X(FALSE)                                                                \
                                                                             \
    /* Region / histogram / table */                                          \
    X(NVARS)                                                                \
    X(ERRORS)                                                               \
    X(TABLETYPE)                                                            \
    X(TAKE)                                                                 \
    X(HISTO)                                                                \
    X(WEIGHT)                                                               \
    X(TABLE)                                                                \
    X(BIN)                                                                  \
    X(NAMED)                                                                \
    X(BINS)                                                                 \
    X(UNION)                                                                \
    X(ALIAS)                                                                \
                                                                             \
    /* particle-related quantities */                                         \
    X(PHI)                                                                  \
    X(ETA)                                                                  \
    X(CHARGE)                                                               \
    X(MASS)                                                                 \
    X(PT)                                                                   \
    X(DR)                                                                   \
    X(DPHI)                                                                 \
    X(DETA)                                                                 \
    X(DR_HADAMARD)                                                          \
    X(DPHI_HADAMARD)                                                        \
    X(DETA_HADAMARD)                                                        \
    X(DISTINCT)                                                             \
    X(ANYOF)                                                                \
    X(ALLOF)                                                                \
    X(THIS)                                                                 \
                                                                             \
    /* operators */                                                           \
    X(EQ)                                                                   \
    X(NE)                                                                   \
    X(LE)                                                                   \
    X(GE)                                                                   \
    X(LT)                                                                   \
    X(GT)                                                                   \
    X(AND)                                                                  \
    X(OR)                                                                   \
    X(NOT)                                                                  \
    X(WITHIN)                                                               \
    X(OUTSIDE)                                                              \
    X(MINUS)                                                                \
    X(PLUS)                                                                 \
    X(MULTIPLY)                                                             \
    X(DIVIDE)                                                               \
    X(AMPERSAND)    /* bitwise operator */                                   \
    X(PIPE)         /* bitwise operator */                                   \
    X(COLON)                                                                \
    X(RAISED_TO_POWER)                                                      \
    X(DOT_INDEX)                                                            \
    X(ARROW_INDEX)                                                          \
    X(OPEN_PAREN)                                                           \
    X(CLOSE_PAREN)                                                          \
    X(OPEN_SQUARE_BRACE)                                                    \
    X(CLOSE_SQUARE_BRACE)                                                   \
    X(OPEN_CURLY_BRACE)                                                     \
    X(CLOSE_CURLY_BRACE)                                                    \
    X(QUESTION)                                                             \
    X(ASSIGN)       /* single = */                                           \
                                                                             \
    /* aggregates / functions */                                              \
    X(NUMOF)                                                                \
    X(AVE)                                                                  \
    X(SUM)                                                                  \
    X(ASCEND)                                                               \
    X(DESCEND)                                                              \
    X(TAN)                                                                  \
    X(SIN)                                                                  \
    X(COS)                                                                  \
    X(SINH)                                                                 \
    X(COSH)                                                                 \
    X(TANH)                                                                 \
    X(EXP)                                                                  \
    X(LOG)                                                                  \
    X(ABS)                                                                  \
    X(SQRT)                                                                 \
    X(SORT)                                                                 \
    X(COMB)                                                                 \
    X(DISJOINT)                                                             \
    X(DIRECT)                                                               \
    X(MIN)                                                                  \
    X(MAX)                                                                  \
                                                                             \
    /* separators */                                                   \
    X(COMMA)                                                                \
    X(UNDERSCORE)                                                           \
                                                                             \
    /* single-letter tokens */                                                \
    X(LETTER_M)                                                             \
    X(LETTER_Q)                                                             \
    X(LETTER_P)                                                             \
    X(LETTER_E)

#define MAKE_ENUM(ENUM) ENUM,

enum class Token_type {
    TOKEN_LIST(MAKE_ENUM)
};

#undef MAKE_ENUM


typedef Token_type TOK;

#define CASE_BUILT_IN_MATH_FUN TOK::ANYOF: case TOK::ALLOF: case TOK::SQRT: case TOK::ABS: case TOK::COS:  case TOK::SIN: case TOK::TAN: case TOK::SINH: case TOK::COSH: case TOK::TANH: case TOK::EXP: case TOK::LOG: case TOK::AVE: case TOK::SUM

#define CASE_BUILT_IN_PARTICLE_FUN_ONE_ARG TOK::LETTER_E: case TOK::LETTER_P: case TOK::LETTER_M: case TOK::LETTER_Q: \
case TOK::CHARGE: case TOK::MASS: case TOK::PHI: case TOK::ETA: case TOK::PT

#define CASE_BUILT_IN_PARTICLE_FUN_TWO_ARG TOK::DR: case TOK::DPHI: case TOK::DETA: \
case TOK::DR_HADAMARD: case TOK::DPHI_HADAMARD: case TOK::DETA_HADAMARD: \
case TOK::NUMOF: case TOK::DISTINCT

#endif
