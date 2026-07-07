#ifndef TOKENS_H
#define TOKENS_H

enum Token_type {
    TOK_LEXER_ERROR, // signifies that a lexing error has occurred

    TOK_COMMENT,

    TOK_SPACE,
    TOK_NEWLINE,
    TOK_END_OF_FILE,

    TOK_DECIMAL,    // Decimal/floating-point-like
    TOK_SCIENTIFIC, // Scientific notation e.g. 2E4
    TOK_INTEGER,    // standard integer

    TOK_STRING,
    TOK_VARNAME,    // variable name format, name_LikeThis or Variable42 or similar
    
    TOK_DEF,
    TOK_OBJ,
    TOK_REG,
    TOK_COMP,

    TOK_HISTOLIST,

    TOK_ADLINFO,

    TOK_PARTICLE_KEYWORD,
    TOK_EXTERNAL,
    TOK_ATTRIBUTE,
    TOK_CORRECTIONLIB,  //TODO: check implementation

    TOK_SELECT,
    TOK_REJEC,

    TOK_TRUE,
    TOK_FALSE,
    
    TOK_NVARS,
    TOK_ERRORS,
    TOK_TABLETYPE,

    TOK_TAKE,

    TOK_HISTO,
    TOK_WEIGHT,

    TOK_TABLE,

    TOK_BIN,
    TOK_BINS,

    TOK_UNION,
    TOK_ALIAS,

    TOK_PHI,
    TOK_ETA,
    TOK_CHARGE,
    TOK_MASS,
    TOK_PT,

    TOK_DR,
    TOK_DPHI,
    TOK_DETA,

    TOK_DR_HADAMARD,
    TOK_DPHI_HADAMARD,
    TOK_DETA_HADAMARD,

    TOK_DISTINCT,

    TOK_ANYOF,
    TOK_ALLOF,
    TOK_ALL,
    TOK_NONE,

    TOK_THIS,

    TOK_EQ,
    TOK_NE,
    TOK_LE,
    TOK_GE,
    TOK_LT,
    TOK_GT,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_WITHIN,
    TOK_OUTSIDE,

    TOK_MINUS,
    TOK_PLUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,

    TOK_AMPERSAND, // bitwise operator
    TOK_PIPE, // bitwise operator

    TOK_COLON,
    TOK_RAISED_TO_POWER,

    TOK_DOT_INDEX,
    TOK_ARROW_INDEX,

    TOK_OPEN_PAREN,
    TOK_CLOSE_PAREN,
    TOK_OPEN_SQUARE_BRACE,
    TOK_CLOSE_SQUARE_BRACE,
    TOK_OPEN_CURLY_BRACE,
    TOK_CLOSE_CURLY_BRACE,

    TOK_QUESTION,
    TOK_ASSIGN, // single =

    TOK_NUMOF,
    TOK_AVE,
    TOK_SUM,


    TOK_ASCEND,
    TOK_DESCEND,

    TOK_TAN,
    TOK_SIN,
    TOK_COS,

    TOK_SINH,
    TOK_COSH,
    TOK_TANH,

    TOK_EXP,
    TOK_LOG,
    TOK_ABS,
    TOK_SQRT,

    TOK_SORT,

    TOK_COMB,
    TOK_DISJOINT,
    TOK_DIRECT,

    TOK_MIN,
    TOK_MAX,


    TOK_COMMA,
    TOK_UNDERSCORE,

    TOK_LETTER_M,
    TOK_LETTER_Q,
    TOK_LETTER_P,
    TOK_LETTER_E,


    // IF,
    // THEN,
    // ELSE,
    // DO,
    // PRINT,
    
    SYSTEMATIC,

    PM,

    
    ADD,

};

#endif
