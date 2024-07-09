#include "parser.h"
#include <memory>

#include <iostream>

#include "exceptions.h"
#include "node.h"
#include "tokens.h"



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

void Parser::parse_input() {
    // INPUT -> BLOCKS
    
    PNode input_node = tree.get_root();
    parse_blocks(input_node);
}

void Parser::parse_blocks(PNode parent) {
    // BLOCKS -> INITIALIZATION BLOCKS
    // BLOCKS -> COUNT_FORMAT BLOCKS
    // BLOCKS -> DEFINITIONS_OBJECTS BLOCKS
    // BLOCKS -> COMMAND BLOCKS
    // BLOCKS -> epsilon

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
            // case CMD: case REJEC: case BINS: case ALGO: case SAVE: case PRINT: case WEIGHT: case COUNTS: case HISTO: case SORT: 
                // parent->add_child(parse_command(parent));
                return;
                
            // BLOCKS -> epsilon
            default:
                return;
        }

}

PNode Parser::parse_info(PNode parent) {
    // INFO -> adlinfo ID INITIALIZATIONS
    PNode info(new Node(INFO, parent));

    lexer->expect_and_consume(ADLINFO);
    info->add_child(parse_id(info));
    parse_initializations(info);

    return info;
}


PNode Parser::parse_count_format(PNode parent) {   
    PNode count(new Node(COUNT_FORMAT, parent));

    lexer->expect_and_consume(COUNTSFORMAT);
    count->add_child(parse_id(count));
    parse_count_processes(count);

    return count;
}


PNode Parser::parse_definition(PNode parent) {

    // DEFINITION -> def ID = DEF_RVALUE
    // DEFINITION -> def ID : DEF_RVALUE

    PNode definition(new Node(DEFINITION, parent));

    lexer->expect_and_consume(DEF);

    auto id_tok = lexer->next();
    if (id_tok->get_token() != ID) raise_parsing_exception("Expected an identifier after a definition keyword", id_tok);

    auto eq_tok = lexer->next();
    if (eq_tok->get_token() != ASSIGN && eq_tok->get_token() != COLON) raise_parsing_exception("Unknown token for definition assignment, expected '=' or ':'", id_tok);

    definition->add_child(parse_def_rvalue(definition));
    return definition;
}

PNode Parser::parse_object(PNode parent) {
    // OBJECT -> obj ID take OBJ_RVALUE 
    // OBJECT -> obj ID : OBJ_RVALUE

    PNode object(new Node(OBJECT, parent));
    
    lexer->expect_and_consume(OBJ);
    object->add_child(parse_id(object));

    auto tok = lexer->next();
    if (tok->get_token() != COLON && tok->get_token() != TAKE) raise_parsing_exception("Expected symbol for object definition, either ':' or TAKE", tok);

    object->add_child(parse_obj_rvalue(object));

    return object;
}

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

PNode Parser::parse_region(PNode parent) {
    // REGION -> algo ID REGION_COMMANDS
    PNode region(new Node(REGION, parent));

    lexer->expect_and_consume(ALGO);
    region->add_child(parse_id(region));
    parse_region_commands(region);

    return region;
}

void Parser::parse_initializations(PNode parent) {
    PToken next = lexer->peek(0);
    
    switch (next->get_token()) {
        case SKPH: case SKPE: case ADLINFO: case PAP_TITLE: case PAP_EXPERIMENT: case PAP_ID: case PAP_PUBLICATION: case PAP_SQRTS: case PAP_LUMI: case PAP_ARXIV: case PAP_DOI: case PAP_HEPDATA: case SYSTEMATIC: case TRGE: case TRGM: 
            // INITIALIZATIONS -> INITIALIZATION INITIALIZATIONS
            parent->add_child(parse_initialization(parent));
            parse_initializations(parent);
            return;
        default:
            // INITIALIZATONS -> epsilon
            return;
    }
}

void Parser::parse_count_processes(PNode parent) {
    PToken next = lexer->peek(0);
    
    switch (next->get_token()) {
        case PROCESS: 
            // COUNT_PROCESSES -> COUNT_PROCESS COUNT_PROCESSES
            parent->add_child(parse_count_process(parent));
            parse_count_processes(parent);
            return;
        default:
            // COUNT_PROCESSES -> epsilon
            return;
    }
}

PNode Parser::parse_initialization(PNode parent) {
    PToken tok = lexer->next();

    switch(tok->get_token()) {
        // INITIALIZATION -> trge = integer
        // INITIALIZATION -> trgm = integer
        // INITIALIZATION -> skph = integer
        // INITIALIZATION -> skpe = integer

        case TRGE: case TRGM: case SKPH: case SKPE:
        {   // Consume terminal equals
            lexer->expect_and_consume(ASSIGN);
            
            PToken next = lexer->next();
            if (next->get_token() != INTEGER) raise_parsing_exception("Invalid non-integer assignment", next);

            PNode integer_target(new Node(TERMINAL, parent, tok));
            integer_target->add_child(make_terminal(integer_target, next));
            return integer_target;
        } 

        // INITIALIZATION -> pap_lumi NUMBER
        // INITIALIZATION -> pap_sqrts NUMBER
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
        } break;

        // should never be reached under normal conditions
        default:
            raise_parsing_exception("Invalid token in info block", tok);
            return PNode(new Node(AST_ERROR, parent));
    }

}

PNode Parser::parse_count_process(PNode parent) {
    PNode count_process(new Node(COUNT_PROCESS, parent));;

    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION
    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE
    // COUNT_PROCESS -> PROCESS ID , DESCRIPTION, ERR_TYPE, ERR_TYPE

    lexer->expect_and_consume(PROCESS);
    count_process->add_child(parse_id(count_process));
    
    // consume comma
    lexer->expect_and_consume(COMMA, "Invalid end of argument. Needs at least 2 arguments separated by commas");
    count_process->add_child(parse_description(count_process));

    if (lexer->peek(0)->get_token() != COMMA) return count_process;
    // consume second comma
    lexer->expect_and_consume(COMMA);
    count_process->add_child(parse_err_type(count_process));

    if (lexer->peek(0)->get_token() != COMMA) return count_process;
    // consume third comma
    lexer->expect_and_consume(COMMA);
    count_process->add_child(parse_err_type(count_process));   

    return count_process;
}

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
            ome->add_child(parse_index(ome));

            lexer->expect_and_consume(CLOSE_PAREN);
            return ome;
        }

        // DEF_RVALUE -> CONSTITUENTS PARTICLE_LIST
        case CONSTITUENTS:
        {
            auto constituents = make_terminal(parent, tok);
            parse_particle_list(constituents);
            return constituents;
        }

        // DEF_RVALUE -> ADD PARTICLE_LIST
        case ADD:
        {
            auto add = make_terminal(parent, tok);
            parse_particle_list(add);
            return add;
        }

        // DEF_RVALUE -> PARTICLE
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV:
            return parse_particle(parent);
        
        // DEF_RVALUE -> E
        default:
            // assume this is an expression if the other components have not succeeded in their production
            return parse_expression(parent);
    }

}

PNode Parser::parse_obj_rvalue(PNode parent) {
    auto tok = lexer->next();

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
            PNode type = make_terminal(parent, tok);
            parse_criteria(type);
            return type;
        }

        // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE)
        // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE, LEPTON_TYPE)
        // OBJ_RVALUE -> union (ID, ID)
        case UNION:
        {
            PNode union_type = make_terminal(parent, tok);

            lexer->expect_and_consume(OPEN_PAREN);
            auto next = lexer->peek(0);

                
            // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE)
            // OBJ_RVALUE -> union (LEPTON_TYPE, LEPTON_TYPE, LEPTON_TYPE)
            if (next->get_token() == ELECTRON || next->get_token() == MUON || next->get_token() == TAU) {
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

            return union_type;
        }
        // OBJ_RVALUE -> comb ( PARTICLES ) HAMHUM CRITERIA
        case COMB:
        {
            PNode comb_type = make_terminal(parent, tok);

            lexer->expect_and_consume(OPEN_PAREN);

            PNode particle_list(new Node(PARTICLES, comb_type));
            comb_type->add_child(particle_list);

            parse_particle_list(particle_list);
            lexer->expect_and_consume(CLOSE_PAREN);

            comb_type->add_child(parse_hamhum(comb_type));
            parse_criteria(comb_type);

            return comb_type;
        }

        // OBJ_RVALUE -> ID CRITERIA
        case STRING: case VARNAME:
        {
            PNode id = parse_id(parent);
            parse_criteria(id);
            return id;
        }
        default:
            raise_parsing_exception("Invalid rvalue for an object definition", tok);
            return PNode(new Node(AST_ERROR, parent));
    }
}

PNode Parser::parse_bool(PNode parent) {
    auto tok = lexer->next();
    if (!(tok->get_token() == TRUE) && !(tok->get_token() == FALSE)) raise_parsing_exception("Excepted boolean, but token is not interpretable as a boolean", tok);

    PNode boolean(new Node(TERMINAL, parent));
    boolean->set_token(tok);
    return boolean;
}

PNode Parser::parse_id(PNode parent) {
    PToken next = lexer->next();
    if (next->get_token() != INTEGER && next->get_token() != STRING && next->get_token() != VARNAME) { 
        raise_parsing_exception("Invalid ID, allowed types are variable-type names and strings", next);
    } else if (next->get_token() == INTEGER) raise_parsing_exception("Invalid ID, integers for ID must be put in quotes.", next);
    return make_terminal(parent, next);
}

PNode Parser::parse_description(PNode parent) {
    // DESCRIPTION -> string DESCRIPTION
    // DESCRIPTION -> string

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


PNode Parser::parse_err_type(PNode parent) {
    auto tok = lexer->next();
    if (tok->get_token() == ERR_SYST || tok->get_token() == ERR_STAT) {
        return make_terminal(parent, tok);
    }
    raise_parsing_exception("Excepted error type 'syst' or 'stat'", tok);
    return PNode(new Node(AST_ERROR, parent));
}

void Parser::parse_region_commands(PNode parent) {
    // REGION_COMMANDS -> REGION_COMMAND REGION_COMMANDS
    // REGION_COMMANDS -> epsilon

    auto tok = lexer->peek(0);

    switch(tok->get_token()) {
        case CMD: case REJEC: case BINS: case SAVE: case PRINT: case WEIGHT: case COUNTS: case HISTO: case SORT: 
            parent->add_child(parse_region_command(parent));
            parse_region_commands(parent);
            return;
        default:
            return;
    }
}

PNode Parser::parse_region_command_cmd(PNode parent) {
    // REGION_COMMAND -> cmd none
    // REGION_COMMAND -> cmd all
    // REGION_COMMAND -> cmd lepsf
    // REGION_COMMAND -> cmd btagsf
    // REGION_COMMAND -> cmd xlumicorrsf
    auto next = lexer->peek(0);
    switch(next->get_token()) {

        case NONE: case ALL: case LEP_SF: case BTAGS_SF: case XSLUMICORR_SF:
            lexer->next();
            return make_terminal(parent, next);

        // REGION_COMMAND -> cmd hlt DESCRIPTION
        case HLT:
        {
            auto node = make_terminal(parent, next);
            lexer->next();
            node->add_child(parse_description(node));
            return node;
        }

        // REGION_COMMAND -> cmd applyhm (ID (E, E) eq integer)
        // REGION_COMMAND -> cmd applyhm (ID (E) eq integer)
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

        // REGION_COMMAND -> cmd IF_OR_CONDITION
        default:
            return parse_if_or_condition(parent);
    }
}

PNode Parser::parse_region_command(PNode parent) {
    // REGION_COMMAND -> cmd CONDITION
    // REGION_COMMAND -> cmd none
    // REGION_COMMAND -> cmd all
    // REGION_COMMAND -> cmd hlt DESCRIPTION
    // REGION_COMMAND -> cmd lepsf
    // REGION_COMMAND -> cmd btagsf
    // REGION_COMMAND -> cmd xlumicorrsf
    // REGION_COMMAND -> cmd applyhm (ID (E, E) eq integer)
    // REGION_COMMAND -> cmd applyhm (ID (E) eq integer)
    // REGION_COMMAND -> cmd ifstatement


    auto tok = lexer->next();
    switch(tok->get_token()) {
        
        case CMD:
            return parse_region_command_cmd(parent);

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

        // REGION_COMMAND -> bin CONDITION
        // REGION_COMMAND -> rejec CONDITION
        case REJEC: case BIN:
        {
            PNode node(make_terminal(parent, tok));
            node->add_child(parse_condition(node));
            return node;
        }

        // REGION_COMMANDS -> bins ID BIN_OR_BOX_VALUES
        case BINS:
        {
            PNode node(new Node(BINS_CMD, parent));
            node->add_child(parse_id(node));
            parse_bin_or_box_values(node);
            return node;
        }
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
        case PRINT:
        {
            PNode print(make_terminal(parent, tok));
            parse_variable_list(print);
        }
        case COUNTS:
        {
            PNode counts(make_terminal(parent, tok));
            counts->add_child(parse_id(counts));
            counts->add_child(parse_counts(counts));
        }
        case HISTO:
        {
            PNode histo(make_terminal(parent, tok));
            histo->add_child(parse_id(histo));
            lexer->expect_and_consume(COMMA);
            histo->add_child(parse_description(histo));
            lexer->expect_and_consume(COMMA);

            auto integer_tok_1 = lexer->next();
            if (integer_tok_1->get_token() != INTEGER) raise_parsing_exception("Only integers are allowed to specify binning quantity on histograms", tok);

            histo->add_child(make_terminal(histo, integer_tok_1));

            auto lower_value_tok_1 = lexer->next();
            if (!is_numerical(lower_value_tok_1->get_token())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_1);
            histo->add_child(make_terminal(histo, lower_value_tok_1));

            auto upper_value_tok_1 = lexer->next();
            if (!is_numerical(upper_value_tok_1->get_token())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
            histo->add_child(make_terminal(histo, upper_value_tok_1));

            // use to check if the list continues 
            bool is_2d = false;
            auto discriminant = lexer->peek(1);
            if (discriminant->get_token() == COMMA) {
                is_2d = true;
                auto lower_value_tok_2 = lexer->next();
                if (!is_numerical(lower_value_tok_2->get_token())) raise_parsing_exception("Only numerical types are allowed for the lower bound of a histogram", lower_value_tok_2);
                histo->add_child(make_terminal(histo, lower_value_tok_1));

                auto upper_value_tok_2 = lexer->next();
                if (!is_numerical(upper_value_tok_2->get_token())) raise_parsing_exception("Only numerical types are allowed for the upper bound of a histogram", upper_value_tok_1);
                histo->add_child(make_terminal(histo, upper_value_tok_2));
            }

            histo->add_child(parse_expression(histo));
            
            if (is_2d) histo->add_child(parse_expression(histo));

            return histo;

        }
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

void Parser::parse_variable_list(PNode parent) {
    // VARIABLE_LIST -> E VARIABLE_LIST
    // VARIABLE_LIST -> E, VARIABLE_LIST
    // VARIABLE_LIST -> epsilon

    auto tok = lexer->peek(0);
    switch(tok->get_token()) {

        // VARIABLE_LIST -> epsilon 
        // this is the follow set for VARIABLE_LIST, and none of them are in the first set of E
        case CLOSE_CURLY_BRACE: case COLON: case OBJ:  case CMD: case PRINT: case HISTO: case REJEC: case BINS: case SAVE: case WEIGHT: case COUNTS: case SORT: case ADLINFO: case COUNTSFORMAT: case DEF: case TABLE: case ALGO: case COMMA:
            return;            

        // VARIABLE_LIST -> E VARIABLE_LIST
        // VARIABLE_LIST -> E, VARIABLE_LIST
        default:
            parent->add_child(parse_expression(parent));
            auto next = lexer->peek(0);
            if (next->get_token() == COMMA) {
                lexer->expect_and_consume(COMMA);
            }
            parse_variable_list(parent);
        
    
    }
}

PNode Parser::parse_if_or_condition(PNode parent) {

    // IF_OR_CONDITION -> CONDITION
    // IF_OR_CONDITION -> CONDITION ? ACTION : ACTION

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

PNode Parser::parse_action(PNode parent){
    auto tok = lexer->peek(0);
    switch (tok->get_token()) {
        case PRINT:
        {
            lexer->next();
            PNode print(make_terminal(parent, tok));
            parse_variable_list(print);
            return print;
        }
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
                    //TODO: finish this rule and subsequent
                }
                
                lexer->expect_and_consume(CLOSE_SQUARE_BRACE);
            } else {
                
            }
            lexer->expect_and_consume(CLOSE_PAREN);
            return apply_ptf;
        }

        // ACTION -> APPLY_HM (ID (E, E) = integer)
        // ACTION -> APPLY_HM (ID (E) = integer)
        case APPLY_HM:
        {
            
        }


    }
}

PNode Parser::parse_condition(PNode parent) {

}
void Parser::parse_bin_or_box_values(PNode parent) {
    // BIN_OR_BOX_VALUES -> number BIN_OR_BOX_VALUES
    // BIN_OR_BOX_VALUES -> number

    auto tok = lexer->next();
    if (!is_numerical(tok->get_token())) raise_parsing_exception("Needs a numerical value for box argument", tok);

    parent->add_child(make_terminal(parent, tok));

    auto next = lexer->peek(0);
    if (is_numerical(next->get_token())) parse_bin_or_box_values(parent);
}
PNode Parser::parse_counts(PNode parent) {

    
}

void Parser::parse_particle_list(PNode parent) {
    parent->add_child(parse_particle(parent));
    auto tok = lexer->peek(0);
    // PARTICLE_LIST -> PARTICLE + PARTICLE_LIST
    switch (tok->get_token()) {
        case PLUS:
            lexer->expect_and_consume(PLUS);
            parse_particle_list(parent);
        // PARTICLE_LIST -> PARTICLE PARTICLE_LIST
        case GEN: case ELECTRON: case MUON: case TAU: case TRACK: case LEPTON: case PHOTON: case JET: case BJET: case FJET: case QGJET: case NUMET: case METLV: case STRING: case VARNAME: case MINUS:
            parse_particle_list(parent);
        default:
        // PARTICLE_LIST -> PARTICLE
            return;
    }
}

PNode Parser::parse_particle(PNode parent) {

}

PNode Parser::parse_index(PNode parent) {

}


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

PNode Parser::parse_hamhum(PNode parent) {
    PNode hamhum(new Node(HAMHUM, parent));
    lexer->expect_and_consume(ALIAS);
    
    hamhum->add_child(parse_id(hamhum));
    return hamhum;
}

void Parser::parse_criteria(PNode parent) {
    // CRITERIA -> CRITERION CRITERIA
    // CRITERIA -> epsilon

    auto tok = lexer->peek(0);
    switch(tok->get_token()) {
        case CMD: case PRINT: case HISTO: case REJEC:
            parent->add_child(parse_criterion(parent));
            parse_criteria(parent);
            return;
        default:
            return;
    }
}

PNode Parser::parse_criterion(PNode parent) {
    auto tok = lexer->next();
    // CRITERION -> cmd ACTION
    PNode node(make_terminal(parent, tok));
    switch (tok->get_token()) {

        case CMD: case HISTO:
            node->add_child(parse_action(node));
            return node;
        case PRINT:
            parse_variable_list(node);
            return node;
        case REJEC:
            node->add_child(parse_condition(node));
            return node;
        default:
            raise_parsing_exception("Invalid token for a criterion", tok);
    }

}

PNode Parser::parse_expression(PNode parent) {
//TODO: precedence climbing
}

void print_children_and_yourself(PNode node, int *top_number) {

    int reserved_number_for_me = (*top_number)++;

    if (node->has_token()) {
        std::cout << reserved_number_for_me << " [label=\"" << node->get_token()->get_lexeme() << "\"]" << std::endl;
    } else {
        std::cout << reserved_number_for_me << " [label=\"" << node->get_ast_type() << "\"]" <<std::endl;
    }

    auto children_vector = node->get_children();
    for (auto it = children_vector.begin(); it != children_vector.end(); ++it) {
        std::cout << reserved_number_for_me << " -> " << *top_number << std::endl;
        print_children_and_yourself(*it, top_number);
    }
}

void Parser::print_parse_dot() {
    PNode input_node = tree.get_root();

    int top = 1;
    print_children_and_yourself(input_node, &top);
}