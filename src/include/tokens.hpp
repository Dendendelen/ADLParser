#ifndef TOKENS_H
#define TOKENS_H

enum Token_type {
    LEXER_ERROR, // type to signify that a lexing error has occurred
    LEXER_COMMENT,
    LEXER_SPACE,
    LEXER_NEWLINE,
    LEXER_END_OF_FILE,

    DECIMAL,
    SCIENTIFIC,

    STRING,
    INTEGER,
    // EXP_INT,
    // REAL,
    VARNAME,
    // VARDEF,
    
    DEF,
    SELECT,
    REJEC,
    OBJ,
    ALGO,
    COMP,

    HISTOLIST,

    ADLINFO,
    PAP_EXPERIMENT,
    PAP_ID,
    PAP_TITLE,
    PAP_PUBLICATION,
    PAP_SQRTS,
    PAP_LUMI,
    PAP_ARXIV,
    PAP_HEPDATA,
    PAP_DOI,

    PARTICLE_KEYWORD,
    EXTERNAL,
    CORRECTIONLIB,
    
    SYSTEMATIC,
    SYST_TTREE,
    SYST_WEIGHT_MC,
    SYST_WEIGHT_PILEUP,
    SYST_WEIGHT_JVT,
    SYST_WEIGHT_LEPTON_SF,
    SYST_WEIGHT_BTAG_SF,

    RUNYEAR,
    MC_CHANNEL_NUMBER,
    EVENT_NO,
    RUN_NO,
    LB_NO,
    OME,

    IF,
    THEN,
    ELSE,
    DO,
    PRINT,

    TRUE,
    FALSE,
    
    NVARS,
    ERRORS,
    TABLETYPE,

    TAKE,
    HISTO,
    WEIGHT,

    TABLE,
    SKIP_HISTO,
    SKIP_EFFS,
    GEN,
    
    ELECTRON,
    MUON,
    TAU,
    TRACK,
    PHOTON,
    JET,
    FJET,
    QGJET,

    BIN,
    BINS,
    CONSTITUENTS,

    METLV,

    GENPART_IDX,

    UNION,
    ALIAS,

    IS_BTAG,
    IS_CTAG,
    IS_TAUTAG,

    PDG_ID,
    JET_ID,
    STATUS_FLAGS,
    
    FLAVOR,

    IS_TIGHT,
    IS_MEDIUM,
    IS_LOOSE,

    MINI_ISO,
    ABS_ISO,

    DXY,
    DZ,

    PHI,
    ETA,
    RAPIDITY,

    CHARGE,
    MASS,

    MSOFTDROP,

    PT,
    PZ,

    THETA,

    DR,

    DPHI,
    DETA,

    DR_HADAMARD,
    DPHI_HADAMARD,
    DETA_HADAMARD,

    ANYOF,
    ALLOF,
    ALL,
    NONE,

    THIS,

    EQ,
    NE,
    MAXIMIZE,
    MINIMIZE,
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

    AMPERSAND,
    PIPE,
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
    ASSIGN,

    NUMOF,
    AVE,
    SUM,
    ADD,
    SAVE,
    CSV,
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

    ANYOCCURRENCES,
    COMB,
    DISJOINT,
    MIN,
    MAX,
    FIRST,
    SECOND,

    PM,

    COMMA,
    UNDERSCORE,

    LETTER_M,
    LETTER_Q,
    LETTER_P,
    LETTER_E,

    INT,
    NB,
    PNB,
    ID,
    HID,


};

#endif
