#include "parser.h"
#include <memory>


void raise_exception(std::string error, PToken token) {
    std::stringstream stream;
    stream << "Failed to parse \"" << token->get_lexeme() << "\", at line " << token->get_line() << ", column " << token->get_column() << ": " << error << std::endl;

    throw ParsingException(stream.str().c_str());
}

Parser::Parser (Lexer &lex): lexer(lex), tree(INPUT) {

}

void Parser::parse() {
    lexer.reset();
    parse_input();

}

void Parser::parse_input() {
    // INPUT -> INITIALIZATIONS COUNTFORMATS DEFINITIONS_AND_OBJECTS COMMANDS
    
    PNode input_node = tree.get_root();
    input_node->add_child(parse_initializations(input_node));
    input_node->add_child(parse_count_formats(input_node));
    input_node->add_child(parse_definitions_and_objects(input_node));
    input_node->add_child(parse_commands(input_node));
}

PNode Parser::parse_initializations(PNode parent) {
    PNode initializations(new Node(INITIALIZATIONS, parent));

    PToken next = lexer.peek(0);
    
    switch (next->get_token()) {
        case SKPH: case SKPE: case ADLINFO: case PAP_TITLE: case PAP_EXPERIMENT: case PAP_ID: case PAP_PUBLICATION: case PAP_SQRTS: case PAP_LUMI: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA: case SYSTEMATIC:
            // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
            initializations->add_child(parse_initialization(initializations));
            parse_initializations(parent);
            break;
        case TRGE: case TRGM: 
            if (lexer.peek(1)->get_token() == EQ) {
                // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
                // disambiguates on the TRGE and TRGM tokens
                initializations->add_child(parse_initialization(initializations));
                parse_initializations(parent);
            } break;
        default:
            // INITIALIZATONS -> epsilon
            break;
    }

    return initializations;
}

void add_two_terminal_children(PNode parent, PToken one, PToken two) {
    PNode op(new Node(TERMINAL, parent));
    parent->add_child(op);
    op->set_token(one);

    PNode source(new Node(TERMINAL, parent));
    parent->add_child(source);
    source->set_token(two); 
}

bool is_numerical(Token_type t) {
    if (t == INTEGER || t == DECIMAL || t == SCIENTIFIC) return true;
    return false;
} 

PNode Parser::parse_initialization(PNode parent) {
    PNode initialization(new Node(INITIALIZATION, parent));

    PToken tok = lexer.next();

    switch(tok->get_token()) {
        // INITIALIZATION -> TRGE = INTEGER
        // INITIALIZATION -> TRGM = INTEGER
        // INITIALIZATION -> SKPH = INTEGER
        // INITIALIZATION -> SKPE = INTEGER
        case TRGE: case TRGM: case SKPH: case SKPE:
        {// Consume terminal equals
            lexer.next();
            PToken next = lexer.next();
            if (next->get_token() != INTEGER) raise_exception("Invalid non-integer assignment", next);
            add_two_terminal_children(initialization, tok, next);
        } break;
        case PAP_LUMI: case PAP_SQRTS:
        {
            PToken next = lexer.next();
            if (!is_numerical(next->get_token())) raise_exception("Non-numerical value given for numerical field", next); 
            add_two_terminal_children(initialization, tok, next);  
        } break;
        case ADLINFO: case PAP_EXPERIMENT:  
        {
            PToken next = lexer.next();
            if (next->get_token() != INTEGER && next->get_token() != STRING) raise_exception("Invalid ID, allowed types are integers and strings", next);
        } break;
        case SYSTEMATIC:
            initialization->add_child(parse_bool(initialization));

        case PAP_TITLE: case PAP_PUBLICATION: case PAP_ID: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA:
            
    }

}

PNode Parser::parse_bool(PNode parent) {
    auto tok = lexer.next();
    if (!(tok->get_token() == TRUE) && !(tok->get_token() == FALSE)) raise_exception("Excepted boolean, but token is not interpretable as a boolean", tok);

    PNode boolean(new Node(TERMINAL, parent));
    boolean->set_token(tok);
    return boolean;
}

PNode Parser::parse_description(PNode parent) {
    // DESCRIPTION -> STRING DESCRIPTION
    auto tok = lexer.peek(0);
    if (tok->get_token() == STRING) {
        // consume the string
        lexer.next();
        PNode description_str(new Node(TERMINAL, parent));
        description_str->set_token(tok);
        parent->add_child(parse_description(parent));
    }

    // DESCRIPTION -> epsilon
}

PNode Parser::parse_count_formats(PNode parent) {
    PNode count_formats(new Node(COUNT_FORMATS, parent));
}
PNode Parser::parse_definitions_and_objects(PNode parent) {}
PNode Parser::parse_commands(PNode parent) {};
