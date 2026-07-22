#ifndef TOKENS_H
#define TOKENS_H

enum class Token_type {
    LEXER_ERROR, // signifies that a lexing error has occurred

    COMMENT,

    SPACE,
    NEWLINE,
    END_OF_FILE,

    DECIMAL,    // Decimal/floating-point-like
    SCIENTIFIC, // Scientific notation e.g. 2E4
    INTEGER,    // standard integer

    STRING,
    VARNAME,    // variable name format, name_LikeThis or Variable42 or similar
    
    DEF,
    OBJ,
    REG,
    COMP,

    HISTOLIST,

    ADLINFO,

    PARTICLE_KEYWORD,
    ADD,

    EXTERNAL,
    ATTRIBUTE,
    CORRECTIONLIB,  //TODO: check implementation

    SELECT,
    REJEC,

    TRUE,
    FALSE,
    
    NVARS,
    ERRORS,
    TABLETYPE,

    TAKE,

    HISTO,
    WEIGHT,

    TABLE,

    BIN,
    NAMED,
    BINS,

    UNION,
    ALIAS,

    PHI,
    ETA,
    CHARGE,
    MASS,
    PT,

    DR,
    DPHI,
    DETA,

    DR_HADAMARD,
    DPHI_HADAMARD,
    DETA_HADAMARD,

    DISTINCT,

    ANYOF,
    ALLOF,

    THIS,

    EQ,
    NE,
    LE,
    GE,
    LT,
    GT,
    AND,
    OR,
    NOT,
    WITHIN,
    OUTSIDE,

    MINUS,
    PLUS,
    MULTIPLY,
    DIVIDE,

    AMPERSAND, // bitwise operator
    PIPE, // bitwise operator

    COLON,
    RAISED_TO_POWER,

    DOT_INDEX,
    ARROW_INDEX,

    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_SQUARE_BRACE,
    CLOSE_SQUARE_BRACE,
    OPEN_CURLY_BRACE,
    CLOSE_CURLY_BRACE,

    QUESTION,
    ASSIGN, // single =

    NUMOF,
    AVE,
    SUM,


    ASCEND,
    DESCEND,

    TAN,
    SIN,
    COS,

    SINH,
    COSH,
    TANH,

    EXP,
    LOG,
    ABS,
    SQRT,

    SORT,

    COMB,
    DISJOINT,
    DIRECT,

    MIN,
    MAX,


    COMMA,
    UNDERSCORE,

    LETTER_M,
    LETTER_Q,
    LETTER_P,
    LETTER_E,

};

typedef Token_type TOK;

#define CASE_BUILT_IN_MATH_FUN TOK::ANYOF: case TOK::ALLOF: case TOK::SQRT: case TOK::ABS: case TOK::COS:  case TOK::SIN: case TOK::TAN: case TOK::SINH: case TOK::COSH: case TOK::TANH: case TOK::EXP: case TOK::LOG: case TOK::AVE: case TOK::SUM

#define CASE_BUILT_IN_PARTICLE_FUN_ONE_ARG TOK::LETTER_E: case TOK::LETTER_P: case TOK::LETTER_M: case TOK::LETTER_Q: \
case TOK::CHARGE: case TOK::MASS: case TOK::PHI: case TOK::ETA: case TOK::PT

#define CASE_BUILT_IN_PARTICLE_FUN_TWO_ARG TOK::DR: case TOK::DPHI: case TOK::DETA: \
case TOK::DR_HADAMARD: case TOK::DPHI_HADAMARD: case TOK::DETA_HADAMARD: \
case TOK::NUMOF: case TOK::DISTINCT

#endif
