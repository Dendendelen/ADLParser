#include "parser.hpp"
#include <iterator>
#include <memory>

#include <iostream>

#include "exceptions.hpp"
#include "node.hpp"
#include "tokens.hpp"



PNode make_terminal(PNode parent, PToken tok) {
    return PNode(new Node(TERMINAL, parent, tok));
}

void add_two_terminal_children(PNode parent, PToken one, PToken two) {
    PNode op(new Node(TERMINAL, parent));
    parent->add_child(op);
    op->set_token(one);

    PNode source(new Node(TERMINAL, parent));
    parent->add_child(source);
    source->set_token(two); 
}

void add_two_nested_terminal_children(PNode parent, PToken one, PToken two) {
    PNode one_node(new Node(TERMINAL, parent, one));
    parent->add_child(one_node);

    PNode two_node(new Node(TERMINAL, one_node, two));
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
        switch(tok->get_token()) {
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
            default:
                return;
        }

}

/*
INFO productions:
---

    INFO -> adlinfo ID INITIALIZATIONS

*/

PNode Parser::parse_info(PNode parent) {

    PNode info(new Node(INFO, parent));

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

    PNode count(new Node(COUNT_FORMAT, parent));

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

    PNode definition(new Node(DEFINITION, parent));

    lexer->expect_and_consume(DEF);
    definition->add_child(parse_id(definition));
    auto eq_tok = lexer->next();

    // DEFINITION -> def ID = DEF_RVALUE
    // DEFINITION -> def ID : DEF_RVALUE
    if (eq_tok->get_token() != ASSIGN && eq_tok->get_token() != COLON) raise_parsing_exception("Unknown token for definition assignment, expected '=' or ':'", eq_tok);
    definition->add_child(parse_def_rvalue(definition));

    return definition;
}

/* OBJECT productions:
---

    OBJECT -> obj ID take OBJ_RVALUE 

    OBJECT -> obj ID : OBJ_RVALUE
 */
PNode Parser::parse_object(PNode parent) {

    PNode object(new Node(OBJECT, parent));
    
    lexer->expect_and_consume(OBJ);
    object->add_child(parse_id(object));
    auto tok = lexer->next();

    // OBJECT -> obj ID take OBJ_RVALUE 
    // OBJECT -> obj ID : OBJ_RVALUE    
    if (tok->get_token() != COLON && tok->get_token() != TAKE) raise_parsing_exception("Expected symbol for object definition, either ':' or TAKE", tok);
    parse_obj_rvalue(object);

    return object;
}

/* TABLE productions:
---
    TABLE -> table ID tabletype ID nvars integer errors BOOL BIN_OR_BOX_VALUES
*/
PNode Parser::parse_table(PNode parent) {

    // TABLE -> table ID tabletype ID nvars integer errors BOOL BIN_OR_BOX_VALUES
    PNode table(new Node(TABLE_DEF, parent));

    lexer->expect_and_consume(TABLE);
    table->add_child(parse_id(table));
    lexer->expect_and_consume(TABLETYPE);
    table->add_child(parse_id(table));
    lexer->expect_and_consume(NVARS);

    auto tok = lexer->next();

    if (tok->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to specify NVars", tok);
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
    PNode region(new Node(REGION, parent));

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
    
    switch (next->get_token()) {

        // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
        case SKIP_HISTO: case SKIP_EFFS: case ADLINFO: case PAP_TITLE: case PAP_EXPERIMENT: case PAP_ID: case PAP_PUBLICATION: case PAP_SQRTS: case PAP_LUMI: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA: case SYSTEMATIC: case TRGE: case TRGM: 
            parent->add_child(parse_initialization(parent));
            parse_initializations(parent);
            return;

        // INITIALIZATONS -> epsilon
        default:
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
    
    switch (next->get_token()) {

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
    PNode histo_list(new Node(HISTO_LIST, parent));

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
    
    switch (next->get_token()) {

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

    switch(tok->get_token()) {

        // INITIALIZATION -> trge = integer
        // INITIALIZATION -> trgm = integer
        // INITIALIZATION -> skph = integer
        // INITIALIZATION -> skpe = integer
        case TRGE: case TRGM: case SKIP_HISTO: case SKIP_EFFS:
        {   // Consume terminal equals
            lexer->expect_and_consume(ASSIGN);
            
            PToken next = lexer->next();
            if (next->get_token() != INTEGER) raise_parsing_exception("Invalid non-integer assignment", next);

            PNode integer_target(new Node(TERMINAL, parent, tok));
            integer_target->add_child(make_terminal(integer_target, next));
            return integer_target;
        } 

        // INITIALIZATION -> pap_lumi number
        // INITIALIZATION -> pap_sqrts number
        case PAP_LUMI: case PAP_SQRTS:
        {
            PToken next = lexer->next();
            if (!is_numerical(next->get_token())) raise_parsing_exception("Non-numerical value given for numerical field", next); 

            PNode number_target(new Node(TERMINAL, parent, tok));
            number_target->add_child(make_terminal(number_target, next));
            return number_target;
        }

        // INITIALIZATION -> pap_experiment ID
        case PAP_EXPERIMENT: 
        {
            PNode experiment(new Node(TERMINAL, parent, tok));
            experiment->add_child(parse_id(experiment));
            return experiment;
        }

        // INITIALIZATION -> systematic BOOL string string SYST_VTYPE
        case SYSTEMATIC:
        {
            PNode systematic(new Node(TERMINAL, parent, tok));

            systematic->add_child(parse_bool(systematic));
            PToken next = lexer->next();
            PToken next2 = lexer->next();

            if (next->get_token() != STRING) raise_parsing_exception("Invalid systematic up identifier, expected string", next);
            if (next->get_token() != STRING) raise_parsing_exception("Invalid systematic down identifier, expected string", next2);

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
            PNode description_target(new Node(TERMINAL, parent, tok));

            description_target->add_child(parse_description(description_target));
            return description_target;
        }

        // should never be reached under normal conditions
        default:
            raise_parsing_exception("Invalid token in info block", tok);
            return PNode(new Node(AST_ERROR, parent));
    }

}


/* COUNT_PROCESS productions:
---

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE

    COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE, ERR_TYPE

*/
PNode Parser::parse_count_process(PNode parent) {

    PNode count_process(new Node(COUNT_PROCESS, parent));;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION
    lexer->expect_and_consume(PROCESS);
    count_process->add_child(parse_id(count_process));
    
    // consume comma
    lexer->expect_and_consume(COMMA, "Invalid end of argument. Needs at least 2 arguments separated by commas");
    count_process->add_child(parse_description(count_process));
    if (lexer->peek(0)->get_token() != COMMA) return count_process;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE
    lexer->expect_and_consume(COMMA);
    count_process->add_child(parse_err_type(count_process));
    if (lexer->peek(0)->get_token() != COMMA) return count_process;

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
    PNode histo(new Node(HISTOLIST_HISTOGRAM, parent));
    parse_histogram(histo);
    return histo;
}

/* DEF_RVALUE productions:
---

        DEF_RVALUE -> { VARIABLE_LIST }

        DEF_RVALUE -> OME ( DESCRIPTION, {VARIABLE_LIST}, INDEX)

        DEF_RVALUE -> constituents PARTICLE_LIST

        DEF_RVALUE -> constituents PARTICLE_LIST

        DEF_RVALUE -> add PARTICLE_LIST

        DEF_RVALUE -> particle_keyword PARTICLE_LIST

        DEF_RVALUE -> E

 */
PNode Parser::parse_def_rvalue(PNode parent) {

    auto tok = lexer->peek(0);
    
    switch(tok->get_token()) {

        // DEF_RVALUE -> { VARIABLE_LIST }
        case OPEN_CURLY_BRACE:
        {
            lexer->expect_and_consume(OPEN_CURLY_BRACE);

            PNode variable_list(new Node(VARIABLE_LIST, parent));
            parse_variable_list(variable_list);

            lexer->expect_and_consume(CLOSE_CURLY_BRACE);
            return variable_list;
        }

        // DEF_RVALUE -> OME ( DESCRIPTION, {VARIABLE_LIST}, INDEX)
        case OME:
        {   
            auto ome = make_terminal(parent, tok);            
            lexer->expect_and_consume(OME);

            lexer->expect_and_consume(OPEN_PAREN);

            ome->add_child(parse_description(ome));
            lexer->expect_and_consume(COMMA);

            lexer->expect_and_consume(OPEN_CURLY_BRACE);
            PNode var_list(new Node(VARIABLE_LIST, ome));
            parse_variable_list(var_list);
            lexer->expect_and_consume(CLOSE_CURLY_BRACE);

            lexer->expect_and_consume(COMMA);

            auto index = lexer->next();
            if (index->get_token() != INTEGER) raise_parsing_exception("Integer required for indexing", index);

            ome->add_child(make_terminal(ome, index));

            lexer->expect_and_consume(CLOSE_PAREN);
            return ome;
        }

        // DEF_RVALUE -> constituents PARTICLE_LIST
        case CONSTITUENTS:
        {
            auto constituents = make_terminal(parent, tok);

            PNode particle_list(new Node(PARTICLE_LIST, constituents));
            parse_particle_list(particle_list);
            constituents->add_child(particle_list);

            return constituents;
        }

        // DEF_RVALUE -> add PARTICLE_LIST
        // DEF_RVALUE -> particle_keyword PARTICLE_LIST
        case ADD: case PARTICLE_KEYWORD:
        {
            auto add_particles = make_terminal(parent, lexer->next());
            
            PNode particle_list(new Node(PARTICLE_LIST, add_particles));
            parse_particle_list(particle_list);
            add_particles->add_child(particle_list);

            return add_particles;
        }

        // illegal to have a particle without the "particle" keyword due to potential parsing ambiguity
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: 
        case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV:
            raise_parsing_exception("Cannot use a particle in a definition without specifying the \"particle\" keyword", tok);
            return PNode(new Node(AST_ERROR, parent));
        
        case STRING: case VARNAME:
            if (lexer->peek(1)->get_token() == OPEN_SQUARE_BRACE || lexer->peek(1)->get_token() == UNDERSCORE)
            {
                raise_parsing_exception("Cannot use a particle in a definition without specifying the \"particle\" keyword", tok);
                return PNode(new Node(AST_ERROR, parent));
            }
            // intentional fall-through

        // DEF_RVALUE -> E
        default:
            // assume this is an expression if the other components have not succeeded in their production
            return parse_expression(parent);
    }

}


/* OBJ_RVALUE productions:
---

    OBJ_RVALUE -> electron CRITERIA

    OBJ_RVALUE -> muon CRITERIA

    OBJ_RVALUE -> tau CRITERIA

    OBJ_RVALUE -> gen CRITERIA

    OBJ_RVALUE -> photon CRITERIA

    OBJ_RVALUE -> jet CRITERIA

    OBJ_RVALUE -> fjet CRITERIA

    OBJ_RVALUE -> lepton CRITERIA

    OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE)

    OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE, LEPTON_TYPE)

    OBJ_RVALUE -> union (ID, ID)

    OBJ_RVALUE -> comb ( PARTICLE_LIST ) HAMHUM CRITERIA

    OBJ_RVALUE -> ID CRITERIA

 */
void Parser::parse_obj_rvalue(PNode parent) {
    
    auto tok = lexer->peek(0);

    switch(tok->get_token()) {

        // OBJ_RVALUE -> electron CRITERIA
        // OBJ_RVALUE -> muon CRITERIA
        // OBJ_RVALUE -> tau CRITERIA
        // OBJ_RVALUE -> gen CRITERIA
        // OBJ_RVALUE -> photon CRITERIA
        // OBJ_RVALUE -> jet CRITERIA
        // OBJ_RVALUE -> fjet CRITERIA
        // OBJ_RVALUE -> lepton CRITERIA
        case ELECTRON: case MUON: case TAU: case GEN: case PHOTON: case JET: case FJET: case LEPTON:
        {
            PNode type = make_terminal(parent, lexer->next());
            parent->add_child(type);
            parse_criteria(parent);
            return;
        }

        // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE)
        // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE, LEPTON_TYPE)
        // OBJ_RVALUE -> union (ID, ID)
        case UNION:
        {
            PNode union_type = make_terminal(parent, lexer->next());

            lexer->expect_and_consume(OPEN_PAREN);
            auto next = lexer->peek(0);
                
            if (next->get_token() == ELECTRON || next->get_token() == MUON || next->get_token() == TAU) {
                // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE)
                union_type->add_child(parse_lepton(union_type));
                lexer->expect_and_consume(COMMA, "Union needs at least two elements comma-separated, token does not match up with that");
                union_type->add_child(parse_lepton(union_type));

                // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE, LEPTON_TYPE)
                if (lexer->peek(0)->get_token() == COMMA) {
                    lexer->expect_and_consume(COMMA);
                    union_type->add_child(parse_lepton(union_type));
                }

            // OBJ_RVALUE -> union (ID, ID)
            } else if (next->get_token() == STRING || next->get_token() == VARNAME) {
                union_type->add_child(parse_id(union_type));
                lexer->expect_and_consume(COMMA, "Union needs at least two elements comma-separated, token does not match up with that");

                union_type->add_child(parse_id(union_type));

            } else {
                raise_parsing_exception("Invalid type specified in union, union only accepts leptons or variable IDs", next);
            }

            lexer->expect_and_consume(CLOSE_PAREN);

            parent->add_child(union_type);
            return;
        }

        // OBJ_RVALUE -> comb ( PARTICLE_LIST ) HAMHUM CRITERIA
        case COMB:
        {
            PNode comb_type = make_terminal(parent, lexer->next());

            parent->add_child(comb_type);

            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(new Node(PARTICLE_LIST, comb_type));
            comb_type->add_child(particle_list);

            parse_particle_list(particle_list);
            lexer->expect_and_consume(CLOSE_PAREN);

            comb_type->add_child(parse_hamhum(comb_type));
            parse_criteria(comb_type);
            return;

        }

        // OBJ_RVALUE -> ID CRITERIA
        case STRING: case VARNAME:
        {
            PNode id = parse_id(parent);
            parent->add_child(id);
            parse_criteria(parent);
            return;
        }
        default:
            raise_parsing_exception("Invalid rvalue for an object definition", tok);
    }
}

/* BOOL productions:
---

    BOOL -> true

    BOOL -> false

 */
PNode Parser::parse_bool(PNode parent) {
    auto tok = lexer->next();
    if (!(tok->get_token() == TRUE) && !(tok->get_token() == FALSE)) raise_parsing_exception("Excepted boolean, but token is not interpretable as a boolean", tok);

    PNode boolean(new Node(TERMINAL, parent));
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
    if (next->get_token() != INTEGER && next->get_token() != STRING && next->get_token() != VARNAME) { 
        raise_parsing_exception("Invalid ID, allowed types are variable-type names and strings", next);
    } else if (next->get_token() == INTEGER) raise_parsing_exception("Invalid ID, integers for ID must be put in quotes.", next);
    return make_terminal(parent, next);
}

/* DESCRIPTION productions:
---

    DESCRIPTION -> string DESCRIPTION

    DESCRIPTION -> string

 */
PNode Parser::parse_description(PNode parent) {

    auto tok = lexer->next();
    PNode description_str(new Node(TERMINAL, parent, tok));
    if (tok->get_token() != STRING) {
        raise_parsing_exception("Excepted string for description", tok);
    }
    // DESCRIPTION -> STRING DESCRIPTION
    if (lexer->peek(0)->get_token() == STRING) {
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
    if (tok->get_token() == ERR_SYST || tok->get_token() == ERR_STAT) {
        return make_terminal(parent, tok);
    }
    raise_parsing_exception("Excepted error type 'syst' or 'stat'", tok);
    return PNode(new Node(AST_ERROR, parent));
}

/* REGION_COMMANDS productions
---

    REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS

    REGION_COMMANDS -> epsilon
 */
void Parser::parse_region_commands(PNode parent) {

    auto tok = lexer->peek(0);

    switch(tok->get_token()) {
        case SELECT: case REJEC: case BINS: case BIN: case SAVE: case PRINT: case WEIGHT: case COUNTS: case HISTO: case SORT: case USE:
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

    REGION_COMMAND_SELECT -> lepsf

    REGION_COMMAND_SELECT -> btagsf

    REGION_COMMAND_SELECT -> xlumicorrsf

    REGION_COMMAND_SELECT -> hlt DESCRIPTION

    REGION_COMMAND_SELECT -> applyhm (ID (E, E) eq integer)

    REGION_COMMAND_SELECT -> applyhm (ID (E) eq integer)
    
    REGION_COMMAND_SELECT -> IF_OR_CONDITION

*/
PNode Parser::parse_region_command_select(PNode parent) {

    auto next = lexer->peek(0);
    switch(next->get_token()) {

        // REGION_COMMAND_SELECT -> none
        // REGION_COMMAND_SELECT -> all
        // REGION_COMMAND_SELECT -> lepsf
        // REGION_COMMAND_SELECT -> btagsf
        // REGION_COMMAND_SELECT -> xlumicorrsf
        case NONE: case ALL: case LEP_SF: case BTAGS_SF: case XSLUMICORR_SF:
        {
            lexer->next();
            PNode cond(new Node(CONDITION, parent));
            cond->add_child(make_terminal(cond, next));
            return cond;
        }
        
        // REGION_COMMAND_SELECT -> hlt DESCRIPTION
        case HLT:
        {
            auto node = make_terminal(parent, next);
            lexer->next();
            node->add_child(parse_description(node));
            return node;
        }

        // REGION_COMMAND_SELECT -> applyhm (ID (E, E) eq integer)
        // REGION_COMMAND_SELECT -> applyhm (ID (E) eq integer)
        case APPLY_HM:
        {
            auto node = make_terminal(parent, next);
            lexer->next();
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_id(node));
            lexer->expect_and_consume(OPEN_PAREN);
            node->add_child(parse_expression(node));

            if (lexer->peek(0)->get_token() == COMMA) {
                lexer->expect_and_consume(COMMA);
                node->add_child(parse_expression(node));
            }

            lexer->expect_and_consume(CLOSE_PAREN);
            lexer->expect_and_consume(EQ);
            auto integer = lexer->next();

            if (integer->get_token() != INTEGER) raise_parsing_exception("Needs integer type", integer);
            node->add_child(make_terminal(node, integer));

            return node;
        }

        // REGION_COMMAND_SELECT -> IF_OR_CONDITION
        default:
            return parse_if_or_condition(parent);
    }
}


/* REGION_COMMAND productions:
---

    REGION_COMMAND -> select REGION_COMMAND_SELECT

    REGION_COMMAND -> weight ID number

    REGION_COMMAND -> weight ID ID

    REGION_COMMAND -> weight ID ID (E, E)

    REGION_COMMAND -> weight ID ID (E)

    REGION_COMMAND -> weight ID (E)

    REGION_COMMAND -> bin CONDITION

    REGION_COMMAND -> rejec CONDITION

    REGION_COMMAND -> use ID

    REGION_COMMAND -> take ID

    REGION_COMMAND -> bins ID BIN_OR_BOX_VALUES

    REGION_COMMAND -> save ID

    REGION_COMMAND -> save ID csv VARIABLE_LIST

    REGION_COMMAND -> counts ID COUNTS

    REGION_COMMAND -> histo HISTOGRAM

    REGION_COMMAND -> sort EXPRESSION ascend

    REGION_COMMAND -> sort EXPRESSION descend
 */

PNode Parser::parse_region_command(PNode parent) {
    
    auto tok = lexer->next();
    switch(tok->get_token()) {
        
        // REGION_COMMAND -> select REGION_COMMAND_SELECT
        case SELECT:
        {
            PNode node(new Node(REGION_SELECT, parent));
            node->add_child(parse_region_command_select(parent));
            return node;
        }

        // REGION_COMMAND -> weight ID number
        // REGION_COMMAND -> weight ID ID
        // REGION_COMMAND -> weight ID ID (E, E)
        // REGION_COMMAND -> weight ID ID (E)
        // REGION_COMMAND -> weight ID (E)
        case WEIGHT:
        {
            PNode node(new Node(WEIGHT_CMD, parent));
            node->add_child(parse_id(node));

            auto next = lexer->peek(0);
            if (next->get_token() == OPEN_PAREN) {
                lexer->expect_and_consume(OPEN_PAREN);
                node->add_child(parse_expression(node));
                lexer->expect_and_consume(CLOSE_PAREN);
            } else if (is_numerical(next->get_token())){
                lexer->next();
                node->add_child(make_terminal(node, next));
            } else {
                // REGION_COMMAND -> weight ID ID (E, E)
                // REGION_COMMAND -> weight ID ID (E)
                node->add_child(parse_id(node));
                lexer->expect_and_consume(OPEN_PAREN);
                node->add_child(parse_expression(node));
                if (lexer->peek(0)->get_token() == COMMA) {
                    // REGION_COMMAND -> weight ID ID (E, E)
                    lexer->expect_and_consume(COMMA);
                    node->add_child(parse_expression(node));
                }
                lexer->expect_and_consume(CLOSE_PAREN);
            }
            return node;
        }

        // REGION_COMMAND -> rejec CONDITION
        case REJEC:
        {
            PNode node(new Node(REGION_REJECT, parent));
            node->add_child(parse_condition(node));
            return node;
        }

        // REGION_COMMAND -> bin CONDITION
        case BIN:
        {
            PNode node(new Node(BIN_CMD, parent));
            node->add_child(parse_condition(node));
            return node;
        }

        // REGION_COMMAND -> use ID
        // REGION_COMMAND -> take ID
        case USE: case TAKE:
        {
            PNode node(new Node(REGION_USE, parent));
            node->add_child(parse_id(node));
            return node;
        }

        // REGION_COMMAND -> bins ID BIN_OR_BOX_VALUES
        case BINS:
        {
            PNode node(new Node(BINS_CMD, parent));
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
            if (next->get_token() == CSV) {
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
            if (lexer->peek(0)->get_token() == USE) {
                lexer->expect_and_consume(USE);
                PNode histo_use(new Node(HISTO_USE, parent));
                histo_use->add_child(parse_id(histo_use));
                return histo_use;
            }
            PNode histo(new Node(HISTOGRAM, parent));
            parse_histogram(histo);
            return histo;
        }

        // REGION_COMMAND -> sort EXPRESSION ascend
        // REGION_COMMAND -> sort EXPRESSION descend
        case SORT:
        {
            PNode sort(make_terminal(parent, tok));
            sort->add_child(parse_expression(sort));
            
            auto next = lexer->next();
            if (next->get_token() != ASCEND && next->get_token() != DESCEND) raise_parsing_exception("Token after a sort expression must specify ascending or descending", next);
            sort->add_child(make_terminal(sort, next));
            return sort;
        }

        default:
            raise_parsing_exception("Unexpected token in region block", tok);
            return PNode(new Node(AST_ERROR, parent));
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
    if (integer_tok_1->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", integer_tok_1);
    parent->add_child(make_terminal(parent, integer_tok_1));
    lexer->expect_and_consume(COMMA);

    // number,
    auto lower_value_tok_1 = lexer->next();
    if (!is_numerical(lower_value_tok_1->get_token())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_1);
    parent->add_child(make_terminal(parent, lower_value_tok_1));
    lexer->expect_and_consume(COMMA);

    // number,
    auto upper_value_tok_1 = lexer->next();
    if (!is_numerical(upper_value_tok_1->get_token())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
    parent->add_child(make_terminal(parent, upper_value_tok_1));
    lexer->expect_and_consume(COMMA);

    // use to check if the list continues 
    bool is_2d = false;
    auto discriminant = lexer->peek(1);

    if (discriminant->get_token() == COMMA) {

        // REGION_COMMANDS -> parent ID, DESCRIPTION, integer, number, number, integer, number, number, EXPRESSION, EXPRESSION
        is_2d = true;

        // integer,
        auto integer_tok_2 = lexer->next();
        if (integer_tok_2->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", integer_tok_2);
        parent->add_child(make_terminal(parent, integer_tok_2));
        lexer->expect_and_consume(COMMA);

        // number,
        auto lower_value_tok_2 = lexer->next();
        if (!is_numerical(lower_value_tok_2->get_token())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_2);
        parent->add_child(make_terminal(parent, lower_value_tok_2));
        lexer->expect_and_consume(COMMA);

        // number,
        auto upper_value_tok_2 = lexer->next();
        if (!is_numerical(upper_value_tok_2->get_token())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
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
    switch(tok->get_token()) {

        // VARIABLE_LIST -> epsilon 
        // this is the follow set for VARIABLE_LIST, and none of them are in the first set of EXPRESSION
        case CLOSE_CURLY_BRACE: case CLOSE_PAREN: case COLON: case OBJ:  case SELECT: case PRINT: case HISTO: case REJEC: case BINS: case BIN: case SAVE: case WEIGHT: case COUNTS: case SORT: case ADLINFO: case COUNTSFORMAT: case DEF: case TABLE: case ALGO: case COMMA: case USE:
            return;            

        // VARIABLE_LIST -> EXPRESSION VARIABLE_LIST
        // VARIABLE_LIST -> EXPRESSION, VARIABLE_LIST
        default:
            parent->add_child(parse_expression(parent));
            auto next = lexer->peek(0);
            if (next->get_token() == COMMA) {
                lexer->expect_and_consume(COMMA);
            }
            parse_variable_list(parent);
            return;
    }
}

/* IF_OR_CONDITION productions:
---

    IF_OR_CONDITION -> CONDITION

    IF_OR_CONDITION -> CONDITION ? ACTION : ACTION
 */
PNode Parser::parse_if_or_condition(PNode parent) {

    PNode node(new Node(IF, parent));

    node->add_child(parse_condition(node));

    auto tok = lexer->peek(0);
    if (tok->get_token() == QUESTION) {
        lexer->expect_and_consume(QUESTION);
        node->add_child(parse_action(node));
        lexer->expect_and_consume(COLON);
        node->add_child(parse_action(node));
    }

    return node;
}

/* ACTION productions:
---

    ACTION -> print VARIABLE_LIST

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
    switch (tok->get_token()) {

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
            if (lexer->peek(1)->get_token() == OPEN_SQUARE_BRACE) {
                lexer->expect_and_consume(OPEN_SQUARE_BRACE);
                apply_ptf->add_child(parse_expression(apply_ptf));

                if (lexer->peek(0)->get_token() == COMMA) {
                    lexer->expect_and_consume(COMMA);
                    apply_ptf->add_child(parse_expression(apply_ptf));

                    if (lexer->peek(0)->get_token() == COMMA) {
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
            if (lexer->peek(0)->get_token() == COMMA) {
                lexer->expect_and_consume(COMMA);
                apply_hm->add_child(parse_expression(apply_hm));
            }
            lexer->expect_and_consume(CLOSE_PAREN);
            lexer->expect_and_consume(EQ);

            auto next = lexer->next();
            if (next->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed for comparison in applying HM", tok);

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
    if (!is_numerical(tok->get_token())) raise_parsing_exception("Needs a numerical value for box argument", tok);

    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    if (is_numerical(next->get_token())) parse_bin_or_box_values(parent);
}

/* COUNTS productions:
---

    COUNTS -> COUNT, COUNTS

    COUNTS -> COUNT
 */
void Parser::parse_counts(PNode parent) {
    // COUNTS -> COUNT
    // COUNTS -> COUNT, COUNTS

    PNode count(new Node(COUNT, parent));
    parse_count(count);
    parent->add_child(count);

    if (lexer->peek(0)->get_token() == COMMA) {
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
    if (!is_numerical(tok->get_token())) raise_parsing_exception("A numerical type is needed for counts", tok);

    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    switch (next->get_token()) {
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

/* PARTICLE_LIST productions:
---

    PARTICLE_LIST -> PARTICLE + PARTICLE_LIST

    PARTICLE_LIST -> PARTICLE, PARTICLE_LIST

    PARTICLE_LIST -> PARTICLE PARTICLE_LIST

    PARTICLE_LIST -> PARTICLE

*/
void Parser::parse_particle_list(PNode parent) {

    parent->add_child(parse_particle(parent));
    auto tok = lexer->peek(0);

    switch (tok->get_token()) {

        // PARTICLE_LIST -> PARTICLE + PARTICLE_LIST
        case PLUS:
            lexer->expect_and_consume(PLUS);
            parse_particle_list(parent);
            return;

        // PARTICLE_LIST -> PARTICLE, PARTICLE_LIST
        case COMMA:
            lexer->expect_and_consume(COMMA);
            parse_particle_list(parent);
            return;

        // PARTICLE_LIST -> PARTICLE PARTICLE_LIST
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: 
        case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV: case STRING: case VARNAME: case MINUS:
            parse_particle_list(parent);
            return;

        default:
        // PARTICLE_LIST -> PARTICLE
            return;
    }
}

/* PARTICLE productions:
---

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

    switch (tok->get_token()) {

        // PARTICLE -> gen constituents
        // PARTICLE -> jet constituents
        // PARTICLE -> fjet constituents
        case GEN: case JET: case FJET:
        {
            if (lexer->peek(1)->get_token()==CONSTITUENTS) {
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

        // PARTICLE -> ID INDEX
        default:
            {
                PNode particle = parse_id(parent);
                particle->add_child(parse_index(particle));
                return particle;
            }
    }

}

/* INDEX productions:
---

    INDEX -> _integer

    INDEX -> [integer]

    INDEX -> [integer:integer]

    INDEX -> _integer:integer
    
    INDEX -> epsilon

 */
PNode Parser::parse_index(PNode parent) {

    auto tok = lexer->peek(0);

    switch(tok->get_token()) {

        // INDEX -> _integer
        // INDEX -> [integer]
        // INDEX -> [integer:integer]
        // INDEX -> _integer:integer        
        case UNDERSCORE: case OPEN_SQUARE_BRACE:
        {    
            lexer->next();
            PNode index(new Node(INDEX, parent));
            auto next = lexer->next();
            if (next->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to be used as indices", next);
            index->add_child(make_terminal(index, next));
            
            // INDEX -> [integer:integer]
            // INDEX -> _integer:integer 
            if (lexer->peek(0)->get_token() == COLON) {
                lexer->expect_and_consume(COLON);

                auto next2 = lexer->next();
                if (next->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to be used as indices", next);

                index->add_child(make_terminal(index, next2));
            }
            if (tok->get_token() == OPEN_SQUARE_BRACE) {
                lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            }
            return index;

        }

        // INDEX -> epsilon
        default:
            return PNode(new Node(AST_EPSILON, parent));
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
    
    switch(tok->get_token()) {
        case ELECTRON: case MUON: case TAU:
            return make_terminal(parent, tok);
        default:
            raise_parsing_exception("This token must be a lepton type", tok);
            return PNode(new Node(AST_ERROR, parent));
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
    switch(tok->get_token()) {
        case SYST_WEIGHT_MC: case SYST_WEIGHT_JVT: case SYST_WEIGHT_PILEUP: case SYST_WEIGHT_LEPTON_SF: case SYST_WEIGHT_BTAG_SF: case SYST_TTREE:
            return make_terminal(parent, tok);
        default:
            raise_parsing_exception("Expected a valid systematics type", tok);
            return PNode(new Node(AST_ERROR, parent));
    }
}


/* HAMHUM productions:
---

    HAMHUM -> hamhum ID
 */
PNode Parser::parse_hamhum(PNode parent) {
    PNode hamhum(new Node(HAMHUM, parent));
    lexer->expect_and_consume(ALIAS);
    
    hamhum->add_child(parse_id(hamhum));
    return hamhum;
}

/* CRITERIA productions:
---

    CRITERIA -> CRITERION CRITERIA

    CRITERIA -> epsilon
*/
void Parser::parse_criteria(PNode parent) {

    auto tok = lexer->peek(0);
    switch(tok->get_token()) {
        case SELECT: case PRINT: case HISTO: case REJEC:
            parent->add_child(parse_criterion(parent));
            parse_criteria(parent);
            return;
        default:
            return;
    }
}

/* CRITERION productions:
---

    CRITERION -> select ACTION

    CRITERION -> histo ACTION

    CRITERION -> print VARIABLE_LIST

    CRITERION -> rejec CONDITION
 */
PNode Parser::parse_criterion(PNode parent) {

    auto tok = lexer->next();

    switch (tok->get_token()) {

        // CRITERION -> cmd ACTION
        case SELECT: case HISTO:
        {
            PNode node(new Node(OBJECT_SELECT, parent));
            node->add_child(parse_action(node));
            return node;
        }
        // CRITERION -> rejec CONDITION
        case REJEC:
        {
            PNode node(new Node(OBJECT_REJECT, parent, tok));
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
    PNode condition(new Node(CONDITION, parent));

    condition->add_child(precedence_climber(condition, parse_expression_helper(condition), 0));

    return condition;
}

int get_precedence(PToken tok) {
    switch(tok->get_token()) {

        case RAISED_TO_POWER:
        return 9;

        case MULTIPLY: case DIVIDE:
        return 8;

        case PLUS: case MINUS:
        return 7;

        case WITHIN: case OUTSIDE:
        return 4;

        case MAXIMIZE: case MINIMIZE:
        return 3;

        case LT: case GT: case LE: case GE: case EQ: case NE: 
        return 2;

        case AND: case OR:
        return 1;

        default:
        return -1;

    }
}

PNode Parser::precedence_climber(PNode parent, PNode lhs, int min_precedence) {
    auto lookahead = lexer->peek(0);
    PToken op;
    PNode op_node;
    while(get_precedence(lookahead) >= min_precedence) {
        op = lexer->next();
        op_node = make_terminal(parent, op);
        auto rhs = parse_expression_helper(op_node);
        lookahead = lexer->peek(0);
        while (get_precedence(lookahead) > get_precedence(op)){
            rhs = precedence_climber(op_node, rhs, get_precedence(op)+ 1);
            lookahead = lexer->peek(0);
        }
        op_node->add_child(lhs);
        op_node->add_child(rhs);
        lhs = op_node;
    }
        
    return lhs;
}

PNode Parser::parse_expression_helper(PNode parent) {

    auto tok = lexer->next();
    PNode node(make_terminal(parent, tok));

    switch(tok->get_token()) {
        case MINUS: 
        {
            PNode negate_node(new Node(NEGATE, parent));
            negate_node->add_child(parse_expression_helper(negate_node));
            return negate_node;
        }
        case NOT:
        {
            node->add_child(parse_expression_helper(node));
            return node;
        }
        case OPEN_PAREN:
        {
            PNode lhs = parse_expression_helper(parent);
            PNode subexpression = precedence_climber(parent, lhs, 0);
            lexer->expect_and_consume(CLOSE_PAREN);
            return subexpression;
        }

        case OPEN_CURLY_BRACE:
        {
            // make a node representing what the particle list function will end up being
            PNode terminal(new Node(TERMINAL, parent));
            PNode particle_list(new Node(PARTICLE_LIST, terminal));

            parse_particle_list(particle_list);
            terminal->add_child(particle_list);
            
            lexer->expect_and_consume(CLOSE_CURLY_BRACE);
            terminal->set_token(lexer->next());
            return terminal;
        }

        case OPEN_SQUARE_BRACE:
        {
            PNode interval(new Node(INTERVAL, parent)); 
            interval->add_child(parse_expression_helper(interval));
            if (lexer->peek(0)->get_token() == COMMA) lexer->expect_and_consume(COMMA);
            interval->add_child(parse_expression_helper(interval));
            lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            return interval;
        }

        case HSTEP: case DELTA: case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: 
        {
            lexer->expect_and_consume(OPEN_PAREN);
            PNode lhs = parse_expression_helper(node);
            node->add_child(precedence_climber(parent, lhs, 0));
            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: 
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case EDXY: case EDZ: case DZ: case VERTR: case VERZ: case VERY: case VERX: case VERT: 
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case ABS_ETA: case THETA: 
        case PTCONE: case ETCONE: case ABS_ISO: case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case PT: case PZ: case NBF: case DR: case DPHI: case DETA: case NUMOF: case HT: case FMT2: case FMTAUTAU: case APLANARITY: case SPHERICITY:
        {
            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(new Node(PARTICLE_LIST, node));
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
            if  (int1->get_token() != INTEGER) raise_parsing_exception("FHemisphere requires integer argument in position 2", int1);
            node->add_child(make_terminal(node, int1));

            lexer->expect_and_consume(COMMA);

            auto int2 = lexer->next();
            if  (int2->get_token() != INTEGER) raise_parsing_exception("FHemisphere requires integer argument in position 3", int2);
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
            if (lexer->peek(0)->get_token() == MET) {
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
            if (!is_numerical(int1->get_token())) raise_parsing_exception("Only numerical arguments are allowed in position 1 of NNLO rec", int1);

            auto int2 = lexer->next();
            if (!is_numerical(int2->get_token())) raise_parsing_exception("Only numerical arguments are allowed in position 2 of NNLO rec", int2);            
            
            auto int3 = lexer->next();
            if (!is_numerical(int3->get_token())) raise_parsing_exception("Only numerical arguments are allowed in position 3 of NNLO rec", int3);            
            
            auto int4 = lexer->next();
            if (!is_numerical(int4->get_token())) raise_parsing_exception("Only numerical arguments are allowed in position 4 of NNLO rec", int4);

            lexer->expect_and_consume(CLOSE_PAREN);
            return node;
        }

        case MET: case METSIGNIF: case ALL: case NONE: case TRGM: case TRGE: case EVENT_NO: case RUN_NO: case LB_NO: case MC_CHANNEL_NUMBER: case HF_CLASSIFICATION: case RUNYEAR:
        {
            return node;
        }

        case STRING: case VARNAME:
        {
            if (lexer->peek(1)->get_token() == OPEN_PAREN) {
                PNode func(new Node(USER_FUNCTION, parent));
                node->set_parent(func);

                lexer->expect_and_consume(OPEN_PAREN);
                PNode lhs = parse_expression_helper(node);
                node->add_child(precedence_climber(node, lhs, 0));
                lexer->expect_and_consume(CLOSE_PAREN);
                return func;
            }

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
            if(!is_numerical(tok->get_token())) raise_parsing_exception("Invalid token used in expression", tok);
            return precedence_climber(parent, node, 0);
    }
}

/* EXPRESSION productions:
---

All productions are done via precedence climbing. Stated grammar does not correctly specify the precedences, but they are accounted for.

---


 */
PNode Parser::parse_expression(PNode parent) {
    
    PNode expression(new Node(EXPRESSION, parent));
    expression->add_child(precedence_climber(expression, parse_expression_helper(expression), 0));

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
        std::cout << "    " << reserved_number_for_me << " [label=\"ID:" << node->get_ast_type() << "\"]" <<std::endl;
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