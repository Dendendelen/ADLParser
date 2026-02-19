#include "parser.hpp"
#include <iterator>
#include <memory>

#include <iostream>

#include "exceptions.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"



PNode make_terminal(PNode parent, PToken tok) {
    return PNode(std::make_shared<Node>(TERMINAL, parent, tok));
}

void add_two_terminal_children(PNode parent, PToken one, PToken two) {
    PNode op(std::make_shared<Node>(TERMINAL, parent));
    parent->add_child(op);
    op->set_token(one);

    PNode source(std::make_shared<Node>(TERMINAL, parent));
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
    if (t == INTEGER || t == DECIMAL || t == SCIENTIFIC) return true;
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

    BLOCKS -> COUNT_FORMAT BLOCKS

    BLOCKS -> DEFINITIONS BLOCKS

    BLOCKS -> TABLE BLOCKS

    BLOCKS -> OBJECT BLOCKS

    BLOCKS -> REGION BLOCKS

    BLOCKS -> epsilon

 */

void Parser::parse_blocks(PNode parent) {

        auto tok = lexer->peek(0); 
        switch(tok->get_token_type()) {
            // BLOCKS -> INFO BLOCKS
            case ADLINFO:
                parent->add_child(parse_info(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> COUNT_FORMAT BLOCKS
            case COUNTSFORMAT:
                parent->add_child(parse_count_format(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> DEFINITIONS BLOCKS
            case DEF: 
                parent->add_child(parse_definition(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> TABLE BLOCKS
            case TABLE:
                parent->add_child(parse_table(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> OBJECT BLOCKS
            case OBJ:
                parent->add_child(parse_object(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> COMPOSITE BLOCKS
            case COMP:
                parent->add_child(parse_composite(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> REGION BLOCKS
            case ALGO:
                parent->add_child(parse_region(parent));
                parse_blocks(parent);
                return;

            // BLOCKS -> HISTO_LIST BLOCKS
            case HISTOLIST:
                parent->add_child(parse_histo_list(parent));
                parse_blocks(parent);
                return;
                
            // BLOCKS -> epsilon
            case LEXER_END_OF_FILE:
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

    PNode info(std::make_shared<Node>(INFO, parent));

    // INFO -> adlinfo ID INITIALIZATIONS
    lexer->expect_and_consume(ADLINFO);
    info->add_child(parse_id(info));
    parse_initializations(info);

    return info;
}

/*
COUNT_FORMAT productions:
---

    COUNT_FORMAT -> countsformat ID COUNTS_PROCESSES
*/
PNode Parser::parse_count_format(PNode parent) {   

    PNode count(std::make_shared<Node>(COUNT_FORMAT, parent));

    // COUNT_FORMAT -> countsformat ID COUNTS_PROCESSES
    lexer->expect_and_consume(COUNTSFORMAT);
    count->add_child(parse_id(count));
    parse_count_processes(count);

    return count;
}


/*
DEFINITION productions:
---

    DEFINITION -> def ID = DEF_RVALUE

    DEFINITION -> def ID : DEF_RVALUE
*/
PNode Parser::parse_definition(PNode parent) {

    PNode definition(std::make_shared<Node>(DEFINITION, parent));

    lexer->expect_and_consume(DEF);
    definition->add_child(parse_id(definition));
    auto eq_tok = lexer->next();

    // DEFINITION -> def ID = DEF_RVALUE
    // DEFINITION -> def ID : DEF_RVALUE
    if (eq_tok->get_token_type() != ASSIGN && eq_tok->get_token_type() != COLON) raise_parsing_exception("Unknown token for definition assignment, expected '=' or ':'", eq_tok);
    definition->add_child(parse_def_rvalue(definition));

    return definition;
}

/* OBJECT productions:
---

    OBJECT -> obj ID take OBJ_RVALUE 

    OBJECT -> obj ID : OBJ_RVALUE

    OBJECT -> obj ID = OBJ_RVALUE
 */
PNode Parser::parse_object(PNode parent) {

    PNode object(std::make_shared<Node>(OBJECT, parent));
    
    lexer->expect_and_consume(OBJ);
    object->add_child(parse_id(object));
    auto tok = lexer->next();

    // OBJECT -> obj ID take OBJ_RVALUE 
    // OBJECT -> obj ID : OBJ_RVALUE  
    // OBJECT -> obj ID : OBJ_RVALUE  
    if (tok->get_token_type() != COLON && tok->get_token_type() != TAKE && tok->get_token_type() != ASSIGN) raise_parsing_exception("Expected symbol for object definition, either ':' or '=' or TAKE", tok);
    parse_obj_rvalue(object);

    return object;
}

//TODO: finish composite
PNode Parser::parse_composite(PNode parent) {

    PNode object(std::make_shared<Node>(COMPOSITE, parent));
    
    lexer->expect_and_consume(COMP);
    object->add_child(parse_id(object));
    auto tok = lexer->next();

    // OBJECT -> obj ID take OBJ_RVALUE 
    // OBJECT -> obj ID : OBJ_RVALUE  
    // OBJECT -> obj ID : OBJ_RVALUE  
    if (tok->get_token_type() != COLON && tok->get_token_type() != TAKE && tok->get_token_type() != ASSIGN) raise_parsing_exception("Expected symbol for object definition, either ':' or '=' or TAKE", tok);
    parse_composite_rvalue(object);

    return object;
}

/* TABLE productions:
---
    TABLE -> table ID tabletype ID nvars integer errors BOOL BIN_OR_BOX_VALUES
*/
PNode Parser::parse_table(PNode parent) {

    // TABLE -> table ID tabletype ID nvars integer errors BOOL BIN_OR_BOX_VALUES
    PNode table(std::make_shared<Node>(TABLE_DEF, parent));

    lexer->expect_and_consume(TABLE);
    table->add_child(parse_id(table));
    lexer->expect_and_consume(TABLETYPE);
    table->add_child(parse_id(table));
    lexer->expect_and_consume(NVARS);

    auto tok = lexer->next();

    if (tok->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed to specify NVars", tok);
    table->add_child(make_terminal(table, tok));

    lexer->expect_and_consume(ERRORS);
    table->add_child(parse_bool(table));
    parse_bin_or_box_values(table);

    return table;
}


/* REGION productions:
---    

    REGION -> algo ID REGION_COMMANDS
*/
PNode Parser::parse_region(PNode parent) {

    // REGION -> algo ID REGION_COMMANDS
    PNode region(std::make_shared<Node>(REGION, parent));

    lexer->expect_and_consume(ALGO);
    region->add_child(parse_id(region));
    parse_region_commands(region);

    return region;
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

        // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
        case SKIP_HISTO: case SKIP_EFFS: case PAP_TITLE: case PAP_EXPERIMENT: case PAP_ID: case PAP_PUBLICATION: case PAP_SQRTS: case PAP_LUMI: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA: case SYSTEMATIC: case TRGE: case TRGM: 
            parent->add_child(parse_initialization(parent));
            parse_initializations(parent);
            return;

        // INITIALIZATONS -> epsilon
        case ADLINFO: case COUNTSFORMAT: case DEF: case TABLE: case OBJ: case ALGO: case HISTOLIST: case COMP:
            return;
        // Anything not in the follow set indicates a problem state, stop parsing here
        default:
            raise_parsing_exception("Unexpected token after an initialization block", next);
            return;
    }
}

/* COUNT_PROCESSES productions:
---

    COUNT_PROCESSES -> COUNT_PROCESS COUNT_PROCESSES

    COUNT_PROCESSES -> epsilon
 */
void Parser::parse_count_processes(PNode parent) {
    PToken next = lexer->peek(0);
    
    switch (next->get_token_type()) {

        // COUNT_PROCESSES -> COUNT_PROCESS COUNT_PROCESSES
        case PROCESS: 
            parent->add_child(parse_count_process(parent));
            parse_count_processes(parent);
            return;

        // COUNT_PROCESSES -> epsilon
        default:
            return;
    }
}

/* HISTO_LIST productions:
---    

    HISTO_LIST -> histolist ID HISTO_ENTRIES
*/
PNode Parser::parse_histo_list(PNode parent) {
    // HISTO_LIST -> histolist ID HISTO_ENTRIES
    PNode histo_list(std::make_shared<Node>(HISTO_LIST, parent));

    lexer->expect_and_consume(HISTOLIST);
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
        case HISTO: 
            parent->add_child(parse_histo_entry(parent));
            parse_histo_entries(parent);
            return;

        // HISTO_ENTRIES -> epsilon
        default:
            return;
    }
}



/* INITIALIZATION productions:
---

    INITIALIZATION -> trge = integer

    INITIALIZATION -> trgm = integer

    INITIALIZATION -> skph = integer

    INITIALIZATION -> skpe = integer

    INITIALIZATION -> pap_lumi NUMBER

    INITIALIZATION -> pap_sqrts NUMBER

    INITIALIZATION -> pap_experiment ID

    INITIALIZATION -> systematic BOOL string string SYST_VTYPE

    INITIALIZATION -> pap_publication DESCRIPTION

    INITIALIZATION -> pap_title DESCRIPTION

    INITIALIZATION -> pap_id DESCRIPTION

    INITIALIZATION -> pap_arxiv DESCRIPTION

    INITIALIZATION -> pap_doi DESCRIPTION

    INITIALIZATION -> pap_hepdata DESCRIPTION

*/
PNode Parser::parse_initialization(PNode parent) {
    PToken tok = lexer->next();

    switch(tok->get_token_type()) {

        // INITIALIZATION -> trge = integer
        // INITIALIZATION -> trgm = integer
        // INITIALIZATION -> skph = integer
        // INITIALIZATION -> skpe = integer
        case TRGE: case TRGM: case SKIP_HISTO: case SKIP_EFFS:
        {   // Consume terminal equals
            lexer->expect_and_consume(ASSIGN);
            
            PToken next = lexer->next();
            if (next->get_token_type() != INTEGER) raise_parsing_exception("Invalid non-integer assignment", next);

            PNode integer_target(std::make_shared<Node>(TERMINAL, parent, tok));
            integer_target->add_child(make_terminal(integer_target, next));
            return integer_target;
        } 

        // INITIALIZATION -> pap_lumi number
        // INITIALIZATION -> pap_sqrts number
        case PAP_LUMI: case PAP_SQRTS:
        {
            PToken next = lexer->next();
            if (!is_numerical(next->get_token_type())) raise_parsing_exception("Non-numerical value given for numerical field", next); 

            PNode number_target(std::make_shared<Node>(TERMINAL, parent, tok));
            number_target->add_child(make_terminal(number_target, next));
            return number_target;
        }

        // INITIALIZATION -> pap_experiment ID
        case PAP_EXPERIMENT: 
        {
            PNode experiment(std::make_shared<Node>(TERMINAL, parent, tok));
            experiment->add_child(parse_id(experiment));
            return experiment;
        }

        // INITIALIZATION -> systematic BOOL string string SYST_VTYPE
        case SYSTEMATIC:
        {
            PNode systematic(std::make_shared<Node>(TERMINAL, parent, tok));

            systematic->add_child(parse_bool(systematic));
            PToken next = lexer->next();
            PToken next2 = lexer->next();

            if (next->get_token_type() != STRING) raise_parsing_exception("Invalid systematic up identifier, expected string", next);
            if (next->get_token_type() != STRING) raise_parsing_exception("Invalid systematic down identifier, expected string", next2);

            add_two_terminal_children(systematic, next, next2);

            systematic->add_child(parse_syst_vtype(systematic));
            return systematic;
        }

        // INITIALIZATION -> pap_publication DESCRIPTION
        // INITIALIZATION -> pap_title DESCRIPTION
        // INITIALIZATION -> pap_id DESCRIPTION
        // INITIALIZATION -> pap_arxiv DESCRIPTION
        // INITIALIZATION -> pap_doi DESCRIPTION
        // INITIALIZATION -> pap_hepdata DESCRIPTION
        case PAP_TITLE: case PAP_PUBLICATION: case PAP_ID: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA:
        {
            PNode description_target(std::make_shared<Node>(TERMINAL, parent, tok));

            description_target->add_child(parse_description(description_target));
            return description_target;
        }

        // should never be reached under normal conditions
        default:
            raise_parsing_exception("Invalid token in info block", tok);
            return PNode(std::make_shared<Node>(AST_ERROR, parent));
    }

}


/* COUNT_PROCESS productions:
---

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE, ERR_TYPE

*/
PNode Parser::parse_count_process(PNode parent) {

    PNode count_process(std::make_shared<Node>(COUNT_PROCESS, parent));;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION
    lexer->expect_and_consume(PROCESS);
    count_process->add_child(parse_id(count_process));
    
    // consume comma
    lexer->expect_and_consume(COMMA, "Invalid end of argument. Needs at least 2 arguments separated by commas");
    count_process->add_child(parse_description(count_process));
    if (lexer->peek(0)->get_token_type() != COMMA) return count_process;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE
    lexer->expect_and_consume(COMMA);
    count_process->add_child(parse_err_type(count_process));
    if (lexer->peek(0)->get_token_type() != COMMA) return count_process;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE, ERR_TYPE
    lexer->expect_and_consume(COMMA);
    count_process->add_child(parse_err_type(count_process));   

    return count_process;
}

/* HISTO_ENTRY productions:
---

    HISTO_ENTRY -> histo HISTOGRAM

*/
PNode Parser::parse_histo_entry(PNode parent) {

    // HISTO_ENTRY -> histo HISTOGRAM
    lexer->expect_and_consume(HISTO);
    PNode histo(std::make_shared<Node>(HISTOLIST_HISTOGRAM, parent));
    parse_histogram(histo);
    return histo;
}

/* DEF_RVALUE productions:
---

        DEF_RVALUE -> { VARIABLE_LIST }

        DEF_RVALUE -> OME ( DESCRIPTION, {VARIABLE_LIST}, INDEX)

        DEF_RVALUE -> constituents PARTICLE_LIST

        DEF_RVALUE -> add PARTICLE_LIST

        DEF_RVALUE -> extern string

        DEF_RVALUE -> correctionlib string string

        DEF_RVALUE -> particle_keyword PARTICLE_SUM

        DEF_RVALUE -> E

 */
PNode Parser::parse_def_rvalue(PNode parent) {

    auto tok = lexer->peek(0);
    
    switch(tok->get_token_type()) {

        // DEF_RVALUE -> { VARIABLE_LIST }
        case OPEN_CURLY_BRACE:
        {
            lexer->expect_and_consume(OPEN_CURLY_BRACE);

            PNode variable_list(std::make_shared<Node>(VARIABLE_LIST, parent));
            parse_variable_list(variable_list);

            lexer->expect_and_consume(CLOSE_CURLY_BRACE);
            return variable_list;
        }

        // DEF_RVALUE -> OME ( DESCRIPTION, {VARIABLE_LIST}, INDEX)
        case OME:
        {   
            auto ome = make_terminal(parent, lexer->next());            
            lexer->expect_and_consume(OME);

            lexer->expect_and_consume(OPEN_PAREN);

            ome->add_child(parse_description(ome));
            lexer->expect_and_consume(COMMA);

            lexer->expect_and_consume(OPEN_CURLY_BRACE);
            PNode var_list(std::make_shared<Node>(VARIABLE_LIST, ome));
            parse_variable_list(var_list);
            lexer->expect_and_consume(CLOSE_CURLY_BRACE);

            lexer->expect_and_consume(COMMA);

            auto index = lexer->next();
            if (index->get_token_type() != INTEGER) raise_parsing_exception("Integer required for indexing", index);

            ome->add_child(make_terminal(ome, index));

            lexer->expect_and_consume(CLOSE_PAREN);
            return ome;
        }

        // DEF_RVALUE -> constituents PARTICLE_LIST
        case CONSTITUENTS:
        {
            auto constituents = make_terminal(parent, lexer->next());

            PNode particle_list(std::make_shared<Node>(PARTICLE_LIST, constituents));
            parse_particle_list(particle_list);
            constituents->add_child(particle_list);

            return constituents;
        }

        // DEF_RVALUE -> external STRING
        case EXTERNAL:
        {
            auto external_func = make_terminal(parent, lexer->next());

            if (lexer->peek(0)->get_token_type() != STRING) raise_parsing_exception("External functions must be given an explicit code string to run", external_func->get_token());

            external_func->add_child(parse_id(external_func));

            return external_func;
        }

        // DEF_RVALUE -> correctionlib string string

        case CORRECTIONLIB:
        {
            auto corrlib_func = make_terminal(parent, lexer->next());

            if (lexer->peek(0)->get_token_type() != STRING) raise_parsing_exception("Correctionlib correction sets must be given an exact string for a file name", corrlib_func->get_token());
            corrlib_func->add_child(parse_id(corrlib_func));

            if (lexer->peek(0)->get_token_type() != STRING) raise_parsing_exception("Correctionlib correction set includes must be given an exact string for a key", corrlib_func->get_token());
            corrlib_func->add_child(parse_id(corrlib_func));

            return corrlib_func;
        }

        // DEF_RVALUE -> add PARTICLE_SUM
        // DEF_RVALUE -> particle_keyword PARTICLE_SUM
        case ADD: case PARTICLE_KEYWORD:
        {
            auto add_particles = make_terminal(parent, lexer->next());
            
            PNode particle_list(std::make_shared<Node>(PARTICLE_SUM, add_particles));
            parse_particle_sum(particle_list);
            add_particles->add_child(particle_list);

            return add_particles;
        }

        // illegal to have a particle without the "particle" keyword due to potential parsing ambiguity
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: 
        case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV:
            raise_parsing_exception("Cannot use a particle in a definition without specifying the \"particle\" keyword", tok);
            return PNode(std::make_shared<Node>(AST_ERROR, parent));
        
        case STRING: case VARNAME:
            if (lexer->peek(1)->get_token_type() == OPEN_SQUARE_BRACE)
            {
                raise_parsing_exception("Cannot use a particle in a definition without specifying the \"particle\" keyword", tok);
                return PNode(std::make_shared<Node>(AST_ERROR, parent));
            }
            // intentional fall-through

        // DEF_RVALUE -> E
        default:
            // assume this is an expression if the other components have not succeeded in their production
            return parse_expression(parent);
    }

}

/* COMPOSITE_RVALUE productions:
---

    COMPOSITE_RVALUE -> comb ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA
    COMPOSITE_RVALUE -> dijoint ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

---
*/

void Parser::parse_composite_rvalue(PNode parent) {
    
    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {

        // OBJ_RVALUE -> comb ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA
        // COMPOSITE_RVALUE -> disjoint ( NAMED_PARTICLE_LIST ) COMPOSITE_CRITERIA

        case COMB: case DISJOINT:
        {
            PNode comb_type = make_terminal(parent, lexer->next());

            parent->add_child(comb_type);

            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(std::make_shared<Node>(NAMED_PARTICLE_LIST, comb_type));
            comb_type->add_child(particle_list);

            parse_named_particle_list(particle_list);
            lexer->expect_and_consume(CLOSE_PAREN);

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

    OBJ_RVALUE -> comb ( PARTICLE_LIST )

    OBJ_RVALUE -> first ( PARTICLE_LIST )

    OBJ_RVALUE -> second (PARTICLE_LIST)

    OBJ_RVALUE -> PARTICLE CRITERIA

 */
void Parser::parse_obj_rvalue(PNode parent) {
    
    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {

        // OBJ_RVALUE -> union ( PARTICLE_LIST )
        case UNION: // case COMB: case FIRST: case SECOND:
        {
            PNode union_or_comb_type = make_terminal(parent, lexer->next());

            parent->add_child(union_or_comb_type);

            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(std::make_shared<Node>(PARTICLE_LIST, union_or_comb_type));
            union_or_comb_type->add_child(particle_list);

            parse_particle_list(particle_list);
            lexer->expect_and_consume(CLOSE_PAREN);

            return;
        }

        // OBJ_RVALUE -> comb ( PARTICLE_LIST )
        // case COMB:
        // {
        //     PNode comb_type = make_terminal(parent, lexer->next());

        //     parent->add_child(comb_type);

        //     lexer->expect_and_consume(OPEN_PAREN);

        //     PNode particle_list(std::make_shared<Node>(PARTICLE_LIST, comb_type));
        //     comb_type->add_child(particle_list);

        //     parse_particle_list(particle_list);
        //     lexer->expect_and_consume(CLOSE_PAREN);

        //     return;
        // }

        // OBJ_RVALUE -> PARTICLE CRITERIA
        default:
        {
            PNode type = parse_particle(parent);
            parent->add_child(type);
            parse_criteria(parent);
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
    if (!(tok->get_token_type() == TRUE) && !(tok->get_token_type() == FALSE)) raise_parsing_exception("Excepted boolean, but token is not interpretable as a boolean", tok);

    PNode boolean(std::make_shared<Node>(TERMINAL, parent));
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
    if (next->get_token_type() != INTEGER && next->get_token_type() != STRING && next->get_token_type() != VARNAME) { 
        raise_parsing_exception("Invalid ID, allowed types are variable-type names and strings", next);
    } else if (next->get_token_type() == INTEGER) raise_parsing_exception("Invalid ID, integers for ID must be put in quotes.", next);
    return make_terminal(parent, next);
}

/* DESCRIPTION productions:
---

    DESCRIPTION -> string DESCRIPTION

    DESCRIPTION -> string

 */
PNode Parser::parse_description(PNode parent) {

    auto tok = lexer->next();
    PNode description_str(std::make_shared<Node>(TERMINAL, parent, tok));
    if (tok->get_token_type() != STRING) {
        raise_parsing_exception("Excepted string for description", tok);
    }
    // DESCRIPTION -> STRING DESCRIPTION
    if (lexer->peek(0)->get_token_type() == STRING) {
        parent->add_child(description_str);
        return parse_description(parent);
    }
    return description_str;
}

/* ERR_TYPE productions:
---

    ERR_TYPE -> ERR_SYST

    ERR_TYPE -> ERR_STAT
 */
PNode Parser::parse_err_type(PNode parent) {
    auto tok = lexer->next();
    if (tok->get_token_type() == ERR_SYST || tok->get_token_type() == ERR_STAT) {
        return make_terminal(parent, tok);
    }
    raise_parsing_exception("Excepted error type 'syst' or 'stat'", tok);
    return PNode(std::make_shared<Node>(AST_ERROR, parent));
}

/* REGION_COMMANDS productions
---

    REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS

    REGION_COMMANDS -> epsilon
 */
void Parser::parse_region_commands(PNode parent) {

    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {
        case SELECT: case REJEC: case BINS: case BIN: case SAVE: case PRINT: case WEIGHT: case COUNTS: case HISTO: case SORT: case USE: case TAKE:
            parent->add_child(parse_region_command(parent));
            parse_region_commands(parent);
            return;
        default:
            return;
    }
}


/*  REGION_COMMAND_SELECT productions:
---

    REGION_COMMAND_SELECT -> none

    REGION_COMMAND_SELECT -> all
    
    REGION_COMMAND_SELECT -> IF_OR_CONDITION

*/
PNode Parser::parse_region_command_select(PNode parent) {

    auto next = lexer->peek(0);
    switch(next->get_token_type()) {

        // REGION_COMMAND_SELECT -> none
        // REGION_COMMAND_SELECT -> all

        case NONE: case ALL: 
        {
            lexer->next();
            PNode cond(std::make_shared<Node>(CONDITION, parent));
            cond->add_child(make_terminal(cond, next));
            return cond;
        }
        

        // REGION_COMMAND_SELECT -> IF_OR_CONDITION
        default:
            return parse_if_or_condition(parent);
    }
}


/* REGION_COMMAND productions:
---

    REGION_COMMAND -> select REGION_COMMAND_SELECT

    REGION_COMMAND -> weight ID E

    REGION_COMMAND -> bin CONDITION

    REGION_COMMAND -> rejec CONDITION

    REGION_COMMAND -> use ID

    REGION_COMMAND -> take ID

    REGION_COMMAND -> bins ID BIN_OR_BOX_VALUES

    REGION_COMMAND -> save ID

    REGION_COMMAND -> save ID csv VARIABLE_LIST

    REGION_COMMAND -> counts ID COUNTS

    REGION_COMMAND -> histo HISTOGRAM

    REGION_COMMAND -> if EXPRESSION then REGION_COMMAND else REGION_COMMAND

    REGION_COMMAND -> if EXPRESSION do REGION_COMMAND

    REGION_COMMAND -> lepsf

    REGION_COMMAND -> btagsf

    REGION_COMMAND -> xlumicorrsf

    REGION_COMMAND -> hlt DESCRIPTION

    REGION_COMMAND -> applyhm (ID (E, E) eq integer)

    REGION_COMMAND -> applyhm (ID (E) eq integer)
 */

PNode Parser::parse_region_command(PNode parent) {
    
    auto tok = lexer->next();
    switch(tok->get_token_type()) {
        
        // REGION_COMMAND -> select REGION_COMMAND_SELECT
        case SELECT:
        {
            PNode node(std::make_shared<Node>(REGION_SELECT, parent));
            node->add_child(parse_region_command_select(parent));
            return node;
        }

        // REGION_COMMAND -> weight ID E
        case WEIGHT:
        {
            PNode node(std::make_shared<Node>(WEIGHT_CMD, parent));
            node->add_child(parse_id(node));
            node->add_child(parse_expression(node));
            return node;
        }

        // REGION_COMMAND -> rejec REGION_COMMAND_SELECT
        case REJEC:
        {
            PNode node(std::make_shared<Node>(REGION_REJECT, parent));
            node->add_child(parse_region_command_select(parent));
            return node;
        }

        // REGION_COMMAND -> bin CONDITION
        case BIN:
        {
            PNode node(std::make_shared<Node>(BIN_CMD, parent));
            node->add_child(parse_condition(node));
            return node;
        }

        // REGION_COMMAND -> use ID
        // REGION_COMMAND -> take ID
        case USE: case TAKE:
        {
            PNode node(std::make_shared<Node>(REGION_USE, parent));
            node->add_child(parse_id(node));
            return node;
        }

        // REGION_COMMAND -> bins ID BIN_OR_BOX_VALUES
        case BINS:
        {
            PNode node(std::make_shared<Node>(BINS_CMD, parent));
            node->add_child(parse_expression(node));
            parse_bin_or_box_values(node);
            return node;
        }

        // REGION_COMMAND -> save ID
        // REGION_COMMAND -> save ID csv VARIABLE_LIST
        case SAVE:
        {
            PNode save(make_terminal(parent, tok));
            save->add_child(parse_id(save));
            auto next = lexer->peek(0);
            if (next->get_token_type() == CSV) {
                lexer->expect_and_consume(CSV);
                parse_variable_list(save);
            }  
            return save;          
        }

        // REGION_COMMAND -> print VARIABLE_LIST
        case PRINT:
        {
            PNode print(make_terminal(parent, tok));
            parse_variable_list(print);
            return print;
        }

        // REGION_COMMAND -> counts ID COUNTS
        case COUNTS:
        {
            PNode counts(make_terminal(parent, tok));
            counts->add_child(parse_id(counts));
            parse_counts(counts);
            return counts;
        }

        // REGION_COMMAND -> histo HISTOGRAM
        // REGION_COMMAND -> histo use ID
        case HISTO:
        {
            if (lexer->peek(0)->get_token_type() == USE) {
                lexer->expect_and_consume(USE);
                PNode histo_use(std::make_shared<Node>(HISTO_USE, parent));
                histo_use->add_child(parse_id(histo_use));
                return histo_use;
            }
            PNode histo(std::make_shared<Node>(HISTOGRAM, parent));
            parse_histogram(histo);
            return histo;
        }

        // REGION_COMMAND -> sort EXPRESSION ascend
        // REGION_COMMAND -> sort EXPRESSION descend
        case SORT:
        {
            PNode sort(std::make_shared<Node>(SORT_CMD, parent));
            sort->add_child(parse_expression(sort));
            
            auto next = lexer->next();
            if (next->get_token_type() != ASCEND && next->get_token_type() != DESCEND) raise_parsing_exception("Token after a sort expression must specify ascending or descending", next);
            sort->add_child(make_terminal(sort, next));
            return sort;
        }

        // REGION_COMMAND_SELECT -> hlt DESCRIPTION
        case HLT:
        {
            auto node = make_terminal(parent, tok);
            lexer->next();
            node->add_child(parse_description(node));
            return node;
        }

        // REGION_COMMAND -> applyhm (ID (E, E) eq integer)
        // REGION_COMMAND -> applyhm (ID (E) eq integer)
        case APPLY_HM:
        {
            auto node = make_terminal(parent, tok);
            lexer->next();
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_id(node));
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_expression(node));

            if (lexer->peek(0)->get_token_type() == COMMA) {
                lexer->expect_and_consume(COMMA);
                node->add_child(parse_expression(node));
            }

            lexer->expect_and_consume(CLOSE_PAREN);
            lexer->expect_and_consume(EQ);
            auto integer = lexer->next();

            if (integer->get_token_type() != INTEGER) raise_parsing_exception("Needs integer type", integer);
            node->add_child(make_terminal(node, integer));

            return node;
        }

        // REGION_COMMAND -> lepsf
        // REGION_COMMAND -> btagsf
        // REGION_COMMAND -> xlumicorrsf
        case LEP_SF: case BTAGS_SF: case XSLUMICORR_SF:
        {
            auto node = make_terminal(parent, tok);
            lexer->next();
            return node;
        }
        default:
            raise_parsing_exception("Unexpected token in region block", tok);
            return PNode(std::make_shared<Node>(AST_ERROR, parent));
    }

}


/* HISTOGRAM productions:
---

    HISTOGRAM -> ID, DESCRIPTION, integer, number, number, EXPRESSION

    HISTOGRAM -> ID, DESCRIPTION, integer, number, number, integer, number, number, EXPRESSION, EXPRESSION
*/
        
void Parser::parse_histogram(PNode parent) {
    // id, 
    parent->add_child(parse_id(parent));
    lexer->expect_and_consume(COMMA);

    // DESCRIPTION,
    parent->add_child(parse_description(parent));
    lexer->expect_and_consume(COMMA);

    // integer,
    auto integer_tok_1 = lexer->next();
    if (integer_tok_1->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", integer_tok_1);
    parent->add_child(make_terminal(parent, integer_tok_1));
    lexer->expect_and_consume(COMMA);

    // number,
    auto lower_value_tok_1 = lexer->next();
    if (!is_numerical(lower_value_tok_1->get_token_type())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_1);
    parent->add_child(make_terminal(parent, lower_value_tok_1));
    lexer->expect_and_consume(COMMA);

    // number,
    auto upper_value_tok_1 = lexer->next();
    if (!is_numerical(upper_value_tok_1->get_token_type())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
    parent->add_child(make_terminal(parent, upper_value_tok_1));
    lexer->expect_and_consume(COMMA);

    // use to check if the list continues 
    bool is_2d = false;
    auto discriminant = lexer->peek(1);

    if (discriminant->get_token_type() == COMMA) {

        // REGION_COMMANDS -> parent ID, DESCRIPTION, integer, number, number, integer, number, number, EXPRESSION, EXPRESSION
        is_2d = true;

        // integer,
        auto integer_tok_2 = lexer->next();
        if (integer_tok_2->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", integer_tok_2);
        parent->add_child(make_terminal(parent, integer_tok_2));
        lexer->expect_and_consume(COMMA);

        // number,
        auto lower_value_tok_2 = lexer->next();
        if (!is_numerical(lower_value_tok_2->get_token_type())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_2);
        parent->add_child(make_terminal(parent, lower_value_tok_2));
        lexer->expect_and_consume(COMMA);

        // number,
        auto upper_value_tok_2 = lexer->next();
        if (!is_numerical(upper_value_tok_2->get_token_type())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
        parent->add_child(make_terminal(parent, upper_value_tok_2));
        lexer->expect_and_consume(COMMA);
    }

    parent->add_child(parse_expression(parent));
    
    if (is_2d) {
        lexer->expect_and_consume(COMMA);
        parent->add_child(parse_expression(parent));
    }
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
        // TODO: update this
        case CLOSE_CURLY_BRACE: case CLOSE_PAREN: case COLON: case OBJ: case COMP:  case SELECT: case PRINT: case HISTO: case REJEC: case BINS: case BIN: case SAVE: case WEIGHT: case COUNTS: case SORT: case ADLINFO: case COUNTSFORMAT: case DEF: case TABLE: case ALGO: case COMMA: case USE:
            return;            

        // VARIABLE_LIST -> EXPRESSION VARIABLE_LIST
        // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
        default:
            parent->add_child(parse_expression(parent));
            auto next = lexer->peek(0);
            if (next->get_token_type() == COMMA) {
                lexer->expect_and_consume(COMMA);
            }
            parse_variable_list(parent);
            return;
    }
}


/* REGION_CONDITIONAL_COMMAND productions:
---

    REGION_CONDITIONAL_COMMAND -> if CONDITION then ACTION else ACTION
    REGION_CONDITIONAL_COMMAND -> if CONDITION do ACTION

*/

//TODO: replace other condition with this one, make expressions parse to them only in parentheses
/* CONDITION productions: 
---

    CONDITION -> E ? E : E

    CONDITION -> E

*/


/* IF_OR_CONDITION productions:
---

    IF_OR_CONDITION -> CONDITION

    IF_OR_CONDITION -> CONDITION ? ACTION : ACTION
 */
PNode Parser::parse_if_or_condition(PNode parent) {

    //TODO: change IF token to something else
    PNode node(std::make_shared<Node>(IF_STATEMENT, parent));

    node->add_child(parse_condition(node));

    auto tok = lexer->peek(0);
    if (tok->get_token_type() == QUESTION) {
        lexer->expect_and_consume(QUESTION);
        node->add_child(parse_action(node));
        lexer->expect_and_consume(COLON);
        node->add_child(parse_action(node));
    }

    return node;
}

/* ACTION productions:
---

    ACTION -> all

    ACTION -> none

    ACTION -> lep_sf

    ACTION -> btags_sf

    ACTION -> APPLY_PTF ( E )

    ACTION -> APPLY_PTF (ID [E] )

    ACTION -> APPLY_PTF (ID [E, E] )

    ACTION -> APPLY_PTF (ID [E, E, E, E] )

    ACTION -> APPLY_HM (ID (E, E) == integer)

    ACTION -> APPLY_HM (ID (E) == integer)

    ACTION -> IF_OR_CONDITION
 */
PNode Parser::parse_action(PNode parent){
    auto tok = lexer->peek(0);
    switch (tok->get_token_type()) {

        // ACTION -> print VARIABLE_LIST
        case PRINT:
        {
            lexer->next();
            PNode print(make_terminal(parent, tok));
            parse_variable_list(print);
            return print;
        }
        
        // ACTION -> all
        // ACTION -> none
        // ACTION -> lep_sf
        // ACTION -> btags_sf
        case ALL: case NONE: case LEP_SF: case BTAGS_SF: 
        {
            lexer->next();
            return make_terminal(parent, tok);
        }

        // ACTION -> APPLY_PTF ( E )
        // ACTION -> APPLY_PTF (ID [E] )
        // ACTION -> APPLY_PTF (ID [E, E] )
        // ACTION -> APPLY_PTF (ID [E, E, E, E] )
        case APPLY_PTF:
        {
            lexer->next();
            PNode apply_ptf(make_terminal(parent, tok));
            lexer->expect_and_consume(OPEN_PAREN);
            if (lexer->peek(1)->get_token_type() == OPEN_SQUARE_BRACE) {
                lexer->expect_and_consume(OPEN_SQUARE_BRACE);
                apply_ptf->add_child(parse_expression(apply_ptf));

                if (lexer->peek(0)->get_token_type() == COMMA) {
                    lexer->expect_and_consume(COMMA);
                    apply_ptf->add_child(parse_expression(apply_ptf));

                    if (lexer->peek(0)->get_token_type() == COMMA) {
                        lexer->expect_and_consume(COMMA);
                        apply_ptf->add_child(parse_expression(apply_ptf));
                        lexer->expect_and_consume(COMMA);
                        apply_ptf->add_child(parse_expression(apply_ptf));
                    }
                }
                
                lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            } else {
                
            }
            lexer->expect_and_consume(CLOSE_PAREN);
            return apply_ptf;
        }

        // ACTION -> APPLY_HM (ID (E, E) == integer)
        // ACTION -> APPLY_HM (ID (E) == integer)
        case APPLY_HM:
        {
            lexer->next();
            PNode apply_hm(make_terminal(parent, tok));
            lexer->expect_and_consume(OPEN_PAREN);
            apply_hm->add_child(parse_id(apply_hm));
            lexer->expect_and_consume(OPEN_PAREN);
            apply_hm->add_child(parse_expression(apply_hm));
            if (lexer->peek(0)->get_token_type() == COMMA) {
                lexer->expect_and_consume(COMMA);
                apply_hm->add_child(parse_expression(apply_hm));
            }
            lexer->expect_and_consume(CLOSE_PAREN);
            lexer->expect_and_consume(EQ);

            auto next = lexer->next();
            if (next->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed for comparison in applying hit-or-miss", tok);

            lexer->expect_and_consume(CLOSE_PAREN);
 
            return apply_hm;
        }

        // ACTION -> IF_OR_CONDITION
        default:
            return parse_if_or_condition(parent);

    }
}

/* BIN_OR_BOX_VALUES productions:
---

    BIN_OR_BOX_VALUES -> number BIN_OR_BOX_VALUES

    BIN_OR_BOX_VALUES -> number

 */
void Parser::parse_bin_or_box_values(PNode parent) {
    // BIN_OR_BOX_VALUES -> number BIN_OR_BOX_VALUES
    // BIN_OR_BOX_VALUES -> number

    auto tok = lexer->next();
    if (!is_numerical(tok->get_token_type())) raise_parsing_exception("Needs a numerical value for box argument", tok);

    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    if (is_numerical(next->get_token_type())) parse_bin_or_box_values(parent);
}

/* COUNTS productions:
---

    COUNTS -> COUNT, COUNTS

    COUNTS -> COUNT
 */
void Parser::parse_counts(PNode parent) {
    // COUNTS -> COUNT
    // COUNTS -> COUNT, COUNTS

    PNode count(std::make_shared<Node>(COUNT, parent));
    parse_count(count);
    parent->add_child(count);

    if (lexer->peek(0)->get_token_type() == COMMA) {
        parse_counts(parent);
    }

    return;
}

/* COUNT productions:
---

    COUNT -> number + COUNT

    COUNT -> number - COUNT

    COUNT -> number pm COUNT

    COUNT -> number
*/
void Parser::parse_count(PNode parent) {

    auto tok = lexer->next();
    if (!is_numerical(tok->get_token_type())) raise_parsing_exception("A numerical type is needed for counts", tok);

    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    switch (next->get_token_type()) {
        case PLUS: case MINUS: case PM:
        {
            parent->add_child(make_terminal(parent, lexer->next()));
            parse_count(parent);
            return;
        }
        default:
            return;
    }   

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
        case PLUS:
            lexer->expect_and_consume(PLUS);
            parse_particle_sum(parent);
            return;

        // PARTICLE_SUM -> PARTICLE PARTICLE_SUM
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: 
        case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV: case STRING: case VARNAME: case MINUS:
            parse_particle_sum(parent);
            return;

        default:
        // PARTICLE_SUM -> PARTICLE
            return;
    }
}

void Parser::parse_particle_list(PNode parent) {

    parent->add_child(parse_particle(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // PARTICLE_LIST -> PARTICLE, PARTICLE_LIST
        case COMMA:
            lexer->expect_and_consume(COMMA);
            parse_particle_list(parent);
            return;

        default:
        // PARTICLE_LIST -> PARTICLE
            return;
    }
}

void Parser::parse_named_particle_list(PNode parent) {

    parent->add_child(parse_particle(parent));
    parent->add_child(parse_id(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // NAMED_PARTICLE_LIST -> PARTICLE ID, NAMED_PARTICLE_LIST
        case COMMA:
            lexer->expect_and_consume(COMMA);
            parse_named_particle_list(parent);
            return;

        default:
        // PARTICLE_LIST -> PARTICLE ID
            return;
    }
}


/* PARTICLE productions:
---

    PARTICLE -> first(PARTICLE)
    
    PARTICLE -> second(PARTICLE)

    PARTICLE -> ID arrow_index ID

    PARTICLE -> gen constituents

    PARTICLE -> jet constituents

    PARTICLE -> fjet constituents

    PARTICLE -> gen INDEX

    PARTICLE -> electron INDEX

    PARTICLE -> muon INDEX

    PARTICLE -> tau INDEX

    PARTICLE -> track INDEX

    PARTICLE -> lepton INDEX

    PARTICLE -> photon INDEX

    PARTICLE -> jet INDEX

    PARTICLE -> bjet INDEX

    PARTICLE -> fjet INDEX

    PARTICLE -> qgjet INDEX

    PARTICLE -> numet INDEX

    PARTICLE -> metlv INDEX

    PARTICLE -> - PARTICLE

    PARTICLE -> ID INDEX
 */
PNode Parser::parse_particle(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        // PARTICLE -> first(PARTICLE)
        // PARTICLE -> second(PARTICLE)
        case FIRST: case SECOND:
        {
            PNode helper_func = make_terminal(parent, lexer->next());
            lexer->expect_and_consume(OPEN_PAREN);
            helper_func->add_child(parse_particle(helper_func));
            lexer->expect_and_consume(CLOSE_PAREN);
            return helper_func;
        }
        
        // PARTICLE -> this
        case THIS:
        {
            PNode this_part = make_terminal(parent, lexer->next());
            return this_part;
        }

        // PARTICLE -> gen constituents
        // PARTICLE -> jet constituents
        // PARTICLE -> fjet constituents
        case GEN: case JET: case FJET:
        {
            if (lexer->peek(1)->get_token_type()==CONSTITUENTS) {
                PNode particle = make_terminal(parent, lexer->next());
                PNode constituents = make_terminal(parent, lexer->next());
                constituents->add_child(particle);
                return constituents;
            }
            // Intentional fall-through
        }
        
        // PARTICLE -> gen INDEX
        // PARTICLE -> jet INDEX
        // PARTICLE -> fjet INDEX
        // PARTICLE -> gen INDEX
        // PARTICLE -> electron INDEX
        // PARTICLE -> muon INDEX
        // PARTICLE -> tau INDE
        // PARTICLE -> track INDEX
        // PARTICLE -> lepton INDEX
        // PARTICLE -> photon INDEX
        // PARTICLE -> jet INDEX
        // PARTICLE -> bjet INDEX
        // PARTICLE -> fjet INDEX
        // PARTICLE -> qgjet INDEX
        // PARTICLE -> numet INDEX
        // PARTICLE -> metlv INDEX
        case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON:  case BJET: case QGJET: case NUMET: case METLV:
            {
                PNode particle = make_terminal(parent, lexer->next());
                particle->add_child(parse_index(particle));
                return particle;
            }
        case MINUS:
            {
                PNode minus = make_terminal(parent, lexer->next());
                minus->add_child(parse_particle(minus));
                return minus;
            }

        // PARTICLE -> ID arrow_index ID
        // PARTICLE -> ID INDEX
        default:
            {
                if (lexer->peek(1)->get_token_type() == ARROW_INDEX) {
                    return precedence_climber(parent, 0);
                }
                PNode particle = parse_id(parent);
                particle->add_child(parse_index(particle));
                return particle;
            }
    }

}

/* INDEX productions:
---

    INDEX -> [integer]

    INDEX -> [integer:integer]
    
    INDEX -> epsilon

 */
PNode Parser::parse_index(PNode parent) {

    auto tok = lexer->peek(0);

    switch(tok->get_token_type()) {

        // INDEX -> [integer]
        // INDEX -> [integer:integer]
        case OPEN_SQUARE_BRACE:
        {    
            lexer->next();
            PNode index(std::make_shared<Node>(INDEX, parent));
            auto next = lexer->next();
            if (next->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed to be used as indices", next);
            index->add_child(make_terminal(index, next));
            
            // INDEX -> [integer:integer]
            if (lexer->peek(0)->get_token_type() == COLON) {
                lexer->expect_and_consume(COLON);

                auto next2 = lexer->next();
                if (next->get_token_type() != INTEGER) raise_parsing_exception("Only integers are allowed to be used as indices", next);

                index->add_child(make_terminal(index, next2));
            }
            if (tok->get_token_type() == OPEN_SQUARE_BRACE) {
                lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            }
            return index;

        }

        // INDEX -> epsilon
        default:
            return PNode(std::make_shared<Node>(AST_EPSILON, parent));
    }
}

/* LEPTON productions:
---

    LEPTON -> electron

    LEPTON -> muon

    LEPTON -> tau
 */
PNode Parser::parse_lepton(PNode parent) {
    auto tok = lexer->next();
    
    switch(tok->get_token_type()) {
        case ELECTRON: case MUON: case TAU:
            return make_terminal(parent, tok);
        default:
            raise_parsing_exception("This token must be a lepton type", tok);
            return PNode(std::make_shared<Node>(AST_ERROR, parent));
    }
}


/* SYST_VTYPE productions:
---

    SYST_VTYPE -> syst_weight_mc

    SYST_VTYPE -> syst_weight_jvt

    SYST_VTYPE -> syst_weight_pileup

    SYST_VTYPE -> syst_weight_lepton_sf

    SYST_VTYPE -> syst_weight_btag_sf

    SYST_VTYPE -> syst_ttree

 */
PNode Parser::parse_syst_vtype(PNode parent) {
    auto tok = lexer->next();
    switch(tok->get_token_type()) {
        case SYST_WEIGHT_MC: case SYST_WEIGHT_JVT: case SYST_WEIGHT_PILEUP: case SYST_WEIGHT_LEPTON_SF: case SYST_WEIGHT_BTAG_SF: case SYST_TTREE:
            return make_terminal(parent, tok);
        default:
            raise_parsing_exception("Expected a valid systematics type", tok);
            return PNode(std::make_shared<Node>(AST_ERROR, parent));
    }
}

/* COMPOSITE_CRITERIA productions:
---

    COMPOSITE_CRITERIA -> COMPOSITE_CRITERION COMPOSITE_CRITERIA

    COMPOSITE_CRITERIA -> epsilon
*/
void Parser::parse_composite_criteria(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        case SELECT: case PRINT: case HISTO: case REJEC: case PARTICLE_KEYWORD:
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
void Parser::parse_criteria(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token_type()) {
        case SELECT: case PRINT: case HISTO: case REJEC:
            parent->add_child(parse_criterion(parent));
            parse_criteria(parent);
            return;
        default:
            return;
    }
}

/* COMPOSITE_CRITERION productions:
---

    COMPOSITE_CRITERION -> particle_keyword ID = PARTICLE_SUM
    COMPOSITE_CRITERION -> particle_keyword ID : PARTICLE_SUM

    COMPOSITE_CRITERION -> CRITERION
 */
PNode Parser::parse_composite_criterion(PNode parent) {

    auto tok = lexer->peek(0);

    switch (tok->get_token_type()) {

        case PARTICLE_KEYWORD:
        {
            PNode definition(std::make_shared<Node>(DEFINITION, parent));

            auto add_particles = make_terminal(parent, lexer->next());
            definition->add_child(parse_id(definition));
            definition->add_child(add_particles);

            auto eq_tok = lexer->next();

            if (eq_tok->get_token_type() != ASSIGN && eq_tok->get_token_type() != COLON) raise_parsing_exception("Unknown token for particle definition assignment, expected '=' or ':'", eq_tok);
            
            PNode particle_list(std::make_shared<Node>(PARTICLE_SUM, add_particles));
            parse_particle_sum(particle_list);
            add_particles->add_child(particle_list);

            return definition;
        }
        default:
            return parse_criterion(parent);
    }

}

/* CRITERION productions:
---

    CRITERION -> select ACTION

    CRITERION -> print VARIABLE_LIST

    CRITERION -> rejec CONDITION
 */
PNode Parser::parse_criterion(PNode parent) {

    auto tok = lexer->next();

    switch (tok->get_token_type()) {

        // CRITERION -> cmd ACTION
        case SELECT: case HISTO:
        {
            PNode node(std::make_shared<Node>(OBJECT_SELECT, parent));
            node->add_child(parse_action(node));
            return node;
        }
        // CRITERION -> rejec CONDITION
        case REJEC:
        {
            PNode node(std::make_shared<Node>(OBJECT_REJECT, parent, tok));
            node->add_child(parse_condition(node));
            return node;
        }
        // CRITERION -> histo ACTION
        {
            PNode node(make_terminal(parent, tok));
            node->add_child(parse_action(node));
            return node;
        }
        // CRITERION -> print VARIABLE_LIST
        case PRINT:
        {
            PNode node(make_terminal(parent, tok));
            parse_variable_list(node);
            return node;
        }

        default:
            raise_parsing_exception("Invalid token for a criterion", tok);
            PNode node(make_terminal(parent, tok));
            return node;
    }

}

/* CONDITION productions:
---

    CONDITION -> EXPRESSION
 */
PNode Parser::parse_condition(PNode parent) {
    PNode condition(std::make_shared<Node>(CONDITION, parent));

    condition->add_child(precedence_climber(condition,  0));

    return condition;
}

int get_precedence(PToken tok, bool increase_if_left_associative = false) {

    int left_associative_addition = increase_if_left_associative ? 1 : 0;
    
    switch(tok->get_token_type()) {

        // highest priority is an indexing of the form composite->subvariable
        case ARROW_INDEX:
        return 110 + left_associative_addition;

        // second-highest priority is an indexing of the form object.function
        case DOT_INDEX:
        return 100 + left_associative_addition;

        // raising to a power is right-associative
        case RAISED_TO_POWER:
        return 90;

        case MULTIPLY: case DIVIDE:
        return 80 + left_associative_addition;

        case PLUS: case MINUS:
        return 70 + left_associative_addition;

        case WITHIN: case OUTSIDE:
        return 40 + left_associative_addition;

        case MAXIMIZE: case MINIMIZE:
        return 30 + left_associative_addition;

        case LT: case GT: case LE: case GE: case EQ: case NE: 
        return 20 + left_associative_addition;

        case AND: case OR:
        return 10 + left_associative_addition;

        default:
        return -5;


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
        op_node = make_terminal(parent, op);

        // find what precedence is our new minimum - if the operator is left-associative, it is one more than it's normal precedence
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
        case MINUS: 
        {
            PNode negate_node(std::make_shared<Node>(NEGATE, parent));
            negate_node->add_child(parse_primary_expression(negate_node));
            return negate_node;
        }
        case NOT:
        {
            node->add_child(parse_primary_expression(node));
            return node;
        }
        case OPEN_PAREN:
        {
            PNode subexpression = precedence_climber(parent, 0);
            lexer->expect_and_consume(CLOSE_PAREN);
            return subexpression;
        }

        case OPEN_CURLY_BRACE:
        {
            // make a node representing what the particle list function will end up being
            PNode terminal(std::make_shared<Node>(TERMINAL, parent));
            PNode particle_list(std::make_shared<Node>(PARTICLE_LIST, terminal));

            parse_particle_list(particle_list);
            terminal->add_child(particle_list);
            
            lexer->expect_and_consume(CLOSE_CURLY_BRACE);
            terminal->set_token(lexer->next());
            return terminal;
        }

        case OPEN_SQUARE_BRACE:
        {
            PNode interval(std::make_shared<Node>(INTERVAL, parent)); 
            interval->add_child(parse_primary_expression(interval));
            if (lexer->peek(0)->get_token_type() == COMMA) lexer->expect_and_consume(COMMA);
            interval->add_child(parse_primary_expression(interval));
            lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            return interval;
        }

        case SORT:
        {
            // E -> sort (E, ascend)
            // E -> sort (E, descend)
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(precedence_climber(parent, 0));
            lexer->expect_and_consume(COMMA);
            node->add_child(make_terminal(node, lexer->next()));
            lexer->expect_and_consume(CLOSE_PAREN);
            
            return node;
        }

        case ANYOCCURRENCES:
        {
            // E -> anyoccurances (E in E)
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(precedence_climber(node, 5));
            lexer->expect_and_consume(WITHIN);
            node->add_child(precedence_climber(node, 5));
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }   

        case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: 
        {
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(precedence_climber(parent, 0));
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: case CHARGE: case MASS:
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case JET_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case DZ:
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case MSOFTDROP: case THETA: 
        case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case PT: case PZ: case DR: case DPHI: case DETA: case DR_HADAMARD: case DPHI_HADAMARD: case DETA_HADAMARD: case NUMOF: case HT: case FMT2: case FMTAUTAU: case APLANARITY: case SPHERICITY:
        {
            if (lexer->peek(0)->get_token_type() != OPEN_PAREN) {
                // the next token is not an open parenthesis - this is not a function call per se, so either the argument is implicit or this is being used in reverse order in some way. That's not our problem here, so we just save that token.
                return node;
            }

            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(std::make_shared<Node>(PARTICLE_LIST, node));
            parse_particle_list(particle_list);
            node->add_child(particle_list);

            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
        
        case FHEMISPHERE: 
        {
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_id(node));
            lexer->expect_and_consume(COMMA);

            auto int1 = lexer->next();
            if  (int1->get_token_type() != INTEGER) raise_parsing_exception("FHemisphere requires integer argument in position 2", int1);
            node->add_child(make_terminal(node, int1));

            lexer->expect_and_consume(COMMA);

            auto int2 = lexer->next();
            if  (int2->get_token_type() != INTEGER) raise_parsing_exception("FHemisphere requires integer argument in position 3", int2);
            node->add_child(make_terminal(node, int2));

            lexer->expect_and_consume(CLOSE_PAREN);

            return node;
        }

        case FMEGAJETS:  case FMR:
        {
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_id(node));
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
        case FMTR:
        {
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_id(node));
            lexer->expect_and_consume(COMMA);
            if (lexer->peek(0)->get_token_type() == METLV) {
                node->add_child(make_terminal(node, lexer->next()));
            } else {
                node->add_child(parse_id(node));
            }
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
        case TTBAR_NNLOREC:
        {
            lexer->expect_and_consume(OPEN_PAREN);

            auto int1 = lexer->next();
            if (!is_numerical(int1->get_token_type())) raise_parsing_exception("Only numerical arguments are allowed in position 1 of NNLO rec", int1);

            auto int2 = lexer->next();
            if (!is_numerical(int2->get_token_type())) raise_parsing_exception("Only numerical arguments are allowed in position 2 of NNLO rec", int2);            
            
            auto int3 = lexer->next();
            if (!is_numerical(int3->get_token_type())) raise_parsing_exception("Only numerical arguments are allowed in position 3 of NNLO rec", int3);            
            
            auto int4 = lexer->next();
            if (!is_numerical(int4->get_token_type())) raise_parsing_exception("Only numerical arguments are allowed in position 4 of NNLO rec", int4);

            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }

        case METSIGNIF: case ALL: case NONE: case TRGM: case TRGE: case EVENT_NO: case RUN_NO: case LB_NO: case MC_CHANNEL_NUMBER: case HF_CLASSIFICATION: case RUNYEAR:
        {
            return node;
        }

        case STRING: case VARNAME:
        {
            // here, we are met with a token that isn't any other known form. If it is immediately followed by parentheses, then this is probably some external function. 
            if (lexer->peek(1)->get_token_type() == OPEN_PAREN) {
                PNode func(std::make_shared<Node>(USER_FUNCTION, parent));
                node->set_parent(func);

                lexer->expect_and_consume(OPEN_PAREN);
                parse_variable_list(node);
                lexer->expect_and_consume(CLOSE_PAREN);
                return func;
            }
            // otherwise, it is unclear what this is other than just some variable name - we will leave it like that
            return node;
        }
        case MIN: case MAX: 
        {
            lexer->expect_and_consume(OPEN_PAREN);
            parse_variable_list(node);
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
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
    
    PNode expression(std::make_shared<Node>(EXPRESSION, parent));
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
