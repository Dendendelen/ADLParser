#include "parser.hpp"
#include <memory>

#include <iostream>

#include "exceptions.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"



PNode create_node(AST_type in, PNode parent, PToken tok) {
    PNode new_node(std::make_shared<Node>(in, parent, tok));
    parent->add_child(new_node);
    return new_node;
}

/**
 * @brief Create an AST node object and adds it as a child of its parent
 * 
 * @param in: type of AST node to be created
 * @param parent" parent of the new node
 * @return: PNode, newly created node
 */
PNode create_node(AST_type in, PNode parent) {
    PNode new_node(std::make_shared<Node>(in, parent));
    parent->add_child(new_node);
    return new_node;
}

PNode create_lost_node(AST_type in, PNode parent, PToken tok) {
    return std::make_shared<Node>(in, parent, tok);
}

PNode create_lost_node(AST_type in, PNode parent) {
    return std::make_shared<Node>(in, parent);
}

PNode make_varying_terminal(PNode parent, PToken tok) {
    PNode new_terminal_node(std::make_shared<Node>(AST_VARYING_TERMINAL, parent, tok));
    parent->add_child(new_terminal_node);
    return new_terminal_node;
}

PNode make_lost_varying_terminal(PNode parent, PToken tok) {
    PNode new_terminal_node(std::make_shared<Node>(AST_VARYING_TERMINAL, parent, tok));
    return new_terminal_node;
}

PNode make_operator_terminal(PNode parent, PToken tok) {
    PNode new_terminal_node(std::make_shared<Node>(AST_OPERATOR_TERMINAL, parent, tok));
    parent->add_child(new_terminal_node);
    return new_terminal_node;
}

PNode make_lost_operator_terminal(PNode parent, PToken tok) {
    PNode new_terminal_node(std::make_shared<Node>(AST_OPERATOR_TERMINAL, parent, tok));
    return new_terminal_node;
}


PNode make_lost_builtin_func_terminal(PNode parent, PToken tok) {
    PNode new_terminal_node(std::make_shared<Node>(AST_BUILTIN_FUNC_TERMINAL, parent, tok));
    return new_terminal_node;
}

/**
 * @brief Gets or creates the root node of a list
 * 
 * @param in: type of the AST node list root
 * @param parent: parent of the node
 * @return: PNode, either the parent if it already matches the type, or a newly created node
 */
PNode make_list_root_node(AST_type in, PNode parent){
    if (parent->get_ast_type() == in) return parent;
    return create_node(in, parent);
}

bool is_numerical(Token_type t) {
    if (t == TOK_INTEGER || t == TOK_DECIMAL || t == TOK_SCIENTIFIC) return true;
    return false;
} 

Parser::Parser (Lexer *lex): lexer(lex), tree(AST_INPUT) {
}

void Parser::parse() {
    lexer->reset();
    parse_input();
}


/*
INPUT productions:
---

    INPUT -> BLOCKS

*/
void Parser::parse_input() {
    // INPUT -> BLOCKS
    PNode input_node = tree.get_root();
    parse_blocks(input_node);
}


/* 
BLOCKS productions:
---

    BLOCKS -> INFO BLOCKS

    BLOCKS -> DEFINITION BLOCKS

    BLOCKS -> COMPOSITE BLOCKS

    BLOCKS -> OBJECT BLOCKS

    BLOCKS -> TABLE BLOCKS

    BLOCKS -> REGION BLOCKS

    BLOCKS -> HISTO_LIST BLOCKS

    BLOCKS -> epsilon

 */
void Parser::parse_blocks(PNode parent) {

        auto tok = lexer->peek(0); 
        switch(tok->get_token_type()) {
            // BLOCKS -> INFO BLOCKS
            case TOK_ADLINFO:
                parse_info(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> DEFINITION BLOCKS
            case TOK_DEF: 
                parse_definition(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> COMPOSITE BLOCKS
            case TOK_COMP:
                parse_composite(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> OBJECT BLOCKS
            case TOK_OBJ:
                parse_object(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> TABLE BLOCKS
            case TOK_TABLE:
                parse_table(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> REGION BLOCKS
            case TOK_REG:
                parse_region(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> HISTO_LIST BLOCKS
            case TOK_HISTOLIST:
                parse_histo_list(parent);
                parse_blocks(parent);
                return;
                
            // BLOCKS -> epsilon
            case TOK_END_OF_FILE:
                return;
            
            // If we have anything but these options and the file has not ended, this is an error state
            default:
                raise_parsing_exception("Unexpected token follows a block - expected either a continuation of the previous block or the start of a new one", tok);
                return;
        }

}


/*
INFO productions:
---

    INFO -> adlinfo ID INITIALIZATIONS

*/
void Parser::parse_info(PNode parent) {

    PNode info(create_node(AST_INFO, parent));

    // INFO -> adlinfo ID INITIALIZATIONS
    lexer->expect_and_consume(TOK_ADLINFO);
    parse_id(info);
    parse_initializations(info);
}


/* 
INITIALIZATIONS productions:
----

    INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS

    INITIALIZATIONS -> epsilon

 */
void Parser::parse_initializations(PNode parent) {

    PNode initializations = make_list_root_node(AST_INITIALIZATIONS, parent);

    PToken next = lexer->peek(0);
    
    switch (next->get_token_type()) {

        // INITIALIZATONS -> epsilon
        case TOK_ADLINFO: case TOK_DEF: case TOK_COMP: case TOK_OBJ: case TOK_TABLE: case TOK_REG: case TOK_HISTOLIST: case TOK_END_OF_FILE:
            return;
        // Anything not in the follow set indicates a continuation
        // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
        default:
            parse_initialization(initializations);
            parse_initializations(initializations);
            return;
    }
}


/* INITIALIZATION productions:
---

    INITIALIZATION -> ID ID

*/
void Parser::parse_initialization(PNode parent) {

    PNode initialization(create_node(AST_INITIALIZATION, parent));

    // assume we just want two strings or names to be an arbitrary extra info statement
    parse_id(initialization);
    parse_id(initialization);

}


/*
DEFINITION productions:
---

    DEFINITION -> def ID ASSIGNMENT DEF_RVALUE

*/
void Parser::parse_definition(PNode parent) {

    PNode definition(create_node(AST_DEFINITION, parent));

    // DEFINITION -> def ID ASSIGNMENT DEF_RVALUE
    lexer->expect_and_consume(TOK_DEF);
    parse_id(definition);
    parse_assignment();
    parse_def_rvalue(definition);

}


/* DEF_RVALUE productions:
---

    DEF_RVALUE -> external STRING

    DEF_RVALUE -> external attribute STRING

    DEF_RVALUE -> correctionlib STRING STRING

    DEF_RVALUE -> add PARTICLE_SUM

    DEF_RVALUE -> particle_keyword PARTICLE_SUM

    DEF_RVALUE -> E

 */
void Parser::parse_def_rvalue(PNode parent) {

    auto tok = lexer->peek(0);
    
    switch(tok->get_token_type()) {

        // DEF_RVALUE -> external STRING
        // DEF_RVALUE -> external attribute STRING 
        case TOK_EXTERNAL:
        {

            PNode external;

            lexer->expect_and_consume(TOK_EXTERNAL);

            if (lexer->peek(0)->get_token_type() == TOK_ATTRIBUTE) {
                lexer->expect_and_consume(TOK_ATTRIBUTE);
                external = create_node(AST_EXTERN_ATTR, parent);
            } else {
                external = create_node(AST_EXTERN_FUN, parent);
            }

            parse_string(external, "External functions must be given an explicit code string to run");
        }

        // DEF_RVALUE -> correctionlib STRING STRING

        case TOK_CORRECTIONLIB:
        {
            PNode correctionlib(create_node(AST_CORRECTIONLIB, parent));

            parse_string(correctionlib, "Correctionlib correction sets must be given an exact string for a file name");
            parse_string(correctionlib, "Correctionlib correction set includes must be given an exact string for a key");

        }

        // DEF_RVALUE -> add PARTICLE_SUM
        // DEF_RVALUE -> particle_keyword PARTICLE_SUM
        case TOK_ADD: case TOK_PARTICLE_KEYWORD:
        {   
            tok->get_token_type() == TOK_ADD ? lexer->expect_and_consume(TOK_ADD) : lexer->expect_and_consume(TOK_PARTICLE_KEYWORD);    
            parse_particle_sum(parent);
        }

        // DEF_RVALUE -> E
        default:
            // assume this is an expression if the other components have not succeeded in their production
            parse_expression(parent);
    }

}


/* COMPOSITE productions:
---

    COMPOSITE -> comp ID ASSIGNMENT COMP_RVALUE 

 */
void Parser::parse_composite(PNode parent) {

    PNode composite(create_node(AST_COMPOSITE, parent));
    
    // COMPOSITE -> comp ID ASSIGNMENT COMP_RVALUE 
    lexer->expect_and_consume(TOK_COMP);
    parse_id(composite);
    parse_assignment();
    parse_comp_rvalue(composite);

}

/* COMP_RVALUE productions:
---

    COMPOSITE_RVALUE -> COMP_TYPE ( NAMED_PARTICLE_LIST ) COMP_CRITERIA

*/

void Parser::parse_comp_rvalue(PNode parent) {
    
    // COMPOSITE_RVALUE -> COMP_TYPE ( NAMED_PARTICLE_LIST ) COMP_CRITERIA
    parse_comp_type(parent);
    lexer->expect_and_consume(TOK_OPEN_PAREN);
    parse_named_particle_list(parent);
    lexer->expect_and_consume(TOK_CLOSE_PAREN);
    parse_comp_criteria(parent);

    return;
}


/* COMPOSITE_TYPE productions:
---

    COMPOSITE_TYPE -> comb

    COMPOSITE_TYPE -> disjoint

    COMPOSITE_TYPE -> direct

*/
void Parser::parse_comp_type(PNode parent) {

    PToken tok = lexer->peek(0);
    switch(tok->get_token_type()) {

        case TOK_COMB:
            create_node(AST_COMPOSITE_CARTESIAN, parent);
            break;
        case TOK_DISJOINT:
            create_node(AST_COMPOSITE_DISJOINT, parent);
            break;
        case TOK_DIRECT:
            create_node(AST_COMPOSITE_DIRECT, parent);
            break;
        default:
            raise_parsing_exception("Invalid input to a composite statement, need either comb or disjoint", tok);
            break;    
    }
}


/* COMPOSITE_CRITERIA productions:
---

    COMPOSITE_CRITERIA -> COMPOSITE_CRITERION COMPOSITE_CRITERIA

    COMPOSITE_CRITERIA -> epsilon

*/
void Parser::parse_comp_criteria(PNode parent) {

    PNode comp_criteria = make_list_root_node(AST_COMP_CRITERIA, parent);

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        // first set of COMPOSITE_CRITERION
        // COMPOSITE_CRITERIA -> COMPOSITE_CRITERION COMPOSITE_CRITERIA
        case TOK_SELECT: case TOK_REJEC: case TOK_PARTICLE_KEYWORD:
            parse_comp_criterion(comp_criteria);
            parse_comp_criteria(comp_criteria);
            return;

        // COMPOSITE_CRITERIA -> epsilon
        default:
            return;
    }
}


/* COMPOSITE_CRITERION productions:
---

    COMPOSITE_CRITERION -> particle_keyword ID ASSIGNMENT PARTICLE_SUM

    COMPOSITE_CRITERION -> OBJ_CRITERION

 */
void Parser::parse_comp_criterion(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        case TOK_PARTICLE_KEYWORD:
        {
            PNode definition(create_node(AST_DEFINITION, parent));

            // COMPOSITE_CRITERION -> particle_keyword ID ASSIGNMENT PARTICLE_SUM
            lexer->expect_and_consume(TOK_PARTICLE_KEYWORD);
            parse_id(definition);
            parse_assignment();
            parse_particle_sum(definition);
            break;
        }
        // COMPOSITE_CRITERION -> OBJ_CRITERION
        default:
            parse_obj_criterion(parent);
            break;
    }

}


/* OBJECT productions:
---

    OBJECT -> obj ID ASSIGNMENT OBJ_RVALUE 

 */
void Parser::parse_object(PNode parent) {

    PNode object(create_node(AST_OBJECT, parent));
    
    // OBJECT -> obj ID ASSIGNMENT OBJ_RVALUE 
    lexer->expect_and_consume(TOK_OBJ);
    parse_id(object);
    parse_assignment();
    parse_obj_rvalue(object);

}


/* OBJ_RVALUE productions:
---

    OBJ_RVALUE -> OBJ_TYPE OBJ_CRITERIA

 */
void Parser::parse_obj_rvalue(PNode parent) {

    //OBJ_RVALUE -> OBJ_TYPE OBJ_CRITERIA
    parse_obj_type(parent);
    parse_obj_criteria(parent);
    
}

/* OBJ_TYPE productions:
---

    OBJ_TYPE -> union ( PARTICLE_LIST )

    OBJ_TYPE -> sort (PARTICLE, E )

    OBJ_TYPE -> sort (PARTICLE, E, ascend )

    OBJ_TYPE -> sort (PARTICLE, E, descend )

    OBJ_TYPE -> PARTICLE

 */
void Parser::parse_obj_type(PNode parent) {

    PToken tok = lexer->peek(0);
    switch(tok->get_token_type()) {

        case TOK_UNION:
        {
            PNode union_type(create_node(AST_OBJ_UNION, parent));

            // OBJ_TYPE -> union ( PARTICLE_LIST )
            lexer->expect_and_consume(TOK_UNION);
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            parse_particle_list(union_type);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return;
        }

        

        case TOK_SORT:
        {
            PNode sort(create_node(AST_OBJ_SORT, parent));

            // OBJ_TYPE -> sort ( PARTICLE , E OPTIONAL_SORT_DIR )
            lexer->expect_and_consume(TOK_SORT);
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            parse_particle(sort);
            lexer->expect_and_consume(TOK_COMMA);
            parse_expression(sort);
            parse_optional_sort_dir(sort);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return;
        }

        // OBJ_TYPE -> PARTICLE
        default:
        {
            parse_particle(parent);

            return;
        }
    }
}

/* OPTIONAL_SORT_DIR productions:
---

    OPTIONAL_SORT_DIR -> , ascend

    OPTIONAL_SORT_DIR -> , descend

    OPTIONAL_SORT_DIR -> epsilon

*/
void Parser::parse_optional_sort_dir(PNode parent) {
    
    if (lexer->peek(0)->get_token_type() == TOK_COMMA && lexer->peek(1)->get_token_type() == TOK_ASCEND) {

        create_node(AST_ASCEND, parent);

        // OPTIONAL_SORT_DIR -> , ascend
        lexer->expect_and_consume(TOK_COMMA);
        lexer->expect_and_consume(TOK_ASCEND);

    } else if (lexer->peek(0)->get_token_type() == TOK_COMMA && lexer->peek(1)->get_token_type() == TOK_DESCEND) {

        create_node(AST_DESCEND, parent);

        // OPTIONAL_SORT_DIR -> , descend
        lexer->expect_and_consume(TOK_COMMA);
        lexer->expect_and_consume(TOK_DESCEND);

    }
    else {
        // OPTIONAL_SORT_DIR -> epsilon
        return;
    }

}

/* OBJ_CRITERIA productions:
---

    OBJ_CRITERIA -> OBJ_CRITERION OBJ_CRITERIA

    OBJ_CRITERIA -> epsilon

*/
void Parser::parse_obj_criteria(PNode parent) {

    PNode obj_criteria = make_list_root_node(AST_OBJECT_CRITERIA, parent);

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        // OBJ_CRITERIA -> OBJ_CRITERION OBJ_CRITERIA
        case TOK_SELECT: case TOK_REJEC:
            parse_obj_criterion(obj_criteria);
            parse_obj_criteria(obj_criteria);
            return;
        // OBJ_CRITERIA -> epsilon
        default:
            return;
    }
}


/* OBJ_CRITERION productions:
---

    OBJ_CRITERION -> select E

    OBJ_CRITERION -> reject E

 */
void Parser::parse_obj_criterion(PNode parent) {

    PToken tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        case TOK_SELECT:
        {
            PNode select(create_node(AST_OBJECT_SELECT, parent));

            // OBJ_CRITERION -> select E
            lexer->expect_and_consume(TOK_SELECT);
            parse_expression(select);
            return;
        }
        case TOK_REJEC:
        {
            PNode reject(create_node(AST_OBJECT_REJECT, parent));

            // OBJ_CRITERION -> reject E
            lexer->expect_and_consume(TOK_REJEC);
            parse_expression(reject);
            return;
        }

        default:
            raise_parsing_exception("Invalid token for an object creation criterion", tok);
            return;
    }

}


/* TABLE productions:
---

    TABLE -> TABLE_HEADER LITERAL_NUMBER_LIST

*/
void Parser::parse_table(PNode parent) {

    PNode table(create_node(AST_TABLE_DEF, parent));

    // TABLE -> TABLE_HEADER LITERAL_NUMBER_LIST
    parse_table_header(table);
    parse_literal_number_list(table);
}


/* TABLE_HEADER productions:
---

    TABLE_HEADER -> table ID tabletype ID nvars INTEGER errors BOOL 

*/
void Parser::parse_table_header(PNode parent) {
    lexer->expect_and_consume(TOK_TABLE);
    parse_id(parent);
    lexer->expect_and_consume(TOK_TABLETYPE);
    parse_id(parent);
    lexer->expect_and_consume(TOK_NVARS);
    parse_integer(parent, "Only integers are allowed to specify NVars");
    lexer->expect_and_consume(TOK_ERRORS);
    parse_bool(parent);
}


/* REGION productions:
---    

    REGION -> reg ID REGION_COMMANDS

*/
void Parser::parse_region(PNode parent) {

    PNode region(create_node(AST_REGION, parent));

    // REGION -> reg ID REGION_COMMANDS
    lexer->expect_and_consume(TOK_REG);
    parse_id(region);
    parse_region_commands(region);
}


/* REGION_COMMANDS productions
---

    REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS

    REGION_COMMANDS -> epsilon

 */
void Parser::parse_region_commands(PNode parent) {

    PNode region_commands = make_list_root_node(AST_REGION_COMMANDS, parent);

    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {
        // first set of REGION_COMMAND
        // REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS
        case TOK_SELECT: case TOK_REJEC: case TOK_TAKE: case TOK_WEIGHT: case TOK_BIN: case TOK_BINS: case TOK_HISTO:
            parse_region_command(region_commands);
            parse_region_commands(region_commands);
            return;
        // REGION_COMMANDS -> epsilon
        default:
            return;
    }
}


/* REGION_COMMAND productions:
---

    REGION_COMMAND -> select E

    REGION_COMMAND -> reject E

    REGION_COMMAND -> take ID

    REGION_COMMAND -> weight ID E

    REGION_COMMAND -> bin E

    REGION_COMMAND -> bin named STRING E

    REGION_COMMAND -> bins E LITERAL_NUMBER_LIST

    REGION_COMMAND -> histo HISTOGRAM

    REGION_COMMAND -> histo take ID

 */
void Parser::parse_region_command(PNode parent) {
    
    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        
        case TOK_SELECT:
        {
            PNode select(create_node(AST_REGION_SELECT, parent));

            // REGION_COMMAND -> select E
            lexer->expect_and_consume(TOK_SELECT);
            parse_expression(select);
            return;
        }
        case TOK_REJEC:
        {
            PNode reject(create_node(AST_REGION_REJECT, parent));

            // REGION_COMMAND -> reject E
            lexer->expect_and_consume(TOK_REJEC);
            parse_expression(parent);
            return;
        }
        
        case TOK_TAKE:
        {
            PNode take(create_node(AST_REGION_USE, parent));

            // REGION_COMMAND -> take ID
            lexer->expect_and_consume(TOK_TAKE);
            parse_id(take);
            return;
        }
        case TOK_WEIGHT:
        {
            PNode weight(create_node(AST_REGION_WEIGHT, parent));

            // REGION_COMMAND -> weight ID E
            lexer->expect_and_consume(TOK_WEIGHT);
            parse_id(weight);
            parse_expression(weight);
            return;
        }

        case TOK_BIN:
        {
            PNode bin(create_node(AST_REGION_BIN, parent));
            PToken next = lexer->peek(1);

            if (next->get_token_type() == TOK_NAMED) {
        
                // REGION_COMMAND -> bin named STRING E
                lexer->expect_and_consume(TOK_BIN);
                lexer->expect_and_consume(TOK_NAMED);
                parse_string(bin);
                parse_expression(bin);
                return;

            } else {

                // REGION_COMMAND -> bin E
                lexer->expect_and_consume(TOK_BIN);
                parse_expression(bin);
                return;
            }

        }

        case TOK_BINS:
        {
            PNode bins(create_node(AST_REGION_BINS, parent));

            // REGION_COMMAND -> bins E LITERAL_NUMBER_LIST
            lexer->expect_and_consume(TOK_BINS);
            parse_expression(bins);
            parse_literal_number_list(bins);
        }

        case TOK_HISTO:
        {
            PToken next = lexer->peek(1);
            if (next->get_token_type() == TOK_TAKE) {

                PNode histo_use(create_node(AST_HISTO_USE, parent));

                // REGION_COMMAND -> histo take ID
                lexer->expect_and_consume(TOK_HISTO);
                lexer->expect_and_consume(TOK_TAKE);
                parse_id(histo_use);
                return;

            } else {

                PNode histo(create_node(AST_REGION_HISTOGRAM, parent));

                // REGION_COMMAND -> histo HISTOGRAM
                lexer->expect_and_consume(TOK_HISTO);
                parse_histogram(histo);
                return;

            }
        }

        default:
            raise_parsing_exception("Unexpected token in region block", tok);
    }

}


/* HISTO_LIST productions:
---    

    HISTO_LIST -> histolist ID HISTO_ENTRIES

*/
void Parser::parse_histo_list(PNode parent) {

    PNode histo_list(create_node(AST_HISTO_LIST, parent));

    // HISTO_LIST -> histolist ID HISTO_ENTRIES
    lexer->expect_and_consume(TOK_HISTOLIST);
    parse_id(histo_list);
    parse_histo_entries(histo_list);

}


/* HISTO_ENTRIES productions:
---

    HISTO_ENTRIES -> HISTO_ENTRY HISTO_ENTRIES
    
    HISTO_ENTRIES -> epsilon

*/
void Parser::parse_histo_entries(PNode parent) {

    PNode histo_entries = make_list_root_node(AST_HISTO_ENTRIES, parent);

    PToken tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        // HISTO_ENTRIES ->  HISTO_ENTRY HISTO_ENTRIES
        case TOK_HISTO: 
            parse_histo_entry(histo_entries);
            parse_histo_entries(histo_entries);
            return;

        // HISTO_ENTRIES -> epsilon
        default:
            return;
    }
}


/* HISTO_ENTRY productions:
---

    HISTO_ENTRY -> histo HISTOGRAM

*/
void Parser::parse_histo_entry(PNode parent) {

    PNode histo(create_node(AST_HISTOLIST_HISTOGRAM, parent));

    // HISTO_ENTRY -> histo HISTOGRAM
    lexer->expect_and_consume(TOK_HISTO);
    parse_histogram(histo);

}


/* HISTOGRAM productions:
---

    HISTOGRAM -> ID, STRING_LIST, BINNING, E

    HISTOGRAM -> ID, STRING_LIST, BINNING, E, BINNING, E

*/     
void Parser::parse_histogram(PNode parent) {
    // id , 
    parse_id(parent);
    lexer->expect_and_consume(TOK_COMMA);

    // STRING_LIST ,
    parse_string_list(parent);
    lexer->expect_and_consume(TOK_COMMA);

    // BINNING ,
    parse_binning(parent);
    lexer->expect_and_consume(TOK_COMMA);

    // E
    parse_expression(parent);

    // use to check if the list continues 
    auto discriminant = lexer->peek(0);

    if (discriminant->get_token_type() == TOK_COMMA) {
        lexer->expect_and_consume(TOK_COMMA);
        parse_binning(parent);
 
        lexer->expect_and_consume(TOK_COMMA);
        parse_expression(parent);
    }
}


/* BINNING productions:
---

    BINNING -> INTEGER, NUMBER, NUMBER

*/
void Parser::parse_binning(PNode parent) {

    // BINNING -> INTEGER, NUMBER, NUMBER
    parse_integer(parent, "Only literal integers are allowed to specify binning quantity on histograms");
    parse_number(parent, "Only literal numbers are allowed for the lower bound of a histogram");
    parse_number(parent, "Only literal numbers are allowed for the upper bound of a histogram");

}


/* BOOL productions:
---

    BOOL -> true

    BOOL -> false

 */
void Parser::parse_bool(PNode parent) {

    PToken tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        case TOK_TRUE:
            create_node(AST_TRUE, parent);
            return;
        case TOK_FALSE:
            create_node(AST_FALSE, parent);
            return;
        default:
            raise_parsing_exception("Excepted boolean, but token is not interpretable as a boolean", tok);
            return;
    }
}


/* ID productions:
---

    ID -> STRING

    ID -> VARNAME

 */
void Parser::parse_id(PNode parent) {

    PToken tok = lexer->peek(0);

    if (tok->get_token_type() == TOK_VARNAME) {
        // ID -> VARNAME
        parse_varname(parent);
    } else if (tok->get_token_type() == TOK_STRING) {
        // ID -> STRING
        parse_string(parent);
    } else {
        raise_parsing_exception("Invalid ID, allowed types are variable-type names and strings", tok);
    }

}


/* STRING productions
---

    STRING -> [string token]

*/
void Parser::parse_string(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // STRING -> [string token]
    lexer->expect_and_consume(TOK_STRING, error);
    make_varying_terminal(parent, tok);

}


/* VARNAME productions

    VARNAME -> [varname token]

*/
void Parser::parse_varname(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // VARNAME -> [varname token]
    lexer->expect_and_consume(TOK_VARNAME, error);
    make_varying_terminal(parent, tok);

}


/* NUMBER productions
---

    NUMBER -> INTEGER

    NUMBER -> SCIENTIFIC

    NUMBER -> DECIMAL

*/
void Parser::parse_number(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        case TOK_INTEGER: 
            parse_integer(parent, error);
            return;
        case TOK_SCIENTIFIC: 
            parse_scientific(parent, error);
            return;
        case TOK_DECIMAL:
            parse_decimal(parent, error);
            return;
        default:
            if (error == "") {
                std::stringstream error_ss;
                error_ss << "Unexpected token, expected a token of a numeric type, but got token of type " << tok->get_token_type_as_string();
                error = error_ss.str();
            }
            raise_parsing_exception(error, tok);
    }

}


/* INTEGER productions
---

    INTEGER -> [integer token]

*/
void Parser::parse_integer(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // INTEGER -> [integer token]
    lexer->expect_and_consume(TOK_INTEGER, error);
    make_varying_terminal(parent, tok);
    
}

/* SCIENTIFIC productions
---

    SCIENTIFIC -> [scientific token]

*/
void Parser::parse_scientific(PNode parent, std::string error) {
    
    PToken tok = lexer->peek(0);

    // SCIENTIFIC -> [scientific token]
    lexer->expect_and_consume(TOK_SCIENTIFIC, error);
    make_varying_terminal(parent, tok);

}


/* DECIMAL productions 
---

    DECIMAL -> [decimal token]

*/
void Parser::parse_decimal(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // DECIMAL -> [decimal token]
    lexer->expect_and_consume(TOK_DECIMAL, error);
    make_varying_terminal(parent, tok);

}



/* ASSIGNMENT productions:
---

    ASSIGNMENT -> :

    ASSIGNMENT -> =

    ASSIGNMENT -> take    
*/
void Parser::parse_assignment() {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        case TOK_COLON:
            //ASSIGNMENT -> :
            lexer->expect_and_consume(TOK_COLON);
            return;
        case TOK_ASSIGN:
            //ASSIGNMENT -> =
            lexer->expect_and_consume(TOK_ASSIGN);
            return;
        case TOK_TAKE:
            // ASSIGNMENT -> take
            lexer->expect_and_consume(TOK_TAKE);
        default:
            raise_parsing_exception("An '=', ':', or 'take', is needed for the first token of this block", tok);
    }
}


/* PARTICLE_SUM productions:
---

    PARTICLE_SUM -> PARTICLE + PARTICLE_SUM

    PARTICLE_SUM -> PARTICLE - PARTICLE_SUM

    PARTICLE_SUM -> PARTICLE 

*/
void Parser::parse_particle_sum(PNode parent) {

    PNode particle_sum = make_list_root_node(AST_PARTICLE_SUM, parent);

    parse_particle(particle_sum);
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        case TOK_PLUS:
            // PARTICLE_SUM -> PARTICLE + PARTICLE_SUM
            lexer->expect_and_consume(TOK_PLUS);
            parse_particle_sum(particle_sum);
            return;

        case TOK_MINUS:
            // PARTICLE_SUM -> PARTICLE - PARTICLE_SUM
            parse_particle_sum(particle_sum);
            return;

        default:
            // PARTICLE_SUM -> PARTICLE
            return;
    }
}


/* PARTICLE_LIST productions:
---

    PARTICLE_LIST -> PARTICLE, PARTICLE_LIST


    PARTICLE_LIST -> PARTICLE 

*/
void Parser::parse_particle_list(PNode parent) {

    PNode particle_list = make_list_root_node(AST_PARTICLE_LIST, parent);
    auto tok = lexer->peek(0);

    parse_particle(particle_list);
    switch (tok->get_token_type()) {

        // PARTICLE_LIST -> PARTICLE, PARTICLE_LIST
        case TOK_COMMA:
            lexer->expect_and_consume(TOK_COMMA);
            parse_particle_list(particle_list);
            return;

        default:
        // PARTICLE_LIST -> PARTICLE
            return;
    }
}


/* NAMED_PARTICLE_LIST productions:
---

    NAMED_PARTICLE_LIST -> PARTICLE ID, NAMED_PARTICLE_LIST


    NAMED_PARTICLE_LIST -> PARTICLE ID 

*/
void Parser::parse_named_particle_list(PNode parent) {

    PNode named_particle_list = make_list_root_node(AST_NAMED_PARTICLE_LIST, parent);
    auto tok = lexer->peek(0);

    parse_particle(parent);
    parse_id(parent);

    switch (tok->get_token_type()) {

        case TOK_COMMA:
            // NAMED_PARTICLE_LIST -> PARTICLE ID, NAMED_PARTICLE_LIST
            lexer->expect_and_consume(TOK_COMMA);
            parse_named_particle_list(parent);
            return;

        default:
            // NAMED_PARTICLE_LIST -> PARTICLE ID
            return;
    }
}


/* LITERAL_NUMBER_LIST productions:
---

    LITERAL_NUMBER_LIST -> NUMBER LITERAL_NUMBER_LIST

    LITERAL_NUMBER_LIST -> NUMBER

 */
void Parser::parse_literal_number_list(PNode parent) {

    PNode literal_number_list = make_list_root_node(AST_LITERAL_NUMBER_LIST, parent);

    // LITERAL_NUMBER_LIST -> NUMBER LITERAL_NUMBER_LIST
    // LITERAL_NUMBER_LIST -> NUMBER
    parse_number(literal_number_list, "Needs a literal numerical value for this partition");

    auto tok = lexer->peek(0);
    if (is_numerical(tok->get_token_type())) {
        parse_literal_number_list(literal_number_list);
    }
}


/* STRING_LIST productions:
---

    STRING_LIST -> STRING STRING_LIST

    STRING_LIST -> STRING

 */
void Parser::parse_string_list(PNode parent) {

    PNode string_list = make_list_root_node(AST_STRING_LIST, parent);

    parse_string(string_list, "Excepted string for description");

    // STRING_LIST -> STRING STRING_LIST
    auto tok = lexer->peek(0);
    if (tok->get_token_type() == TOK_STRING) {
        parse_string_list(string_list);
    }
}


/* VARIABLE_LIST productions:
---

    VARIABLE_LIST -> E, VARIABLE_LIST

    VARIABLE_LIST -> E

    VARIABLE_LIST -> epsilon

 */
void Parser::parse_variable_list(PNode parent) {

    PNode variable_list = make_list_root_node(AST_VARIABLE_LIST, parent);

    PToken tok = lexer->peek(0);
    switch(tok->get_token_type()) {

        // VARIABLE_LIST -> epsilon 
        // this is the follow set for VARIABLE_LIST, and none of them are in the first set of EXPRESSION
        case TOK_CLOSE_CURLY_BRACE: case TOK_CLOSE_PAREN:
            return;            

        // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
        // VARIABLE_LIST -> EXPRESSION
        default:
        {
            parse_expression(variable_list);
            auto next = lexer->peek(0);

            // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
            if (next->get_token_type() == TOK_COMMA) {
                lexer->expect_and_consume(TOK_COMMA);
                parse_variable_list(variable_list);
                return;
            }
            // VARIABLE_LIST -> EXPRESSION
            return;
        }
    }
}


/* PARTICLE productions:
---

    PARTICLE -> this

    PARTICLE -> ID arrow_index ID

    PARTICLE -> ID arrow_index ID [INDEX]

    PARTICLE -> ID
    
    PARTICLE -> ID [INDEX]
 */
void Parser::parse_particle(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        
        case TOK_THIS:
        {   
            // PARTICLE -> this
            lexer->expect_and_consume(TOK_THIS);
            create_node(AST_THIS, parent);
            return;
        }


        // PARTICLE -> ID arrow_index ID
        // PARTICLE -> ID arrow_index ID [INDEX]
        // PARTICLE -> ID
        // PARTICLE -> ID [INDEX]
        default:
        {
            PToken next = lexer->peek(1);
            PNode root_node;
            if (next->get_token_type() == TOK_OPEN_SQUARE_BRACE || lexer->peek(3)->get_token_type() == TOK_OPEN_SQUARE_BRACE) {
                root_node = create_node(AST_INDEX_OPERATOR, parent);
            } else {
                root_node = parent;
            }

            if (next->get_token_type() == TOK_ARROW_INDEX) {
                PNode arrow = make_operator_terminal(root_node, next);
                // ID arrow_index ID
                parse_id(arrow);
                lexer->expect_and_consume(TOK_ARROW_INDEX);
                parse_id(arrow);
            } else {
                parse_id(root_node);
            }
            if  (lexer->peek(0)->get_token_type() == TOK_OPEN_SQUARE_BRACE) {
                
                // PARTICLE -> ID arrow_index ID [INDEX]
                // PARTICLE -> ID [INDEX]
                lexer->expect_and_consume(TOK_OPEN_SQUARE_BRACE);
                parse_index(root_node);
                lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
            }
            return;
        }
    }

}


/* INDEX productions:
---

    INDEX -> INTEGER

    INDEX -> INTEGER:INTEGER

    INDEX -> :INTEGER

    INDEX -> INTEGER:
    
 */
void Parser::parse_index(PNode parent) {

    // INDEX -> INTEGER
    // INDEX -> INTEGER:INTEGER
    // INDEX -> :INTEGER
    // INDEX -> INTEGER:
    
    PNode index(create_node(AST_INDEX, parent));

    PToken tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        case TOK_INTEGER:
        {
            // INTEGER
            parse_integer(index);

            PToken next = lexer->peek(0);
            if (next->get_token_type() == TOK_COLON) {
                // : 
                lexer->expect_and_consume(TOK_COLON);
                PToken next2 = lexer->peek(0);
                if (next2->get_token_type() == TOK_INTEGER) {
                    // INDEX -> INTEGER : INTEGER
                    parse_integer(index);
                } else {
                    // INDEX -> INTEGER :
                    create_node(AST_UNBOUNDED, index, next);
                }
            }
            return;
        }
        case TOK_COLON:
            create_node(AST_UNBOUNDED, index, tok);

            // INDEX -> : INTEGER
            lexer->expect_and_consume(TOK_COLON);
            parse_integer(index);
            return;
        default:
            raise_parsing_exception("Only integers are allowed to be used as indices", tok);
    }

}


int get_precedence(PToken tok, bool increase_if_left_associative = false) {

    int left_associative_addition = increase_if_left_associative ? 1 : 0;
    
    switch(tok->get_token_type()) {

        // highest priority is an indexing of the form composite->subvariable
        // E -> E arrow_index E
        case TOK_ARROW_INDEX:
        return 110 + left_associative_addition;

        // second-highest priority is an indexing of the form object.function
        // E -> E.E
        case TOK_DOT_INDEX:
        return 100 + left_associative_addition;

        // third-highest priority is the indexing operation
        // E -> E [INDEX]
        case TOK_OPEN_SQUARE_BRACE:
        return 95 + left_associative_addition;

        // raising to a power is right-associative
        case TOK_RAISED_TO_POWER:
        return 90;

        // arithmetic multiplication and division
        case TOK_MULTIPLY: case TOK_DIVIDE:
        return 80 + left_associative_addition;

        // arithmetic addition and subtraction
        case TOK_PLUS: case TOK_MINUS:
        return 70 + left_associative_addition;

        // the explicit within and outside interval operators
        case TOK_WITHIN: case TOK_OUTSIDE:
        return 40 + left_associative_addition;

        // bitwise and/or - note that this priority is not where it is in C
        case TOK_AMPERSAND: case TOK_PIPE:
        return 30 + left_associative_addition;

        // numeric comparators
        case TOK_LT: case TOK_GT: case TOK_LE: case TOK_GE: case TOK_EQ: case TOK_NE: case TOK_ASSIGN: 
        return 20 + left_associative_addition;

        // logical comparators
        case TOK_AND: case TOK_OR:
        return 10 + left_associative_addition;

        // ternary operator parses as E -> E : E
        // it is also right-associative
        case TOK_QUESTION:
        return 5;

        default:
        return -20;


    }
}

/* E productions:
---

    E -> E'

    E -> E arrow_index E

    E -> E . E

    E -> E [INDEX]

    E -> E ^ E


    E -> E * E

    E -> E / E


    E -> E + E

    E -> E - E


    E -> E within E

    E -> E outside E


    E -> E & E

    E -> E | E


    E -> E < E

    E -> E > E

    E -> E <= E
    
    E -> E >= E

    E -> E == E

    E -> E = E

    E -> E != E


    E -> E and E

    E -> E or E


    E -> E ? E : E

 */
PNode Parser::precedence_climber(PNode parent, int min_precedence) {

    auto lhs = parse_primary_expression(parent);

    PToken next_op = lexer->peek(0);
    
    PNode op_node;

    // keep going while the next token is an operator with at least our current level of precedence
    while (get_precedence(next_op) >= min_precedence) {

        // E -> E[INDEX]
        if (next_op->get_token_type() == TOK_OPEN_SQUARE_BRACE) {
            PNode indexing(create_node(AST_INDEX_OPERATOR, parent));
            indexing->add_child(lhs);
            lhs->set_parent(indexing);
            parse_index(indexing);
            lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
            return indexing;
        } else if (next_op->get_token_type() == TOK_QUESTION) {
            PNode if_statement(create_node(AST_IF_STATEMENT, parent));
            if_statement->add_child(lhs);
            lhs->set_parent(if_statement);
            if_statement->add_child(precedence_climber(if_statement, 0));
            lexer->expect_and_consume(TOK_COLON);
            if_statement->add_child(precedence_climber(if_statement, 0));
            return if_statement;
        }

        op_node = make_lost_operator_terminal(parent, next_op);

        // find what precedence is our new minimum - if the operator is left-associative, it is one more than its normal precedence
        int new_min_precedence = get_precedence(next_op, true);

        auto rhs = precedence_climber(op_node, new_min_precedence);

        op_node->add_child(lhs);
        lhs->set_parent(op_node);

        op_node->add_child(rhs);

        lhs = op_node;

        next_op = lexer->peek(0);
    }
        
    // at this point, we have parsed all we can of precedences above our threshold. We give our final node of the loop
    return lhs;
}

#define CASE_BUILT_IN_MATH_FUN TOK_ANYOF: case TOK_ALLOF: case TOK_SQRT: case TOK_ABS: case TOK_COS:  case TOK_SIN: case TOK_TAN: case TOK_SINH: case TOK_COSH: case TOK_TANH: case TOK_EXP: case TOK_LOG: case TOK_AVE: case TOK_SUM

#define CASE_BUILT_IN_PARTICLE_FUN TOK_LETTER_E: case TOK_LETTER_P: case TOK_LETTER_M: case TOK_LETTER_Q: \
case TOK_CHARGE: case TOK_MASS: case TOK_PHI: case TOK_ETA: case TOK_PT: \
case TOK_DR: case TOK_DPHI: case TOK_DETA: \
case TOK_DR_HADAMARD: case TOK_DPHI_HADAMARD: case TOK_DETA_HADAMARD: \
case TOK_NUMOF: case TOK_DISTINCT


/* E' productions:
---

    E' -> 
    
 */
PNode Parser::parse_primary_expression(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {


        // E' -> this

        // particles are allowed in expressions since user defined functions may use them - this is included in that
        case TOK_THIS:
        {
            return create_lost_node(AST_THIS, parent, tok);
        }

        // E' -> (E)
        case TOK_OPEN_PAREN:
        {
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            PNode subexpression = precedence_climber(parent, 0);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return subexpression;
        }

        // E' -> {VARIABLE_LIST}
        case TOK_OPEN_CURLY_BRACE:
        {
            PNode varlist = create_lost_node(AST_VARIABLE_LIST, parent);

            lexer->expect_and_consume(TOK_OPEN_CURLY_BRACE);
            parse_variable_list(varlist);
            lexer->expect_and_consume(TOK_CLOSE_CURLY_BRACE);
            return varlist;
        }

        // E' -> [E, E]
        case TOK_OPEN_SQUARE_BRACE:
        {
            PNode interval(create_lost_node(AST_INTERVAL, parent)); 
            interval->add_child(precedence_climber(interval, 0));
            if (lexer->peek(0)->get_token_type() == TOK_COMMA) lexer->expect_and_consume(TOK_COMMA);
            interval->add_child(precedence_climber(interval, 0));
            lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
            return interval; 

        }

        case TOK_SORT:
        {

            PNode sort_expr(create_lost_node(AST_SORT_EXPRESSION, parent));
            // E' -> sort (E OPTIONAL_SORT_DIR)
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            sort_expr->add_child(precedence_climber(parent, 0));
            parse_optional_sort_dir(sort_expr);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            
            return sort_expr;
        }

        
        // E' -> min (VARIABLE_LIST)
        // E' -> max (VARIABLE_LIST)
        case TOK_MIN: case TOK_MAX: 
        {
            PNode minmax(create_lost_node(tok->get_token_type() == TOK_MIN ? AST_MIN_EXPRESSION : AST_MAX_EXPRESSION, parent));

            lexer->expect_and_consume(TOK_OPEN_PAREN);
            parse_variable_list(minmax);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            return minmax;
        }

        // E' -> BUILT_IN_MATHEMATIC_FUN
        // E' -> BUILT_IN_MATHEMATIC_FUN (E)
        case CASE_BUILT_IN_MATH_FUN:
        {
            PNode mathfun(make_lost_builtin_func_terminal(parent, tok));

            lexer->expect_and_consume(tok->get_token_type());

            if (lexer->peek(0)->get_token_type() == TOK_OPEN_PAREN) {

                lexer->expect_and_consume(TOK_OPEN_PAREN);
                mathfun->add_child(precedence_climber(parent, 0));
                lexer->expect_and_consume(TOK_CLOSE_PAREN);            
            }

            return mathfun;
        }
        
        // Functions which take a particle as an argument
        // E' -> BUILT_IN_PARTICLE_FUN
        // E' -> BUILT_IN_PARTICLE_FUN (PARTICLE_LIST)
        case CASE_BUILT_IN_PARTICLE_FUN:
        {

            PNode partfun(make_lost_builtin_func_terminal(parent, tok));

            lexer->expect_and_consume(tok->get_token_type());

            if (lexer->peek(0)->get_token_type() == TOK_OPEN_PAREN) {

                lexer->expect_and_consume(TOK_OPEN_PAREN);
                parse_particle_list(partfun);
                lexer->expect_and_consume(TOK_CLOSE_PAREN);            
            }


            return partfun;
        }
        


        // E -> ID
        // E -> ID (VARIABLE_LIST)
        case TOK_STRING: case TOK_VARNAME:
        {
            PNode name(make_lost_varying_terminal(parent, tok));
            // here, we are met with a token that isn't any other known form. If it is immediately followed by parentheses, then this is probably some external function. 
            if (lexer->peek(1)->get_token_type() == TOK_OPEN_PAREN) {
                PNode func(create_node(AST_USER_FUNCTION, parent));
                func->add_child(name);
                name->set_parent(func);

                lexer->expect_and_consume(TOK_OPEN_PAREN);
                parse_variable_list(func);
                lexer->expect_and_consume(TOK_CLOSE_PAREN);
                return func;
            }
            
            // otherwise, it is unclear what this is other than just some variable name - we will leave it like that
            return name;
        }


        // E' -> - E
        case TOK_MINUS: 
        {
            PNode negate_node(create_lost_node(AST_NEGATE, parent));
            // precedence of negation should be stronger than multiplication but weaker than power
            negate_node->add_child(precedence_climber(negate_node, 85));
            return negate_node;
        }
        // E' -> not E
        case TOK_NOT:
        {   
            PNode not_node(create_lost_node(AST_L_NOT, parent));
            // precedence of the logical not should be higher than the other logical operations but lower than comparison
            not_node->add_child(precedence_climber(not_node, 15));
            return not_node;
        }

        // E -> NUMBER
        default:
            if(!is_numerical(tok->get_token_type())) raise_parsing_exception("Invalid token used in expression", tok);
            return create_node(AST_ERROR, parent);
    }
}

// helper to create an AST node for an E
PNode Parser::parse_expression(PNode parent) {
    
    PNode expression(create_node(AST_EXPRESSION, parent));
    expression->add_child(precedence_climber(expression, 0));

    return expression;
}

void Parser::print_children_and_yourself(PNode node, int *top_number) {

    int reserved_number_for_me = (*top_number)++;

    if (node->has_token()) {
        std::string lexeme(node->get_token()->get_lexeme());
        std::regex quotes = std::regex("\"+");
        lexeme = std::regex_replace(lexeme, quotes, "");

        std::cout << "    " << reserved_number_for_me << " [label=\"" << lexeme << "\"]" << std::endl;
    } else {
        std::cout << "    " << reserved_number_for_me << " [label=\"ID:" << node->get_ast_type_as_string() << "\"]" <<std::endl;
    }

    auto children_vector = node->get_children();
    for (auto it = children_vector.begin(); it != children_vector.end(); ++it) {
        std::cout << "    " << reserved_number_for_me << " -> " << *top_number << std::endl;
        print_children_and_yourself(*it, top_number);
    }
}

void Parser::print_parse_dot() {
    PNode input_node = tree.get_root();

    std::cout << "digraph G {" << std::endl;

    int top = 1;

    print_children_and_yourself(input_node, &top);

    std::cout << "}" << std::endl;
}

PNode Parser::get_root() {
    return tree.get_root();
}
