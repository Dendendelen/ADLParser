#include "parser.hpp"
#include <iterator>
#include <memory>

#include <iostream>

#include "exceptions.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"


/**
 * @brief Create an AST node object
 * 
 * @param in: type of AST node to be created
 * @param parent" parent of the new node
 * @return: PNode, newly created node
 */
PNode create_node(AST_type in, PNode parent) {
    return std::make_shared<Node>(in, parent);
}

/**
 * @brief Create a terminal AST node
 * 
 * @param parent: parent of new node
 * @param tok: literal token to be included in the AST
 * @return: PNode, newly created node
 */
PNode make_terminal(PNode parent, PToken tok) {
    return std::make_shared<Node>(TERMINAL, parent, tok);
}

void add_two_terminal_children(PNode parent, PToken one, PToken two) {
    PNode op(create_node(TERMINAL, parent));
    parent->add_child(op);
    op->set_token(one);

    PNode source(create_node(TERMINAL, parent));
    parent->add_child(source);
    source->set_token(two); 
}

void add_two_nested_terminal_children(PNode parent, PToken one, PToken two) {
    PNode one_node(std::make_shared<Node>(TERMINAL, parent, one));
    parent->add_child(one_node);

    PNode two_node(std::make_shared<Node>(TERMINAL, one_node, two));
    one_node->add_child(two_node);
}

bool is_numerical(Token_type t) {
    if (t == TOK_INTEGER || t == TOK_DECIMAL || t == TOK_SCIENTIFIC) return true;
    return false;
} 

Parser::Parser (Lexer *lex): lexer(lex), tree(INPUT) {
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

    BLOCKS -> DEFINITIONS BLOCKS

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
                parent->add_child(parse_info(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> DEFINITIONS BLOCKS
            case TOK_DEF: 
                parent->add_child(parse_definition(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> COMPOSITE BLOCKS
            case TOK_COMP:
                parent->add_child(parse_composite(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> OBJECT BLOCKS
            case TOK_OBJ:
                parent->add_child(parse_object(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> TABLE BLOCKS
            case TOK_TABLE:
                parent->add_child(parse_table(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> REGION BLOCKS
            case TOK_REG:
                parent->add_child(parse_region(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> HISTO_LIST BLOCKS
            case TOK_HISTOLIST:
                parent->add_child(parse_histo_list(parent));
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

PNode Parser::parse_info(PNode parent) {

    PNode info(create_node(INFO, parent));

    // INFO -> adlinfo ID INITIALIZATIONS
    lexer->expect_and_consume(TOK_ADLINFO);
    info->add_child(parse_id(info));
    parse_initializations(info);

    return info;
}


/* 
INITIALIZATIONS productions:
----

    INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS

    INITIALIZATIONS -> epsilon
 */
void Parser::parse_initializations(PNode parent) {
    PToken next = lexer->peek(0);
    
    switch (next->get_token_type()) {

        // INITIALIZATONS -> epsilon
        case TOK_ADLINFO: case TOK_DEF: case TOK_COMP: case TOK_OBJ: case TOK_TABLE: case TOK_REG: case TOK_HISTOLIST: case TOK_END_OF_FILE:
            return;
        // Anything not in the follow set indicates a continuation
        // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
        default:
            parent->add_child(parse_initialization(parent));
            parse_initializations(parent);
            return;
    }
}


/* INITIALIZATION productions:
---

    INITIALIZATION -> ID ID

*/
PNode Parser::parse_initialization(PNode parent) {
    PToken tok = lexer->peek(0);

    // assume we just want two strings or names to be an arbitrary extra info statement
    {
        PNode tagname = parse_id(parent);
        parent->add_child(tagname);

        tagname->add_child(parse_id(tagname));
        return tagname;
    }
    

}


/*
DEFINITION productions:
---

    DEFINITION -> def ID ASSIGNMENT DEF_RVALUE

*/
PNode Parser::parse_definition(PNode parent) {

    PNode definition(create_node(DEFINITION, parent));

    // DEFINITION -> def ID ASSIGNMENT DEF_RVALUE
    lexer->expect_and_consume(TOK_DEF);
    definition->add_child(parse_id(definition));

    parse_assignment();
    definition->add_child(parse_def_rvalue(definition));

    return definition;
}


/* DEF_RVALUE productions:
---

        DEF_RVALUE -> { VARIABLE_LIST }

        DEF_RVALUE -> extern string

        DEF_RVALUE -> correctionlib string string

        DEF_RVALUE -> add PARTICLE_SUM

        DEF_RVALUE -> particle_keyword PARTICLE_SUM

        DEF_RVALUE -> E

 */
PNode Parser::parse_def_rvalue(PNode parent) {

    auto tok = lexer->peek(0);
    
    switch(tok->get_token_type()) {

        // DEF_RVALUE -> { VARIABLE_LIST }
        case TOK_OPEN_CURLY_BRACE:
        {
            lexer->expect_and_consume(TOK_OPEN_CURLY_BRACE);

            PNode variable_list(create_node(VARIABLE_LIST, parent));
            parse_variable_list(variable_list);

            lexer->expect_and_consume(TOK_CLOSE_CURLY_BRACE);
            return variable_list;
        }

        // DEF_RVALUE -> external STRING
        // DEF_RVALUE -> external attribute STRING 
        case TOK_EXTERNAL:
        {
            auto external_func = make_terminal(parent, lexer->next());
            if (lexer->peek(0)->get_token_type() == TOK_ATTRIBUTE) {
                external_func->add_child(make_terminal(external_func, lexer->next()));
            }

            if (lexer->peek(0)->get_token_type() != TOK_STRING) raise_parsing_exception("External functions must be given an explicit code string to run", external_func->get_token());

            external_func->add_child(parse_id(external_func));

            return external_func;
        }

        // DEF_RVALUE -> correctionlib string string

        case TOK_CORRECTIONLIB:
        {
            auto corrlib_func = make_terminal(parent, lexer->next());

            if (lexer->peek(0)->get_token_type() != TOK_STRING) raise_parsing_exception("Correctionlib correction sets must be given an exact string for a file name", corrlib_func->get_token());
            corrlib_func->add_child(parse_id(corrlib_func));

            if (lexer->peek(0)->get_token_type() != TOK_STRING) raise_parsing_exception("Correctionlib correction set includes must be given an exact string for a key", corrlib_func->get_token());
            corrlib_func->add_child(parse_id(corrlib_func));

            return corrlib_func;
        }

        // DEF_RVALUE -> add PARTICLE_SUM
        // DEF_RVALUE -> particle_keyword PARTICLE_SUM
        case ADD: case TOK_PARTICLE_KEYWORD:
        {
            auto add_particles = make_terminal(parent, lexer->next());
            
            PNode particle_list(create_node(PARTICLE_SUM, add_particles));
            parse_particle_sum(particle_list);
            add_particles->add_child(particle_list);

            return add_particles;
        }

        // DEF_RVALUE -> E
        default:
            // assume this is an expression if the other components have not succeeded in their production
            return parse_expression(parent);
    }

}

/* OBJECT productions:
---

    OBJECT -> obj ID ASSIGNMENT OBJ_RVALUE 

 */
PNode Parser::parse_object(PNode parent) {

    PNode object(create_node(OBJECT, parent));
    
    lexer->expect_and_consume(TOK_OBJ);
    object->add_child(parse_id(object));

    // OBJECT -> obj ID ASSIGNMENT OBJ_RVALUE 
    parse_assignment();
    parse_obj_rvalue(object);

    return object;
}

/* COMPOSITE productions:
---

    COMPOSITE -> comp ID ASSIGNMENT COMP_RVALUE 

 */
PNode Parser::parse_composite(PNode parent) {

    PNode composite(create_node(COMPOSITE, parent));
    
    lexer->expect_and_consume(TOK_COMP);
    composite->add_child(parse_id(composite));

    // COMPOSITE -> comp ID ASSIGNMENT COMP_RVALUE 
    parse_assignment();
    parse_composite_rvalue(composite);

    return composite;
}

/* TABLE productions:
---
    TABLE -> TABLE_HEADER LITERAL_NUMBER_LIST
*/
PNode Parser::parse_table(PNode parent) {

    // TABLE -> TABLE_HEADER LITERAL_NUMBER_LIST

    PNode table(create_node(TABLE_DEF, parent));

    parse_table_header(table);
    parse_literal_number_list(table);

    return table;
}

/* TABLE_HEADER productions:
---
    TABLE_HEADER -> table ID tabletype ID nvars INTEGER errors BOOL 
*/
void Parser::parse_table_header(PNode parent) {
    lexer->expect_and_consume(TOK_TABLE);
    parent->add_child(parse_id(parent));

    lexer->expect_and_consume(TOK_TABLETYPE);
    parent->add_child(parse_id(parent));

    lexer->expect_and_consume(TOK_NVARS);
    auto tok = lexer->next();

    if (tok->get_token_type() != TOK_INTEGER) raise_parsing_exception("Only integers are allowed to specify NVars", tok);
    parent->add_child(make_terminal(parent, tok));

    lexer->expect_and_consume(TOK_ERRORS);
    parent->add_child(parse_bool(parent));
}

/* REGION productions:
---    

    REGION -> reg ID REGION_COMMANDS
*/
PNode Parser::parse_region(PNode parent) {

    // REGION -> reg ID REGION_COMMANDS
    PNode region(create_node(REGION, parent));

    lexer->expect_and_consume(TOK_REG);
    region->add_child(parse_id(region));
    parse_region_commands(region);

    return region;
}



/* HISTO_LIST productions:
---    

    HISTO_LIST -> histolist ID HISTO_ENTRIES
*/
PNode Parser::parse_histo_list(PNode parent) {
    // HISTO_LIST -> histolist ID HISTO_ENTRIES
    PNode histo_list(create_node(HISTO_LIST, parent));

    lexer->expect_and_consume(TOK_HISTOLIST);
    histo_list->add_child(parse_id(histo_list));
    parse_histo_entries(histo_list);

    return histo_list;
}

/* HISTO_ENTRIES productions:
---

    HISTO_ENTRIES -> HISTO_ENTRY HISTO_ENTRIES
    
    HISTO_ENTRIES -> epsilon

*/
void Parser::parse_histo_entries(PNode parent) {
PToken next = lexer->peek(0);
    
    switch (next->get_token_type()) {

        // HISTO_ENTRIES ->  HISTO_ENTRY HISTO_ENTRIES
        case TOK_HISTO: 
            parent->add_child(parse_histo_entry(parent));
            parse_histo_entries(parent);
            return;

        // HISTO_ENTRIES -> epsilon
        default:
            return;
    }
}



/* ASSIGNMENT productions:
---

    ASSIGNMENT -> :

    ASSIGNMENT -> =

    ASSIGNMENT -> take    
*/
void Parser::parse_assignment() {
    auto tok = lexer->next();
    if (tok->get_token_type() != TOK_COLON && tok->get_token_type() != TOK_TAKE && tok->get_token_type() != TOK_ASSIGN) raise_parsing_exception("Expected symbol starts this block, either ':' or '=' or TAKE expected. \n This sort of block requires one such symbol to start it off.", tok);

}



/* HISTO_ENTRY productions:
---

    HISTO_ENTRY -> histo HISTOGRAM

*/
PNode Parser::parse_histo_entry(PNode parent) {

    // HISTO_ENTRY -> histo HISTOGRAM
    lexer->expect_and_consume(TOK_HISTO);
    PNode histo(create_node(HISTOLIST_HISTOGRAM, parent));
    parse_histogram(histo);
    return histo;
}



/* COMPOSITE_RVALUE productions:
---

    COMPOSITE_RVALUE -> comb ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

    COMPOSITE_RVALUE -> dijoint ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

    COMPOSITE_RVALUE -> direct ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

---
*/

void Parser::parse_composite_rvalue(PNode parent) {
    
    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {

        // COMPOSITE_RVALUE -> comb ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA
        // COMPOSITE_RVALUE -> disjoint ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA
        // COMPOSITE_RVALUE -> direct ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

        case TOK_COMB: case TOK_DISJOINT: case TOK_DIRECT:
        {
            PNode comb_type = make_terminal(parent, lexer->next());

            parent->add_child(comb_type);

            lexer->expect_and_consume(TOK_OPEN_PAREN);

            PNode particle_list(create_node(NAMED_PARTICLE_LIST, comb_type));
            comb_type->add_child(particle_list);

            parse_named_particle_list(particle_list);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            parse_composite_criteria(parent);

            return;
        }

        default:
        {
            raise_parsing_exception("Invalid input to a composite statement, need either comb or disjoint", tok);
        }
    }
}


/* OBJ_RVALUE productions:
---

    OBJ_RVALUE -> union ( PARTICLE_LIST )

    OBJ_RVALUE -> sort (PARTICLE, E)

    OBJ_RVALUE -> PARTICLE OBJ_CRITERIA

 */
void Parser::parse_obj_rvalue(PNode parent) {
    
    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {

        // OBJ_RVALUE -> union ( PARTICLE_LIST )
        case TOK_UNION:
        {
            PNode union_type = make_terminal(parent, lexer->next());

            parent->add_child(union_type);

            lexer->expect_and_consume(TOK_OPEN_PAREN);

            PNode particle_list(create_node(PARTICLE_LIST, union_type));
            union_type->add_child(particle_list);

            parse_particle_list(particle_list);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return;
        }

        
        // OBJ_RVALUE -> sort (PARTICLE, EXPRESSION, ascend)
        // OBJ_RVALUE -> sort (PARTICLE, EXPRESSION, descend)
        // OBJ_RVALUE -> sort (PARTICLE, EXPRESSION)
        case TOK_SORT:
        {
            lexer->expect_and_consume(TOK_SORT);
            lexer->expect_and_consume(TOK_OPEN_PAREN);

            PNode sort(create_node(SORT_CMD, parent));
            parent->add_child(sort);

            sort->add_child(parse_particle(sort));

            lexer->expect_and_consume(TOK_COMMA);

            sort->add_child(parse_expression(sort));
            
            auto next = lexer->next();

            if (next->get_token_type() == TOK_CLOSE_PAREN) return;
            else if (next->get_token_type() != TOK_COMMA) raise_parsing_exception("Comma or close parenthesis expected in sorting statement", next);

            auto direction = lexer->next();
            if (direction->get_token_type() != TOK_ASCEND && direction->get_token_type() != TOK_DESCEND) raise_parsing_exception("Token after a sort expression must specify ascending or descending", direction);
            sort->add_child(make_terminal(sort, direction));

            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return;
        }

        // OBJ_RVALUE -> PARTICLE CRITERIA
        default:
        {
            PNode type = parse_particle(parent);
            parent->add_child(type);
            parse_obj_criteria(parent);
            return;
        }
    }
}

/* BOOL productions:
---

    BOOL -> true

    BOOL -> false

 */
PNode Parser::parse_bool(PNode parent) {
    auto tok = lexer->next();
    if (!(tok->get_token_type() == TOK_TRUE) && !(tok->get_token_type() == TOK_FALSE)) raise_parsing_exception("Excepted boolean, but token is not interpretable as a boolean", tok);

    PNode boolean(create_node(TERMINAL, parent));
    boolean->set_token(tok);
    return boolean;
}

/* ID productions:
---

    ID -> string

    ID -> varname

 */
PNode Parser::parse_id(PNode parent) {
    PToken next = lexer->next();
    if (next->get_token_type() != TOK_INTEGER && next->get_token_type() != TOK_STRING && next->get_token_type() != TOK_VARNAME) { 
        raise_parsing_exception("Invalid ID, allowed types are variable-type names and strings", next);
    } else if (next->get_token_type() == TOK_INTEGER) raise_parsing_exception("Invalid ID, integers for ID must be put in quotes.", next);
    return make_terminal(parent, next);
}

/* STRING_LIST productions:
---

    STRING_LIST -> string STRING_LIST

    STRING_LIST -> string

 */
PNode Parser::parse_string_list(PNode parent) {

    auto tok = lexer->next();
    if (tok->get_token_type() != TOK_STRING) {
        raise_parsing_exception("Excepted string for description", tok);
    }

    PNode description_str(make_terminal(parent, tok));

    // STRING_LIST -> STRING STRING_LIST
    if (lexer->peek(0)->get_token_type() == TOK_STRING) {
        parent->add_child(description_str);
        return parse_string_list(parent);
    }
    return description_str;
}


/* REGION_COMMANDS productions
---

    REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS

    REGION_COMMANDS -> epsilon
 */
void Parser::parse_region_commands(PNode parent) {

    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {
        case TOK_SELECT: case TOK_REJEC: case TOK_BINS: case TOK_BIN: case TOK_WEIGHT: case TOK_HISTO: case TOK_SORT: case TOK_TAKE:
            parent->add_child(parse_region_command(parent));
            parse_region_commands(parent);
            return;
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

    REGION_COMMAND -> bin string E

    REGION_COMMAND -> bins E LITERAL_NUMBER_LIST

    REGION_COMMAND -> histo HISTOGRAM
 */

PNode Parser::parse_region_command(PNode parent) {
    
    auto tok = lexer->next();
    switch(tok->get_token_type()) {
        
        // REGION_COMMAND -> select REGION_COMMAND_SELECT
        case TOK_SELECT:
        {
            PNode node(create_node(REGION_SELECT, parent));
            node->add_child(parse_expression(parent));
            return node;
        }

        // REGION_COMMAND -> reject REGION_COMMAND_SELECT
        case TOK_REJEC:
        {
            PNode node(create_node(REGION_REJECT, parent));
            node->add_child(parse_expression(parent));
            return node;
        }
        
        // REGION_COMMAND -> take ID
        case TOK_TAKE:
        {
            PNode node(create_node(REGION_USE, parent));
            node->add_child(parse_id(node));
            return node;
        }

        // REGION_COMMAND -> weight ID E
        case TOK_WEIGHT:
        {
            PNode node(create_node(WEIGHT_CMD, parent));
            node->add_child(parse_id(node));
            node->add_child(parse_expression(node));
            return node;
        }

        // REGION_COMMAND -> bin E
        // REGION_COMMAND -> bin string E
        case TOK_BIN:
        {
            PNode node(create_node(BIN_CMD, parent));

            auto next = lexer->peek(0);

            if (next->get_token_type() == TOK_STRING) {
                node->add_child(make_terminal(parent ,lexer->next()));
            }

            node->add_child(parse_expression(node));
            return node;
        }

        // REGION_COMMAND -> bins E LITERAL_NUMBER_LIST
        case TOK_BINS:
        {
            PNode node(create_node(BINS_CMD, parent));
            node->add_child(parse_expression(node));
            parse_literal_number_list(node);
            return node;
        }

        // REGION_COMMAND -> histo HISTOGRAM
        // REGION_COMMAND -> histo take ID
        case TOK_HISTO:
        {
            if (lexer->peek(0)->get_token_type() == TOK_TAKE) {
                lexer->expect_and_consume(TOK_TAKE);
                PNode histo_use(create_node(HISTO_USE, parent));
                histo_use->add_child(parse_id(histo_use));
                return histo_use;
            }
            PNode histo(create_node(HISTOGRAM, parent));
            parse_histogram(histo);
            return histo;
        }

        default:
            raise_parsing_exception("Unexpected token in region block", tok);
            return PNode(create_node(AST_ERROR, parent));
    }

}


/* HISTOGRAM productions:
---

    HISTOGRAM -> ID, DESCRIPTION, BINNING, EXPRESSION

    HISTOGRAM -> ID, DESCRIPTION, BINNING, BINNING, EXPRESSION, EXPRESSION
*/
        
void Parser::parse_histogram(PNode parent) {
    // id, 
    parent->add_child(parse_id(parent));
    lexer->expect_and_consume(TOK_COMMA);

    // DESCRIPTION,
    parent->add_child(parse_string_list(parent));
    lexer->expect_and_consume(TOK_COMMA);

    // BINNING
    parse_binning(parent);

    // use to check if the list continues 
    bool is_2d = false;
    auto discriminant = lexer->peek(1);

    if (discriminant->get_token_type() == TOK_COMMA) {
        is_2d = true;
        parse_binning(parent);
    }

    parent->add_child(parse_expression(parent));
    
    if (is_2d) {
        lexer->expect_and_consume(TOK_COMMA);
        parent->add_child(parse_expression(parent));
    }
}


/* BINNING productions:
---

    BINNING -> integer, number, number
*/

void Parser::parse_binning(PNode parent) {
auto integer_tok = lexer->next();
    if (integer_tok->get_token_type() != TOK_INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", integer_tok);
    parent->add_child(make_terminal(parent, integer_tok));
    lexer->expect_and_consume(TOK_COMMA);

    // number,
    auto lower_value_tok = lexer->next();
    if (!is_numerical(lower_value_tok->get_token_type())) raise_parsing_exception("Only literal numbers are allowed for the lower bound of a histogram", lower_value_tok);
    parent->add_child(make_terminal(parent, lower_value_tok));
    lexer->expect_and_consume(TOK_COMMA);

    // number
    auto upper_value_tok = lexer->next();
    if (!is_numerical(upper_value_tok->get_token_type())) raise_parsing_exception("Only literal numbers are allowed for the upper bound of a histogram", upper_value_tok);
    parent->add_child(make_terminal(parent, upper_value_tok));
}

/* VARIABLE_LIST productions:
---

    VARIABLE_LIST -> EXPRESSION VARIABLE_LIST

    VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST

    VARIABLE_LIST -> epsilon
 */
void Parser::parse_variable_list(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {

        // VARIABLE_LIST -> epsilon 
        // this is the follow set for VARIABLE_LIST, and none of them are in the first set of EXPRESSION
        case TOK_CLOSE_CURLY_BRACE: case TOK_CLOSE_PAREN:
            return;            

        // VARIABLE_LIST -> EXPRESSION VARIABLE_LIST
        // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
        default:
            parent->add_child(parse_expression(parent));
            auto next = lexer->peek(0);
            if (next->get_token_type() == TOK_COMMA) {
                lexer->expect_and_consume(TOK_COMMA);
            }
            parse_variable_list(parent);
            return;
    }
}




/*  productions:
---

    LITERAL_NUMBER_LIST -> number LITERAL_NUMBER_LIST

    LITERAL_NUMBER_LIST -> number

 */
void Parser::parse_literal_number_list(PNode parent) {
    // LITERAL_NUMBER_LIST -> number LITERAL_NUMBER_LIST
    // LITERAL_NUMBER_LIST -> number

    auto tok = lexer->next();
    if (!is_numerical(tok->get_token_type())) {
        raise_parsing_exception("Needs a literal numerical value for this partition", tok);
    }


    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    if (is_numerical(next->get_token_type())) parse_literal_number_list(parent);
}


/* PARTICLE_SUM productions:
---

    PARTICLE_SUM -> PARTICLE + PARTICLE_SUM

    PARTICLE_SUM -> PARTICLE PARTICLE_SUM

    PARTICLE_SUM -> PARTICLE 

*/
void Parser::parse_particle_sum(PNode parent) {

    parent->add_child(parse_particle(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // PARTICLE_SUM -> PARTICLE + PARTICLE_SUM
        case TOK_PLUS:
            lexer->expect_and_consume(TOK_PLUS);
            parse_particle_sum(parent);
            return;

        // PARTICLE_SUM -> PARTICLE PARTICLE_SUM
        case TOK_STRING: case TOK_VARNAME: case TOK_MINUS:
            parse_particle_sum(parent);
            return;

        default:
        // PARTICLE_SUM -> PARTICLE
            return;
    }
}

/* PARTICLE_LIST productions:
---

    PARTICLE_LIST -> PARTICLE, PARTICLE_SUM


    PARTICLE_LIST -> PARTICLE 

*/
void Parser::parse_particle_list(PNode parent) {

    parent->add_child(parse_particle(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // PARTICLE_LIST -> PARTICLE, PARTICLE_LIST
        case TOK_COMMA:
            lexer->expect_and_consume(TOK_COMMA);
            parse_particle_list(parent);
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

    parent->add_child(parse_particle(parent));
    parent->add_child(parse_id(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // NAMED_PARTICLE_LIST -> PARTICLE ID, NAMED_PARTICLE_LIST
        case TOK_COMMA:
            lexer->expect_and_consume(TOK_COMMA);
            parse_named_particle_list(parent);
            return;

        default:
        // PARTICLE_LIST -> PARTICLE ID
            return;
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
PNode Parser::parse_particle(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {
        
        // PARTICLE -> this
        case TOK_THIS:
        {
            PNode this_part = make_terminal(parent, lexer->next());
            return this_part;
        }


        // PARTICLE -> ID arrow_index ID [INDEX]
        // PARTICLE -> ID [INDEX]
        default:
            {
                PNode particle;
                if (lexer->peek(1)->get_token_type() == TOK_ARROW_INDEX) {
                    particle = precedence_climber(parent, 105);
                } else {
                    particle = parse_id(parent);
                }
                if  (lexer->peek(0)->get_token_type() == TOK_OPEN_SQUARE_BRACE) {
                    lexer->expect_and_consume(TOK_OPEN_SQUARE_BRACE);
                    particle->add_child(parse_index(particle));
                    lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
                }
                return particle;

            }
    }

}

/* INDEX productions:
---

    INDEX -> integer

    INDEX -> integer:integer

    INDEX -> :integer

    INDEX -> integer:
    
 */
PNode Parser::parse_index(PNode parent) {

    // INDEX -> integer
    // INDEX -> integer:integer
    // INDEX -> :integer
    // INDEX -> integer:
    
    PNode index(create_node(INDEX, parent));
    auto next = lexer->next();
    if (next->get_token_type() != TOK_INTEGER && next->get_token_type() != TOK_COLON) raise_parsing_exception("Only integers are allowed to be used as indices", next);
    index->add_child(make_terminal(index, next));
    
    // INDEX -> [integer:integer]
    // INDEX -> [:integer]
    if (next->get_token_type() == TOK_COLON || lexer->peek(0)->get_token_type() == TOK_COLON) {
        if (lexer->peek(0)->get_token_type() == TOK_COLON) lexer->expect_and_consume(TOK_COLON);

        auto next2 = lexer->next();
        if (next2->get_token_type() != TOK_INTEGER && next2->get_token_type() != TOK_CLOSE_SQUARE_BRACE) raise_parsing_exception("Only integers are allowed to be used as indices", next2);

        index->add_child(make_terminal(index, next2));
    }
    if (lexer->peek(0)->get_token_type() == TOK_CLOSE_SQUARE_BRACE) {
        lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
    }
    return index;

        

        
}


/* COMPOSITE_CRITERIA productions:
---

    COMPOSITE_CRITERIA -> COMPOSITE_CRITERION COMPOSITE_CRITERIA

    COMPOSITE_CRITERIA -> epsilon
*/
void Parser::parse_composite_criteria(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        case TOK_SELECT: case TOK_HISTO: case TOK_REJEC: case TOK_PARTICLE_KEYWORD: //case PRINT: 
            parent->add_child(parse_composite_criterion(parent));
            parse_composite_criteria(parent);
            return;
        default:
            return;
    }
}


/* CRITERIA productions:
---

    CRITERIA -> CRITERION CRITERIA

    CRITERIA -> epsilon
*/
void Parser::parse_obj_criteria(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        case TOK_SELECT: case TOK_REJEC:
            parent->add_child(parse_obj_criterion(parent));
            parse_obj_criteria(parent);
            return;
        default:
            return;
    }
}

/* COMPOSITE_CRITERION productions:
---

    COMPOSITE_CRITERION -> particle_keyword ID ASSIGNMENT PARTICLE_SUM
    COMPOSITE_CRITERION -> CRITERION
 */
PNode Parser::parse_composite_criterion(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        case TOK_PARTICLE_KEYWORD:
        {
            PNode definition(create_node(DEFINITION, parent));

            auto add_particles = make_terminal(parent, lexer->next());
            definition->add_child(parse_id(definition));
            definition->add_child(add_particles);

            parse_assignment();

            PNode particle_list(create_node(PARTICLE_SUM, add_particles));
            parse_particle_sum(particle_list);
            add_particles->add_child(particle_list);

            return definition;
        }
        default:
            return parse_obj_criterion(parent);
    }

}

/* CRITERION productions:
---

    CRITERION -> select E

    CRITERION -> reject E
 */
PNode Parser::parse_obj_criterion(PNode parent) {

    auto tok = lexer->next();

    switch (tok->get_token_type()) {

        // CRITERION -> select E
        case TOK_SELECT: case TOK_HISTO:
        {
            PNode node(create_node(OBJECT_SELECT, parent));
            node->add_child(parse_expression(node));
            return node;
        }
        // CRITERION -> reject CONDITION
        case TOK_REJEC:
        {
            PNode node(create_node(OBJECT_REJECT, parent));
            node->add_child(parse_expression(node));
            return node;
        }

        default:
            raise_parsing_exception("Invalid token for an object creation criterion", tok);
            PNode node(make_terminal(parent, tok));
            return node;
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


PNode Parser::precedence_climber(PNode parent, int min_precedence) {

    auto lhs = parse_primary_expression(parent);

    auto lookahead = lexer->peek(0);
    PToken op;
    PNode op_node;

    // keep going while the next token is an operator with at least our current level of precedence
    while (get_precedence(lookahead) >= min_precedence) {
        op = lexer->next();

        // E -> E[INDEX]
        if (op->get_token_type() == TOK_OPEN_SQUARE_BRACE) {
            lhs->add_child(parse_index(lhs));
            lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
            return lhs;
        } else if (op->get_token_type() == TOK_QUESTION) {
            PNode if_statement(create_node(IF_STATEMENT, parent));
            if_statement->add_child(lhs);
            lhs->set_parent(if_statement);
            if_statement->add_child(precedence_climber(if_statement, 0));
            lexer->expect_and_consume(TOK_COLON);
            if_statement->add_child(precedence_climber(if_statement, 0));
            return if_statement;
        }

        op_node = make_terminal(parent, op);

        // find what precedence is our new minimum - if the operator is left-associative, it is one more than its normal precedence
        int new_min_precedence = get_precedence(op, true);

        auto rhs = precedence_climber(op_node, new_min_precedence);

        op_node->add_child(lhs);
        lhs->set_parent(op_node);

        op_node->add_child(rhs);

        lhs = op_node;

        lookahead = lexer->peek(0);
    }
        
    // at this point, we have parsed all we can of precedences above our threshold. We give our final node of the loop
    return lhs;
}

PNode Parser::parse_primary_expression(PNode parent) {

    auto tok = lexer->next();
    PNode node(make_terminal(parent, tok));

    switch(tok->get_token_type()) {
        // E' -> - E
        case TOK_MINUS: 
        {
            PNode negate_node(create_node(NEGATE, parent));
            // precedence of negation should be stronger than multiplication but weaker than power
            negate_node->add_child(precedence_climber(negate_node, 85));
            return negate_node;
        }
        // E' -> not E
        case TOK_NOT:
        {   
            // precedence of the logical not should be higher than the other logical operations but lower than comparison
            node->add_child(precedence_climber(node, 15));
            return node;
        }
        // E' -> (E)
        case TOK_OPEN_PAREN:
        {
            PNode subexpression = precedence_climber(parent, 0);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);

            return subexpression;
        }

        // E -> {PARTICLE_LIST}ID
        // E -> {PARTICLE_LIST}BUILT_IN_PARTICLE_FUN
        // E -> {VARIABLE_LIST}
        case TOK_OPEN_CURLY_BRACE:
        {
            // make a node representing what the particle list function will end up being
            PNode terminal(create_node(TERMINAL, parent));
            PNode particle_list(create_node(PARTICLE_LIST, terminal));

            parse_particle_list(particle_list);
            terminal->add_child(particle_list);
            
            lexer->expect_and_consume(TOK_CLOSE_CURLY_BRACE);
            terminal->set_token(lexer->next());
            return terminal;
        }

        // E -> [E, E]
        case TOK_OPEN_SQUARE_BRACE:
        {
            PNode interval(create_node(INTERVAL, parent)); 
            interval->add_child(precedence_climber(interval, 0));
            if (lexer->peek(0)->get_token_type() == TOK_COMMA) lexer->expect_and_consume(TOK_COMMA);
            interval->add_child(precedence_climber(interval, 0));
            lexer->expect_and_consume(TOK_CLOSE_SQUARE_BRACE);
            return interval; 

        }

        case TOK_SORT:
        {
            // E -> sort (E, ascend)
            // E -> sort (E, descend)
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            node->add_child(precedence_climber(parent, 0));
            lexer->expect_and_consume(TOK_COMMA);
            node->add_child(make_terminal(node, lexer->next()));
            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            
            return node;
        }

        // E -> BUILT_IN_MATHEMATIC_FUN (E)
        case TOK_ANYOF: case TOK_ALLOF: case TOK_SQRT: case TOK_ABS: case TOK_COS:  case TOK_SIN: case TOK_TAN: case TOK_SINH: case TOK_COSH: case TOK_TANH: case TOK_EXP: case TOK_LOG: case TOK_AVE: case TOK_SUM: 
        {
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            node->add_child(precedence_climber(parent, 0));
            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            return node;
        }
        
        // Functions which take a particle as an argument
        // E -> BUILT_IN_PARTICLE_FUN
        // E -> BUILT_IN_PARTICLE_FUN (PARTICLE_LIST)
        case TOK_LETTER_E: case TOK_LETTER_P: case TOK_LETTER_M: case TOK_LETTER_Q: 
        case TOK_CHARGE: case TOK_MASS: case TOK_PHI: case TOK_ETA: case TOK_PT: 
        case TOK_DR: case TOK_DPHI: case TOK_DETA: case TOK_DR_HADAMARD: case TOK_DPHI_HADAMARD: case TOK_DETA_HADAMARD: case TOK_NUMOF: case TOK_DISTINCT:
        {
            if (lexer->peek(0)->get_token_type() != TOK_OPEN_PAREN) {
                // the next token is not an open parenthesis - this is not a function call per se, so either the argument is implicit or this is being used in reverse order in some way. That's not our problem here, so we just save that token.
                return node;
            }

            lexer->expect_and_consume(TOK_OPEN_PAREN);

            PNode particle_list(create_node(PARTICLE_LIST, node));
            parse_particle_list(particle_list);
            node->add_child(particle_list);

            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            return node;
        }
        
        // E -> all
        // E -> none
        // E -> this

        // equivalent to the logical true and false
        case TOK_ALL: case TOK_NONE:
        // allow all particles, including the this keyword, to be a token per se - this will usually not be valid in cases other than externally defined attributes
        case TOK_THIS:
        {
            return node;
        }

        // E -> ID (VARIABLE_LIST)
        // E -> ID
        case TOK_STRING: case TOK_VARNAME:
        {
            // here, we are met with a token that isn't any other known form. If it is immediately followed by parentheses, then this is probably some external function. 
            if (lexer->peek(0)->get_token_type() == TOK_OPEN_PAREN) {
                PNode func(create_node(USER_FUNCTION, parent));
                func->add_child(node);
                node->set_parent(func);

                lexer->expect_and_consume(TOK_OPEN_PAREN);
                parse_variable_list(node);
                lexer->expect_and_consume(TOK_CLOSE_PAREN);
                return func;
            }
            
            // otherwise, it is unclear what this is other than just some variable name - we will leave it like that
            return node;
        }

        // E -> min (VARIABLE_LIST)
        // E -> max (VARIABLE_LIST)
        case TOK_MIN: case TOK_MAX: 
        {
            lexer->expect_and_consume(TOK_OPEN_PAREN);
            parse_variable_list(node);
            lexer->expect_and_consume(TOK_CLOSE_PAREN);
            return node;
        }
        // E -> number
        default:
            if(!is_numerical(tok->get_token_type())) raise_parsing_exception("Invalid token used in expression", tok);
            return node;
    }
}

/* EXPRESSION productions:
---

All productions are done via precedence climbing. Stated grammar does not correctly specify the precedences, but they are accounted for.

---


 */
PNode Parser::parse_expression(PNode parent) {
    
    PNode expression(create_node(EXPRESSION, parent));
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
