#include "parser.hpp"
#include <memory>

#include <iostream>
#include <sstream>

#include "exceptions.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"



PNode Parser::create_node(AST_type in, PNode parent, PToken tok) {
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
PNode Parser::create_node(AST_type in, PNode parent) {
    PToken tok = lexer->peek(0);
    return create_node(in, parent, tok);
}

PNode Parser::create_lost_node(AST_type in, PNode parent, PToken tok) {
    return std::make_shared<Node>(in, parent, tok);
}

PNode Parser::create_lost_node(AST_type in, PNode parent) {
    PToken tok = lexer->peek(0);
    return create_lost_node(in, parent, tok);
}

/**
 * @brief Gets or creates the root node of a list
 * 
 * @param in: type of the AST node list root
 * @param parent: parent of the node
 * @return: PNode, either the parent if it already matches the type, or a newly created node
 */
PNode Parser::make_list_root_node(AST_type in, PNode parent){
    if (parent->get_ast_type() == in) return parent;
    return create_node(in, parent);
}

bool is_numerical(Token_type t) {
    if (t == TOK::INTEGER || t == TOK::DECIMAL || t == TOK::SCIENTIFIC) return true;
    return false;
} 

Parser::Parser (Lexer *lex): lexer(lex), tree(AST::INPUT) {
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
            case TOK::ADLINFO:
                parse_info(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> DEFINITION BLOCKS
            case TOK::DEF: 
                parse_definition(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> COMPOSITE BLOCKS
            case TOK::COMP:
                parse_composite(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> OBJECT BLOCKS
            case TOK::OBJ:
                parse_object(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> TABLE BLOCKS
            case TOK::TABLE:
                parse_table(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> REGION BLOCKS
            case TOK::REG:
                parse_region(parent);
                parse_blocks(parent);
                return;

            // BLOCKS -> HISTO_LIST BLOCKS
            case TOK::HISTOLIST:
                parse_histo_list(parent);
                parse_blocks(parent);
                return;
                
            // BLOCKS -> epsilon
            case TOK::END_OF_FILE:
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

    PNode info(create_node(AST::INFO, parent));

    // INFO -> adlinfo ID INITIALIZATIONS
    lexer->expect_and_consume(TOK::ADLINFO);
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

    PNode initializations = make_list_root_node(AST::INITIALIZATIONS, parent);

    PToken next = lexer->peek(0);
    
    switch (next->get_token_type()) {

        // INITIALIZATONS -> epsilon
        case TOK::ADLINFO: case TOK::DEF: case TOK::COMP: case TOK::OBJ: case TOK::TABLE: case TOK::REG: case TOK::HISTOLIST: case TOK::END_OF_FILE:
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

    PNode initialization(create_node(AST::INITIALIZATION, parent));

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

    PNode definition(create_node(AST::DEFINITION, parent));

    // DEFINITION -> def ID ASSIGNMENT DEF_RVALUE
    lexer->expect_and_consume(TOK::DEF);
    parse_id(definition);
    parse_assignment();
    parse_def_rvalue(definition);

}


/* DEF_RVALUE productions:
---

    DEF_RVALUE -> external STRING

    DEF_RVALUE -> external attribute STRING

    DEF_RVALUE -> external particle STRING

    DEF_RVALUE -> correctionlib STRING STRING

    DEF_RVALUE -> add PARTICLE_SUM

    DEF_RVALUE -> particle_keyword PARTICLE_SUM

    DEF_RVALUE -> E

 */
void Parser::parse_def_rvalue(PNode parent) {

    auto tok = lexer->peek(0);
    
    switch(tok->get_token_type()) {

        // DEF_RVALUE -> external STRING
        // DEF_RVALUE -> external particle STRING
        // DEF_RVALUE -> external attribute STRING 
        case TOK::EXTERNAL:
        {

            PNode external;

            lexer->expect_and_consume(TOK::EXTERNAL);

            if (lexer->peek(0)->get_token_type() == TOK::ATTRIBUTE) {
                lexer->expect_and_consume(TOK::ATTRIBUTE);
                external = create_node(AST::EXTERN_ATTR, parent, tok);
            } else if (lexer->peek(0)->get_token_type() == TOK::PARTICLE_KEYWORD) {
                lexer->expect_and_consume(TOK::PARTICLE_KEYWORD);
                external = create_node(AST::EXTERN_PARTICLE, parent, tok);
            } else {
                external = create_node(AST::EXTERN_FUN, parent, tok);
            }

            parse_string(external, "External functions must be given an explicit code string to run");
        } break;

        // DEF_RVALUE -> correctionlib STRING STRING

        case TOK::CORRECTIONLIB:
        {
            PNode correctionlib(create_node(AST::CORRECTIONLIB, parent, tok));

            lexer->expect_and_consume(TOK::CORRECTIONLIB);
            
            parse_string(correctionlib, "Correctionlib correction sets must be given an exact string for a file name");
            parse_string(correctionlib, "Correctionlib correction set includes must be given an exact string for a key");

        } break;

        // DEF_RVALUE -> add PARTICLE_SUM
        // DEF_RVALUE -> particle_keyword PARTICLE_SUM
        case TOK::ADD: case TOK::PARTICLE_KEYWORD:
        {   
            tok->get_token_type() == TOK::ADD ? lexer->expect_and_consume(TOK::ADD) : lexer->expect_and_consume(TOK::PARTICLE_KEYWORD);    
            parse_particle_sum(parent);
        } break;

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

    PNode composite(create_node(AST::COMPOSITE, parent));
    
    // COMPOSITE -> comp ID ASSIGNMENT COMP_RVALUE 
    lexer->expect_and_consume(TOK::COMP);
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
    lexer->expect_and_consume(TOK::OPEN_PAREN);
    parse_named_particle_list(parent);
    lexer->expect_and_consume(TOK::CLOSE_PAREN);
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

        case TOK::COMB:
            // COMPOSITE_TYPE -> comb
            lexer->expect_and_consume(TOK::COMB);
            create_node(AST::COMPOSITE_CARTESIAN, parent, tok);
            break;
        case TOK::DISJOINT:
            // COMPOSITE_TYPE -> disjoint
            lexer->expect_and_consume(TOK::DISJOINT);
            create_node(AST::COMPOSITE_DISJOINT, parent, tok);
            break;
        case TOK::DIRECT:
            // COMPOSITE_TYPE -> direct
            lexer->expect_and_consume(TOK::DIRECT);
            create_node(AST::COMPOSITE_DIRECT, parent, tok);
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

    PNode comp_criteria = make_list_root_node(AST::COMP_CRITERIA, parent);

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        // first set of COMPOSITE_CRITERION
        // COMPOSITE_CRITERIA -> COMPOSITE_CRITERION COMPOSITE_CRITERIA
        case TOK::SELECT: case TOK::REJEC: case TOK::PARTICLE_KEYWORD:
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

        case TOK::PARTICLE_KEYWORD:
        {
            PNode definition(create_node(AST::DEFINITION, parent, tok));

            // COMPOSITE_CRITERION -> particle_keyword ID ASSIGNMENT PARTICLE_SUM
            lexer->expect_and_consume(TOK::PARTICLE_KEYWORD);
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

    PNode object(create_node(AST::OBJECT, parent));
    
    // OBJECT -> obj ID ASSIGNMENT OBJ_RVALUE 
    lexer->expect_and_consume(TOK::OBJ);
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

        case TOK::UNION:
        {
            PNode union_type(create_node(AST::OBJ_UNION, parent, tok));

            // OBJ_TYPE -> union ( PARTICLE_LIST )
            lexer->expect_and_consume(TOK::UNION);
            lexer->expect_and_consume(TOK::OPEN_PAREN);
            parse_particle_list(union_type);
            lexer->expect_and_consume(TOK::CLOSE_PAREN);

            return;
        }

        

        case TOK::SORT:
        {
            PNode sort(create_node(AST::OBJ_SORT, parent, tok));

            // OBJ_TYPE -> sort ( PARTICLE , E OPTIONAL_SORT_DIR )
            lexer->expect_and_consume(TOK::SORT);
            lexer->expect_and_consume(TOK::OPEN_PAREN);
            parse_particle(sort);
            lexer->expect_and_consume(TOK::COMMA);
            parse_expression(sort);
            parse_optional_sort_dir(sort);
            lexer->expect_and_consume(TOK::CLOSE_PAREN);

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
    
    if (lexer->peek(0)->get_token_type() == TOK::COMMA && lexer->peek(1)->get_token_type() == TOK::ASCEND) {

        create_node(AST::ASCEND, parent, lexer->peek(1));

        // OPTIONAL_SORT_DIR -> , ascend
        lexer->expect_and_consume(TOK::COMMA);
        lexer->expect_and_consume(TOK::ASCEND);

    } else if (lexer->peek(0)->get_token_type() == TOK::COMMA && lexer->peek(1)->get_token_type() == TOK::DESCEND) {

        create_node(AST::DESCEND, parent, lexer->peek(1));

        // OPTIONAL_SORT_DIR -> , descend
        lexer->expect_and_consume(TOK::COMMA);
        lexer->expect_and_consume(TOK::DESCEND);

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

    PNode obj_criteria = make_list_root_node(AST::OBJECT_CRITERIA, parent);

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        // OBJ_CRITERIA -> OBJ_CRITERION OBJ_CRITERIA
        case TOK::SELECT: case TOK::REJEC:
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

        case TOK::SELECT:
        {
            PNode select(create_node(AST::OBJECT_SELECT, parent, tok));

            // OBJ_CRITERION -> select E
            lexer->expect_and_consume(TOK::SELECT);
            parse_expression(select);
            return;
        }
        case TOK::REJEC:
        {
            PNode reject(create_node(AST::OBJECT_REJECT, parent, tok));

            // OBJ_CRITERION -> reject E
            lexer->expect_and_consume(TOK::REJEC);
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

    PNode table(create_node(AST::TABLE_DEF, parent));

    // TABLE -> TABLE_HEADER LITERAL_NUMBER_LIST
    parse_table_header(table);
    parse_literal_number_list(table);
}


/* TABLE_HEADER productions:
---

    TABLE_HEADER -> table ID tabletype ID nvars INTEGER errors BOOL 

*/
void Parser::parse_table_header(PNode parent) {
    lexer->expect_and_consume(TOK::TABLE);
    parse_id(parent);
    lexer->expect_and_consume(TOK::TABLETYPE);
    parse_id(parent);
    lexer->expect_and_consume(TOK::NVARS);
    parse_integer(parent, "Only integers are allowed to specify NVars");
    lexer->expect_and_consume(TOK::ERRORS);
    parse_bool(parent);
}


/* REGION productions:
---    

    REGION -> reg ID REGION_COMMANDS

*/
void Parser::parse_region(PNode parent) {

    PNode region(create_node(AST::REGION, parent));

    // REGION -> reg ID REGION_COMMANDS
    lexer->expect_and_consume(TOK::REG);
    parse_id(region);
    parse_region_commands(region);
}


/* REGION_COMMANDS productions
---

    REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS

    REGION_COMMANDS -> epsilon

 */
void Parser::parse_region_commands(PNode parent) {

    PNode region_commands = make_list_root_node(AST::REGION_COMMANDS, parent);

    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {
        // first set of REGION_COMMAND
        // REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS
        case TOK::SELECT: case TOK::REJEC: case TOK::TAKE: case TOK::WEIGHT: case TOK::BIN: case TOK::BINS: case TOK::HISTO:
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
        
        case TOK::SELECT:
        {
            PNode select(create_node(AST::REGION_SELECT, parent, tok));

            // REGION_COMMAND -> select E
            lexer->expect_and_consume(TOK::SELECT);
            parse_expression(select);
            return;
        }
        case TOK::REJEC:
        {
            PNode reject(create_node(AST::REGION_REJECT, parent, tok));

            // REGION_COMMAND -> reject E
            lexer->expect_and_consume(TOK::REJEC);
            parse_expression(parent);
            return;
        }
        
        case TOK::TAKE:
        {
            PNode take(create_node(AST::REGION_USE, parent, tok));

            // REGION_COMMAND -> take ID
            lexer->expect_and_consume(TOK::TAKE);
            parse_id(take);
            return;
        }
        case TOK::WEIGHT:
        {
            PNode weight(create_node(AST::REGION_WEIGHT, parent, tok));

            // REGION_COMMAND -> weight ID E
            lexer->expect_and_consume(TOK::WEIGHT);
            parse_id(weight);
            parse_expression(weight);
            return;
        }

        case TOK::BIN:
        {
            PNode bin(create_node(AST::REGION_BIN, parent, tok));
            PToken next = lexer->peek(1);

            if (next->get_token_type() == TOK::NAMED) {
        
                // REGION_COMMAND -> bin named STRING E
                lexer->expect_and_consume(TOK::BIN);
                lexer->expect_and_consume(TOK::NAMED);
                parse_string(bin);
                parse_expression(bin);
                return;

            } else {

                // REGION_COMMAND -> bin E
                lexer->expect_and_consume(TOK::BIN);
                parse_expression(bin);
                return;
            }

        }

        case TOK::BINS:
        {
            PNode bins(create_node(AST::REGION_BINS, parent, tok));

            // REGION_COMMAND -> bins E LITERAL_NUMBER_LIST
            lexer->expect_and_consume(TOK::BINS);
            parse_expression(bins);
            parse_literal_number_list(bins);
            return;
        }

        case TOK::HISTO:
        {
            PToken next = lexer->peek(1);
            if (next->get_token_type() == TOK::TAKE) {

                PNode histo_use(create_node(AST::REGION_HISTO_USE, parent, tok));

                // REGION_COMMAND -> histo take ID
                lexer->expect_and_consume(TOK::HISTO);
                lexer->expect_and_consume(TOK::TAKE);
                parse_id(histo_use);
                return;

            } else {

                PNode histo(create_node(AST::REGION_HISTOGRAM, parent, tok));

                // REGION_COMMAND -> histo HISTOGRAM
                lexer->expect_and_consume(TOK::HISTO);
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

    PNode histo_list(create_node(AST::HISTO_LIST, parent));

    // HISTO_LIST -> histolist ID HISTO_ENTRIES
    lexer->expect_and_consume(TOK::HISTOLIST);
    parse_id(histo_list);
    parse_histo_entries(histo_list);

}


/* HISTO_ENTRIES productions:
---

    HISTO_ENTRIES -> HISTO_ENTRY HISTO_ENTRIES
    
    HISTO_ENTRIES -> epsilon

*/
void Parser::parse_histo_entries(PNode parent) {

    PNode histo_entries = make_list_root_node(AST::HISTO_ENTRIES, parent);

    PToken tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        // HISTO_ENTRIES ->  HISTO_ENTRY HISTO_ENTRIES
        case TOK::HISTO: 
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

    PNode histo(create_node(AST::HISTOLIST_HISTOGRAM, parent));

    // HISTO_ENTRY -> histo HISTOGRAM
    lexer->expect_and_consume(TOK::HISTO);
    parse_histogram(histo);

}


/* HISTOGRAM productions:
---

    HISTOGRAM -> ID, STRING_LIST, BINNING, E

    HISTOGRAM -> ID, STRING_LIST, BINNING, E, BINNING, E

*/     
void Parser::parse_histogram(PNode parent) {
    PNode histogram = create_node(AST::HISTOGRAM, parent, lexer->peek(0));

    // id , 
    parse_id(histogram);
    lexer->expect_and_consume(TOK::COMMA);

    // STRING_LIST ,
    parse_string_list(histogram);
    lexer->expect_and_consume(TOK::COMMA);

    // BINNING ,
    parse_binning(histogram);
    lexer->expect_and_consume(TOK::COMMA);

    // E
    parse_expression(histogram);

    // use to check if the list continues 
    auto discriminant = lexer->peek(0);

    if (discriminant->get_token_type() == TOK::COMMA) {
        lexer->expect_and_consume(TOK::COMMA);
        parse_binning(histogram);
 
        lexer->expect_and_consume(TOK::COMMA);
        parse_expression(histogram);
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
        case TOK::TRUE:
            create_node(AST::TRUE, parent, tok);
            return;
        case TOK::FALSE:
            create_node(AST::FALSE, parent, tok);
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

    if (tok->get_token_type() == TOK::VARNAME) {
        // ID -> VARNAME
        parse_varname(parent);
    } else if (tok->get_token_type() == TOK::STRING) {
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
    lexer->expect_and_consume(TOK::STRING, error);
    create_node(AST::VARYING_TERMINAL, parent, tok);

}


/* VARNAME productions

    VARNAME -> [varname token]

*/
void Parser::parse_varname(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // VARNAME -> [varname token]
    lexer->expect_and_consume(TOK::VARNAME, error);
    create_node(AST::VARYING_TERMINAL, parent, tok);

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
        case TOK::INTEGER: 
            parse_integer(parent, error);
            return;
        case TOK::SCIENTIFIC: 
            parse_scientific(parent, error);
            return;
        case TOK::DECIMAL:
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
    lexer->expect_and_consume(TOK::INTEGER, error);
    create_node(AST::VARYING_TERMINAL, parent, tok);
    
}

/* SCIENTIFIC productions
---

    SCIENTIFIC -> [scientific token]

*/
void Parser::parse_scientific(PNode parent, std::string error) {
    
    PToken tok = lexer->peek(0);

    // SCIENTIFIC -> [scientific token]
    lexer->expect_and_consume(TOK::SCIENTIFIC, error);
    create_node(AST::VARYING_TERMINAL, parent, tok);

}


/* DECIMAL productions 
---

    DECIMAL -> [decimal token]

*/
void Parser::parse_decimal(PNode parent, std::string error) {

    PToken tok = lexer->peek(0);

    // DECIMAL -> [decimal token]
    lexer->expect_and_consume(TOK::DECIMAL, error);
    create_node(AST::VARYING_TERMINAL, parent, tok);

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
        case TOK::COLON:
            //ASSIGNMENT -> :
            lexer->expect_and_consume(TOK::COLON);
            return;
        case TOK::ASSIGN:
            //ASSIGNMENT -> =
            lexer->expect_and_consume(TOK::ASSIGN);
            return;
        case TOK::TAKE:
            // ASSIGNMENT -> take
            lexer->expect_and_consume(TOK::TAKE);
            return;
        default:
            raise_parsing_exception("An '=', ':', or 'take', is needed for the first token of this block", tok);
    }
}


/* PARTICLE_SUM productions:
---

    PARTICLE_SUM -> PARTICLE PARTICLE_SUM_TAIL

    PARTICLE_SUM -> - PARTICLE_SUM_TAIL

*/
void Parser::parse_particle_sum(PNode parent) {

    PNode particle_sum = make_list_root_node(AST::PARTICLE_SUM, parent);

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        case TOK::MINUS:
        {
            // PARTICLE_SUM -> - PARTICLE PARTICLE_SUM_TAIL
            lexer->expect_and_consume(TOK::MINUS);

            PNode negate(create_node(AST::PARTICLE_NEGATE, particle_sum));
            parse_particle(negate);

            parse_particle_sum_tail(particle_sum);

            return;
        }
        default:
            // PARTICLE_SUM -> PARTICLE PARTICLE_SUM_TAIL
            parse_particle(particle_sum);
            parse_particle_sum_tail(particle_sum);
            return;

    }
}


/* PARTICLE_SUM_TAIL productions:

    PARTICLE_SUM_TAIL -> + PARTICLE PARTICLE_SUM_TAIL
    
    PARTICLE_SUM_TAIL -> - PARTICLE PARTICLE_SUM_TAIL

    PARTICLE_SUM_TAIL -> epsilon

*/
void Parser::parse_particle_sum_tail(PNode parent) {
    PNode particle_sum = make_list_root_node(AST::PARTICLE_SUM, parent);

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        
        case TOK::PLUS:
            // PARTICLE_SUM_TAIL -> + PARTICLE PARTICLE_SUM_TAIL
            lexer->expect_and_consume(TOK::PLUS);
            parse_particle(particle_sum);

            parse_particle_sum_tail(particle_sum);

            return;

        case TOK::MINUS:
        {
            // PARTICLE_SUM_TAIL -> - PARTICLE PARTICLE_SUM_TAIL
            lexer->expect_and_consume(TOK::MINUS);
            PNode negate(create_node(AST::PARTICLE_NEGATE, particle_sum));
            parse_particle(negate);

            parse_particle_sum_tail(particle_sum);

            return;
        }
        default:
            // PARTICLE_SUM_TAIL -> epsilon
            return;

    }
}


/* PARTICLE_LIST productions:
---

    PARTICLE_LIST -> PARTICLE, PARTICLE_LIST


    PARTICLE_LIST -> PARTICLE 

*/
void Parser::parse_particle_list(PNode parent) {

    PNode particle_list = make_list_root_node(AST::PARTICLE_LIST, parent);

    parse_particle(particle_list);

    auto tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        // PARTICLE_LIST -> PARTICLE, PARTICLE_LIST
        case TOK::COMMA:
            lexer->expect_and_consume(TOK::COMMA);
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

    PNode named_particle_list = make_list_root_node(AST::NAMED_PARTICLE_LIST, parent);
    parse_particle(parent);
    parse_id(parent);

    auto tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        case TOK::COMMA:
            // NAMED_PARTICLE_LIST -> PARTICLE ID, NAMED_PARTICLE_LIST
            lexer->expect_and_consume(TOK::COMMA);
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

    PNode literal_number_list = make_list_root_node(AST::LITERAL_NUMBER_LIST, parent);

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

    PNode string_list = make_list_root_node(AST::STRING_LIST, parent);

    parse_string(string_list, "Excepted string for description");

    // STRING_LIST -> STRING STRING_LIST
    auto tok = lexer->peek(0);
    if (tok->get_token_type() == TOK::STRING) {
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

    PNode variable_list = make_list_root_node(AST::VARIABLE_LIST, parent);

    PToken tok = lexer->peek(0);
    switch(tok->get_token_type()) {

        // VARIABLE_LIST -> epsilon 
        // this is the follow set for VARIABLE_LIST, and none of them are in the first set of EXPRESSION
        case TOK::CLOSE_CURLY_BRACE: case TOK::CLOSE_PAREN:
            return;            

        // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
        // VARIABLE_LIST -> EXPRESSION
        default:
        {
            parse_expression(variable_list);
            auto next = lexer->peek(0);

            // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
            if (next->get_token_type() == TOK::COMMA) {
                lexer->expect_and_consume(TOK::COMMA);
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
        
        case TOK::THIS:
        {   
            // PARTICLE -> this
            lexer->expect_and_consume(TOK::THIS);
            create_node(AST::THIS, parent, tok);
            return;
        }


        // PARTICLE -> ID arrow_index ID
        // PARTICLE -> ID arrow_index ID [INDEX]
        // PARTICLE -> ID
        // PARTICLE -> ID [INDEX]
        case TOK::STRING: case TOK::VARNAME:
        {
            PToken next = lexer->peek(1);
            PNode root_node;
            if (next->get_token_type() == TOK::OPEN_SQUARE_BRACE || lexer->peek(3)->get_token_type() == TOK::OPEN_SQUARE_BRACE) {
                root_node = create_node(AST::INDEX_OPERATOR, parent, next->get_token_type() == TOK::OPEN_SQUARE_BRACE ? next : lexer->peek(3));
            } else {
                root_node = parent;
            }

            if (next->get_token_type() == TOK::ARROW_INDEX) {
                PNode arrow = create_node(AST::OPERATOR_TERMINAL, root_node, next);
                // ID arrow_index ID
                parse_id(arrow);
                lexer->expect_and_consume(TOK::ARROW_INDEX);
                parse_id(arrow);
            } else {
                parse_id(root_node);
            }
            if  (lexer->peek(0)->get_token_type() == TOK::OPEN_SQUARE_BRACE) {
                
                // PARTICLE -> ID arrow_index ID [INDEX]
                // PARTICLE -> ID [INDEX]
                lexer->expect_and_consume(TOK::OPEN_SQUARE_BRACE);
                parse_index(root_node);
                lexer->expect_and_consume(TOK::CLOSE_SQUARE_BRACE);
            }
            return;
        }
        default:
            raise_parsing_exception("Invalid token, expected a valid name of a particle object", tok);
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
    
    PNode index(create_node(AST::INDEX, parent));

    PToken tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        case TOK::INTEGER:
        {
            // INTEGER
            parse_integer(index);

            PToken next = lexer->peek(0);
            if (next->get_token_type() == TOK::COLON) {
                // : 
                lexer->expect_and_consume(TOK::COLON);
                PToken next2 = lexer->peek(0);
                if (next2->get_token_type() == TOK::INTEGER) {
                    // INDEX -> INTEGER : INTEGER
                    parse_integer(index);
                } else {
                    // INDEX -> INTEGER :
                    create_node(AST::UNBOUNDED, index, next);
                }
            }
            return;
        }
        case TOK::COLON:
            create_node(AST::UNBOUNDED, index, tok);

            // INDEX -> : INTEGER
            lexer->expect_and_consume(TOK::COLON);
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
        case TOK::ARROW_INDEX:
        return 110 + left_associative_addition;

        // second-highest priority is an indexing of the form object.function
        // E -> E.E
        case TOK::DOT_INDEX:
        return 100 + left_associative_addition;

        // third-highest priority is the indexing operation
        // E -> E [INDEX]
        case TOK::OPEN_SQUARE_BRACE:
        return 95 + left_associative_addition;

        // raising to a power is right-associative
        case TOK::RAISED_TO_POWER:
        return 90;

        // arithmetic multiplication and division
        case TOK::MULTIPLY: case TOK::DIVIDE:
        return 80 + left_associative_addition;

        // arithmetic addition and subtraction
        case TOK::PLUS: case TOK::MINUS:
        return 70 + left_associative_addition;

        // the explicit within and outside interval operators
        case TOK::WITHIN: case TOK::OUTSIDE:
        return 40 + left_associative_addition;

        // bitwise and/or - note that this priority is not where it is in C
        case TOK::AMPERSAND: case TOK::PIPE:
        return 30 + left_associative_addition;

        // numeric comparators
        case TOK::LT: case TOK::GT: case TOK::LE: case TOK::GE: case TOK::EQ: case TOK::NE: case TOK::ASSIGN: 
        return 20 + left_associative_addition;

        // logical comparators
        case TOK::AND: 
        return 15 + left_associative_addition;

        case TOK::OR:
        return 10 + left_associative_addition;

        // ternary operator parses as E -> E : E
        // it is also right-associative
        case TOK::QUESTION:
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
        lexer->expect_and_consume(next_op->get_token_type());
        // E -> E[INDEX]
        if (next_op->get_token_type() == TOK::OPEN_SQUARE_BRACE) {
            PNode indexing(create_node(AST::INDEX_OPERATOR, parent, next_op));
            indexing->add_child(lhs);
            lhs->set_parent(indexing);
            parse_index(indexing);
            lexer->expect_and_consume(TOK::CLOSE_SQUARE_BRACE);
            return indexing;
        } else if (next_op->get_token_type() == TOK::QUESTION) {
            PNode if_statement(create_node(AST::IF_STATEMENT, parent, next_op));
            if_statement->add_child(lhs);
            lhs->set_parent(if_statement);
            if_statement->add_child(precedence_climber(if_statement, 0));
            lexer->expect_and_consume(TOK::COLON);
            if_statement->add_child(precedence_climber(if_statement, 0));
            return if_statement;
        } else if (next_op->get_token_type() == TOK::WITHIN || next_op->get_token_type() == TOK::OUTSIDE) {
            PNode interval_statement(create_node(next_op->get_token_type() == TOK::WITHIN ? AST::WITHIN_STATEMENT : AST::OUTSIDE_STATEMENT, parent, next_op));
            interval_statement->add_child(lhs);
            lhs->set_parent(interval_statement);
            interval_statement->add_child(precedence_climber(interval_statement, 0));
            lexer->expect_and_consume(TOK::COMMA);
            interval_statement->add_child(precedence_climber(interval_statement, 0));
            return interval_statement;
        }

        op_node = create_lost_node(AST::OPERATOR_TERMINAL, parent, next_op);

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

/* E' productions:
---

    E' -> 
    
 */
PNode Parser::parse_primary_expression(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {


        // E' -> this

        // particles are allowed in expressions since user defined functions may use them - this is included in that
        case TOK::THIS:
        {
            lexer->expect_and_consume(TOK::THIS);
            return create_lost_node(AST::THIS, parent, tok);
        }

        // E' -> (E)
        case TOK::OPEN_PAREN:
        {
            lexer->expect_and_consume(TOK::OPEN_PAREN);
            PNode subexpression = precedence_climber(parent, 0);
            lexer->expect_and_consume(TOK::CLOSE_PAREN);

            return subexpression;
        }

        // E' -> {VARIABLE_LIST}
        case TOK::OPEN_CURLY_BRACE:
        {
            PNode varlist = create_lost_node(AST::VARIABLE_LIST, parent, tok);

            lexer->expect_and_consume(TOK::OPEN_CURLY_BRACE);
            parse_variable_list(varlist);
            lexer->expect_and_consume(TOK::CLOSE_CURLY_BRACE);
            return varlist;
        }

        case TOK::SORT:
        {

            PNode sort_expr(create_lost_node(AST::SORT_EXPRESSION, parent, tok));
            // E' -> sort (E OPTIONAL_SORT_DIR)
            lexer->expect_and_consume(TOK::SORT);
            lexer->expect_and_consume(TOK::OPEN_PAREN);
            sort_expr->add_child(precedence_climber(parent, 0));
            parse_optional_sort_dir(sort_expr);
            lexer->expect_and_consume(TOK::CLOSE_PAREN);
            
            return sort_expr;
        }

        
        // E' -> min (VARIABLE_LIST)
        // E' -> max (VARIABLE_LIST)
        case TOK::MIN: case TOK::MAX: 
        {
            PNode minmax(create_lost_node(tok->get_token_type() == TOK::MIN ? AST::MIN_EXPRESSION : AST::MAX_EXPRESSION, parent, tok));

            lexer->expect_and_consume(tok->get_token_type());
            lexer->expect_and_consume(TOK::OPEN_PAREN);
            parse_variable_list(minmax);
            lexer->expect_and_consume(TOK::CLOSE_PAREN);
            return minmax;
        }

        // E' -> BUILT_IN_MATHEMATIC_FUN
        // E' -> BUILT_IN_MATHEMATIC_FUN (E)
        case CASE_BUILT_IN_MATH_FUN:
        {
            PNode mathfun(create_lost_node(AST::BUILTIN_FUNC_TERMINAL, parent, tok));

            lexer->expect_and_consume(tok->get_token_type());

            if (lexer->peek(0)->get_token_type() == TOK::OPEN_PAREN) {

                lexer->expect_and_consume(TOK::OPEN_PAREN);
                mathfun->add_child(precedence_climber(parent, 0));
                lexer->expect_and_consume(TOK::CLOSE_PAREN);            
            }

            return mathfun;
        }
        
        // Functions which take a particle as an argument
        // E' -> BUILT_IN_PARTICLE_FUN
        // E' -> BUILT_IN_PARTICLE_FUN (PARTICLE_LIST)
        case CASE_BUILT_IN_PARTICLE_FUN_ONE_ARG:
        case CASE_BUILT_IN_PARTICLE_FUN_TWO_ARG:
        {

            PNode partfun(create_lost_node(AST::BUILTIN_FUNC_TERMINAL, parent, tok));

            lexer->expect_and_consume(tok->get_token_type());

            if (lexer->peek(0)->get_token_type() == TOK::OPEN_PAREN) {

                lexer->expect_and_consume(TOK::OPEN_PAREN);
                parse_particle_list(partfun);
                lexer->expect_and_consume(TOK::CLOSE_PAREN);            
            }


            return partfun;
        }
        


        // E -> ID
        // E -> ID (VARIABLE_LIST)
        case TOK::STRING: case TOK::VARNAME:
        {
            PNode name(create_lost_node(AST::VARYING_TERMINAL, parent, tok));
            lexer->expect_and_consume(tok->get_token_type());
            // here, we are met with a token that isn't any other known form. If it is immediately followed by parentheses, then this is probably some external function. 
            if (lexer->peek(0)->get_token_type() == TOK::OPEN_PAREN) {
                PNode func(create_lost_node(AST::USER_FUNCTION, parent, tok));
                func->add_child(name);
                name->set_parent(func);

                lexer->expect_and_consume(TOK::OPEN_PAREN);
                parse_variable_list(func);
                lexer->expect_and_consume(TOK::CLOSE_PAREN);
                return func;
            }
            
            // otherwise, it is unclear what this is other than just some variable name - we will leave it like that
            return name;
        }


        // E' -> - E
        case TOK::MINUS: 
        {
            PNode negate_node(create_lost_node(AST::NEGATE, parent, tok));
            lexer->expect_and_consume(TOK::MINUS);
            // precedence of negation should be stronger than multiplication but weaker than power
            negate_node->add_child(precedence_climber(negate_node, 85));
            return negate_node;
        }
        // E' -> not E
        case TOK::NOT:
        {   
            PNode not_node(create_lost_node(AST::L_NOT, parent, tok));
            lexer->expect_and_consume(TOK::NOT);
            // precedence of the logical not should be higher than the other logical operations but lower than comparison
            not_node->add_child(precedence_climber(not_node, 15));
            return not_node;
        }

        // E -> NUMBER
        default:
            if(!is_numerical(tok->get_token_type())) raise_parsing_exception("Invalid token used in expression", tok);
            lexer->expect_and_consume(tok->get_token_type());
            PNode number(create_lost_node(AST::VARYING_TERMINAL, parent, tok));
            return number;
    }
}

// helper to create an AST node for an E
PNode Parser::parse_expression(PNode parent) {
    
    PNode expression(create_node(AST::EXPRESSION, parent));
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
