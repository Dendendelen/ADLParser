#include "aliconverter.h"
#include "lexer.h"
#include "node.h"
#include "tokens.h"
#include "exceptions.h"
#include <sstream>
#include <iostream>


AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst): instruction(inst)  {

}

void AnalysisCommand::add_argument(std::string arg) {
    arguments.push_back(arg);
}

void AnalysisCommand::convert_instruction() {
    
}

std::string instruction_to_text(AnalysisLevelInstruction inst) {

    switch (inst) {
        case BEGIN_REGION_BLOCK:
            return "BEGIN_REGION_BLOCK";
        case END_REGION_BLOCK:
            return "END_REGION_BLOCK";
        case CREATE_REGION:
            return "CREATE_REGION";
        case MERGE_REGIONS:
            return "MERGE_REGIONS";
        case CUT_REGION:
            return "CUT_REGION";
        case RUN_REGION:
            return "RUN_REGION";
        case ADD_ALIAS:
            return "ADD_ALIAS";
        case ADD_OBJECT:
            return "ADD_OBJECT";
        case CREATE_MASK:
            return "CREATE_MASK";
        case LIMIT_MASK:
            return "LIMIT_MASK";
        case APPLY_MASK:
            return "APPLY_MASK";
        case BEGIN_EXPRESSION:
            return "BEGIN_EXPRESSION";
        case END_EXPRESSION:
            return "END_EXPRESSION";
        case BEGIN_IF:
            return "BEGIN_IF";
        case END_IF:
            return "END_IF";
        case EXPR_RAISE:
            return "EXPR_RAISE";
        case EXPR_MULTIPLY:
            return "EXPR_MULTIPLY";
        case EXPR_DIVIDE:
            return "EXPR_DIVIDE";
        case EXPR_ADD:
            return "EXPR_ADD";
        case EXPR_SUBTRACT:
            return "EXPR_SUBTRACT";
        case EXPR_LT:
            return "EXPR_LT";
        case EXPR_LE:
            return "EXPR_LE";
        case EXPR_GT:
            return "EXPR_GT";
        case EXPR_GE:
            return "EXPR_GE";
        case EXPR_EQ:
            return "EXPR_EQ";
        case EXPR_NE:
            return "EXPR_NE";
        case EXPR_AMPERSAND:
            return "EXPR_AMPERSAND";
        case EXPR_PIPE:
            return "EXPR_PIPE";
        case EXPR_AND:
            return "EXPR_AND";
        case EXPR_OR:
            return "EXPR_OR";
        case EXPR_WITHIN:
            return "EXPR_WITHIN";
        case EXPR_OUTSIDE:
            return "EXPR_OUTSIDE";
        case EXPR_NEGATE:
            return "EXPR_NEGATE";
        case EXPR_LOGICAL_NOT:
            return "EXPR_LOGICAL_NOT";

        case FUNC_BTAG:
            return "FUNC_BTAG"; 
        case FUNC_PT:
            return "FUNC_PT"; 
        case FUNC_ETA:
            return "FUNC_ETA"; 
        case FUNC_PHI:
            return "FUNC_PHI"; 
        case FUNC_M:
            return "FUNC_M"; 
        case FUNC_E:
            return "FUNC_E"; 
        case MAKE_EMPTY_PARTICLE:
            return "MAKE_EMPTY_PARTICLE"; 
        case CREATE_PARTICLE_VARIABLE:
            return "CREATE_PARTICLE_VARIABLE"; 
        case ADD_PART_ELECTRON:
            return "ADD_PART_ELECTRON"; 
        case ADD_PART_MUON:
            return "ADD_PART_MUON"; 
        case ADD_PART_TAU:
            return "ADD_PART_TAU"; 
        case ADD_PART_TRACK:
            return "ADD_PART_TRACK"; 
        case ADD_PART_LEPTON:
            return "ADD_PART_LEPTON"; 
        case ADD_PART_PHOTON:
            return "ADD_PART_PHOTON"; 
        case ADD_PART_BJET:
            return "ADD_PART_BJET"; 
        case ADD_PART_QGJET:
            return "ADD_PART_QGJET"; 
        case ADD_PART_NUMET:
            return "ADD_PART_NUMET"; 
        case ADD_PART_METLV:
            return "ADD_PART_METLV"; 
        case ADD_PART_GEN:
            return "ADD_PART_GEN"; 
        case ADD_PART_JET:
            return "ADD_PART_JET"; 
        case ADD_PART_FJET:
            return "ADD_PART_FJET"; 
        case ADD_PART_NAMED:
            return "ADD_PART_NAMED"; 
        case SUB_PART_ELECTRON:
            return "SUB_PART_ELECTRON"; 
        case SUB_PART_MUON:
            return "SUB_PART_MUON"; 
        case SUB_PART_TAU:
            return "SUB_PART_TAU"; 
        case SUB_PART_TRACK:
            return "SUB_PART_TRACK"; 
        case SUB_PART_LEPTON:
            return "SUB_PART_LEPTON"; 
        case SUB_PART_PHOTON:
            return "SUB_PART_PHOTON"; 
        case SUB_PART_BJET:
            return "SUB_PART_BJET"; 
        case SUB_PART_QGJET:
            return "SUB_PART_QGJET"; 
        case SUB_PART_NUMET:
            return "SUB_PART_NUMET"; 
        case SUB_PART_METLV:
            return "SUB_PART_METLV"; 
        case SUB_PART_GEN:
            return "SUB_PART_GEN"; 
        case SUB_PART_JET:
            return "SUB_PART_JET"; 
        case SUB_PART_FJET:
            return "SUB_PART_FJET"; 
        case SUB_PART_NAMED:
            return "SUB_PART_NAMED";
        case FUNC_HSTEP:
            return "FUNC_HSTEP";
        case FUNC_DELTA:
            return "FUNC_DELTA";
        case FUNC_ANYOF:
            return "FUNC_ANYOF";
        case FUNC_ALLOF:
            return "FUNC_ALLOF";
        case FUNC_SQRT:
            return "FUNC_SQRT";
        case FUNC_ABS:
            return "FUNC_ABS";
        case FUNC_COS:
            return "FUNC_COS";
        case FUNC_SIN:
            return "FUNC_SIN";
        case FUNC_TAN:
            return "FUNC_TAN";
        case FUNC_SINH:
            return "FUNC_SINH";
        case FUNC_COSH:
            return "FUNC_COSH";
        case FUNC_TANH:
            return "FUNC_TANH";
        case FUNC_EXP:
            return "FUNC_EXP";
        case FUNC_LOG:
            return "FUNC_LOG";
        case FUNC_AVE:
            return "FUNC_AVE";
        case FUNC_SUM:
            return "FUNC_SUM";
        case FUNC_NAMED:
            return "FUNC_NAMED";
        case MAKE_EMPTY_UNION:
            return "MAKE_EMPTY_UNION";
        case ADD_NAMED_TO_UNION:
            return "ADD_NAMED_TO_UNION";
        case ADD_ELECTRON_TO_UNION:
            return "ADD_ELECTRON_TO_UNION";
        case ADD_MUON_TO_UNION:
            return "ADD_MUON_TO_UNION";
        case ADD_TAU_TO_UNION:
            return "ADD_TAU_TO_UNION";
    }
}

void AnalysisCommand::print_instruction() {
    
    if (instruction == BEGIN_EXPRESSION || instruction == CREATE_REGION || instruction == CREATE_MASK) std::cout << std::endl;

    std::cout << instruction_to_text(instruction);

    for (auto it = arguments.begin(); it != arguments.end(); ++it) {
        std::cout << " (";
        std::cout << *it;
        std::cout << ")";
    }
    std::cout << std::endl;

    if (instruction == END_EXPRESSION) std::cout << std::endl;

}

void visit_if(PNode node) {

}

std::string ALIConverter::reserve_scoped_value_name() {
    std::stringstream new_var_name;
    new_var_name << "$V" << highest_var_val++ << "_" << current_scope_name;
    return new_var_name.str();
}

std::string ALIConverter::reserve_scoped_limit_name() {
    std::stringstream new_var_name;
    new_var_name << "$L" << highest_var_val++ << "_" << current_scope_name;
    return new_var_name.str();
}

std::string ALIConverter::reserve_scoped_region_name() {
    std::stringstream new_var_name;
    new_var_name << "$R" << highest_var_val++ << "_" << current_scope_name;
    return new_var_name.str();
}

std::string ALIConverter::if_operator(PNode node) {
    
}

std::string ALIConverter::interval_operator(PNode node) {

    AnalysisLevelInstruction inst;

    if (node->get_token()->get_token() == OUTSIDE) {
        inst = EXPR_OUTSIDE;
    } else {
        inst = EXPR_WITHIN;
    }

    std::string lhs = visit_expression(node->get_children()[0]);

    PNode interval = node->get_children()[1];
    if (interval->get_ast_type() != INTERVAL) raise_analysis_conversion_exception("An interval must follow this token.", node->get_token());
    std::string interval_lhs = visit_expression(interval->get_children()[0]);
    std::string interval_rhs = visit_expression(interval->get_children()[1]);

    AnalysisCommand interval_exp(inst);

    std::string dest = reserve_scoped_value_name();

    interval_exp.add_argument(dest);
    interval_exp.add_argument(lhs);
    interval_exp.add_argument(interval_lhs);
    interval_exp.add_argument(interval_rhs);

    command_list.push_back(interval_exp);

    return dest;

}

std::string ALIConverter::handle_particle(PNode node, std::string last_part) {
    AnalysisLevelInstruction inst;

    PToken tok = node->get_token();

    if (tok->get_token() == MINUS) {
        node = node->get_children()[0];
        tok = node->get_token();
        switch(tok->get_token()) {
            case ELECTRON: 
                inst = SUB_PART_ELECTRON; break;
            case MUON: 
                inst = SUB_PART_MUON; break;
            case TAU: 
                inst = SUB_PART_TAU; break;
            case TRACK: 
                inst = SUB_PART_TRACK; break;
            case LEPTON: 
                inst = SUB_PART_LEPTON; break;
            case PHOTON:  
                inst = SUB_PART_PHOTON; break;
            case BJET: 
                inst = SUB_PART_BJET; break;
            case QGJET: 
                inst = SUB_PART_QGJET; break;
            case NUMET: 
                inst = SUB_PART_NUMET; break;
            case METLV:
                inst = SUB_PART_METLV; break;
            case GEN: 
                inst = SUB_PART_GEN; break;
            case JET: 
                inst = SUB_PART_JET; break;
            case FJET:
                inst = SUB_PART_FJET; break;
            default: 
                inst = SUB_PART_NAMED; break;
        }
    } else {
        switch(tok->get_token()) {
            case ELECTRON: 
                inst = ADD_PART_ELECTRON; break;
            case MUON: 
                inst = ADD_PART_MUON; break;
            case TAU: 
                inst = ADD_PART_TAU; break;
            case TRACK: 
                inst = ADD_PART_TRACK; break;
            case LEPTON: 
                inst = ADD_PART_LEPTON; break;
            case PHOTON:  
                inst = ADD_PART_PHOTON; break;
            case BJET: 
                inst = ADD_PART_BJET; break;
            case QGJET: 
                inst = ADD_PART_QGJET; break;
            case NUMET: 
                inst = ADD_PART_NUMET; break;
            case METLV:
                inst = ADD_PART_METLV; break;
            case GEN: 
                inst = ADD_PART_GEN; break;
            case JET: 
                inst = ADD_PART_JET; break;
            case FJET:
                inst = ADD_PART_FJET; break;
            default: 
                inst = ADD_PART_NAMED; break;
        }
    }


    AnalysisCommand next_part(inst);

    std::string part_name = reserve_scoped_value_name();
    next_part.add_argument(part_name);

    if (inst == ADD_PART_NAMED || inst == SUB_PART_NAMED) {
        next_part.add_argument(tok->get_lexeme());
    }

    next_part.add_argument(last_part);

    if (node->get_children().size() > 0 && node->get_children()[0]->get_ast_type() == INDEX) {
        next_part.add_argument(node->get_children()[0]->get_children()[0]->get_token()->get_lexeme());
        if (node->get_children()[0]->get_children().size() > 1) {
            next_part.add_argument(node->get_children()[0]->get_children()[1]->get_token()->get_lexeme());
        }
    }

    command_list.push_back(next_part);

    return part_name;
}

std::string ALIConverter::handle_particle_list(PNode node) {

    AnalysisCommand start(MAKE_EMPTY_PARTICLE);

    std::string last_part = reserve_scoped_value_name();
    start.add_argument(last_part);
    command_list.push_back(start);

    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {

        last_part = handle_particle(*it, last_part);

    }

    return last_part;
}



std::string ALIConverter::particle_list_function(PNode node) {

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token()) {
        case LETTER_E: 

        case LETTER_P: 
        
        case LETTER_M: 
        
        case LETTER_Q: 

        case FLAVOR: 
        
        case CONSTITUENTS: 
        
        case PDG_ID: 
        
        case IDX: 
        
        case IS_TAUTAG: 
        
        case IS_CTAG: 
        
        case IS_BTAG: 
            inst = FUNC_BTAG; break;
        case DXY: 
        
        case EDXY: 
        
        case EDZ: 
        
        case DZ: 
        
        case VERTR: 
        
        case VERZ: 
        
        case VERY: 
        
        case VERX: 
        
        case VERT: 

        case GENPART_IDX: 
        
        case PHI: 
            inst = FUNC_PHI; break;
        case RAPIDITY: 

        case ETA: 
            inst = FUNC_ETA; break;
        case ABS_ETA: 

        case THETA: 

        case PTCONE: 
        
        case ETCONE: 
        
        case ABS_ISO: 
        
        case MINI_ISO: 
        
        case IS_TIGHT: 
        
        case IS_MEDIUM: 
        
        case IS_LOOSE: 

        case PT: 
            inst = FUNC_PT; break;
        case PZ: 
        
        case NBF: 
        
        case DR: 
        
        case DPHI: 
        
        case DETA: 
        
        case NUMOF:
        
        case FMT2: 
         
        case FMTAUTAU:

        default:
            break;

    }

    AnalysisCommand func(inst);

    std::string dest = reserve_scoped_value_name();
    std::string particle_name = handle_particle_list(node);
    
    func.add_argument(dest);
    func.add_argument(particle_name);

    command_list.push_back(func);

    return dest;
}


std::string ALIConverter::unary_operator(PNode node) {

    std::string source = visit_expression(node->get_children()[0]);

    AnalysisLevelInstruction inst;

    if (node->get_ast_type() == NEGATE) {
        inst = EXPR_NEGATE;
    } else if (node->get_token()->get_token() == NOT) {
        inst = EXPR_LOGICAL_NOT;
    }

    AnalysisCommand unary_exp(inst);

    std::string dest = reserve_scoped_value_name();

    unary_exp.add_argument(dest);
    unary_exp.add_argument(source);

    command_list.push_back(unary_exp);
    return dest;
    
}

std::string ALIConverter::binary_operator(PNode node) {
    std::string lhs = visit_expression(node->get_children()[0]);

    std::string rhs = visit_expression(node->get_children()[1]);

    AnalysisLevelInstruction inst;

    switch(node->get_token()->get_token()) {
        case RAISED_TO_POWER:
            inst = EXPR_RAISE; break;
        case MULTIPLY:
            inst = EXPR_MULTIPLY; break;
        case DIVIDE:
            inst = EXPR_DIVIDE; break;
        case ADD:
            inst = EXPR_ADD; break;
        case MINUS:
            inst = EXPR_SUBTRACT; break;
        case LT:
            inst = EXPR_LT; break;
        case LE:
            inst = EXPR_LE; break;
        case GT:
            inst = EXPR_GT; break;
        case GE:
            inst = EXPR_GE; break;
        case EQ:
            inst = EXPR_EQ; break;
        case NE:
            inst = EXPR_NE; break;
        case AMPERSAND:
            inst = EXPR_AMPERSAND; break;
        case PIPE:
            inst = EXPR_PIPE; break;
        case AND:
            inst = EXPR_AND; break;
        case OR:
            inst = EXPR_OR; break;
        case WITHIN:
            inst = EXPR_WITHIN; break;
        case OUTSIDE:
            inst = EXPR_OUTSIDE; break;
        
    }

    AnalysisCommand binary_exp(inst);

    std::string dest = reserve_scoped_value_name();

    binary_exp.add_argument(dest);
    binary_exp.add_argument(lhs);
    binary_exp.add_argument(rhs);

    command_list.push_back(binary_exp);
    return dest;
}

std::string ALIConverter::literal_value(PNode node) {
    AnalysisCommand assign(ADD_ALIAS);

    std::string dest = reserve_scoped_value_name();
    assign.add_argument(dest);
    assign.add_argument(node->get_token()->get_lexeme());

    command_list.push_back(assign);

    return dest;
}

std::string ALIConverter::expression_function(PNode node) {
    std::string source = visit_expression(node->get_children()[0]);
    std::string dest = reserve_scoped_limit_name();

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token()) {
        case HSTEP:
            inst = FUNC_HSTEP; break;
        case DELTA: 
            inst = FUNC_DELTA; break;
        case ANYOF: 
            inst = FUNC_ANYOF; break;
        case ALLOF: 
            inst = FUNC_ALLOF; break;
        case SQRT: 
            inst = FUNC_SQRT; break;
        case ABS: 
            inst = FUNC_ABS; break;
        case COS:  
            inst = FUNC_COS; break;
        case SIN: 
            inst = FUNC_SIN; break;
        case TAN: 
            inst = FUNC_TAN; break;
        case SINH: 
            inst = FUNC_SINH; break;
        case COSH: 
            inst = FUNC_COSH; break;
        case TANH: 
            inst = FUNC_TANH; break;
        case EXP: 
            inst = FUNC_EXP; break;
        case LOG: 
            inst = FUNC_LOG; break;
        case AVE: 
            inst = FUNC_AVE; break;
        case SUM: 
            inst = FUNC_SUM; break;
        default:
            raise_analysis_conversion_exception("Invalid function acting on an expression", node->get_token());
            break;
    }

    AnalysisCommand func(inst);
    func.add_argument(dest);
    func.add_argument(source);

    command_list.push_back(func);
    return dest;
}

std::string ALIConverter::visit_expression(PNode node) {

    if (node->get_ast_type() == USER_FUNCTION) {

    } else if (node->get_ast_type() == NEGATE) {
        return unary_operator(node);
    }

    switch(node->get_token()->get_token()) {
        case BWL: case BWR: case RAISED_TO_POWER: case MULTIPLY: case DIVIDE: case ADD: case MINUS: case IRG: case ERG: case MAXIMIZE: case MINIMIZE: case LT: case GT: case LE: case GE: case EQ: case NE: case AMPERSAND: case PIPE: case AND: case OR:
            return binary_operator(node);
        case WITHIN: case OUTSIDE:
            return interval_operator(node);
        case NOT:
            return unary_operator(node);
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: 
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case EDXY: case EDZ: case DZ: case VERTR: case VERZ: case VERY: case VERX: case VERT: 
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case ABS_ETA: case THETA: 
        case PTCONE: case ETCONE: case ABS_ISO: case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case PT: case PZ: case NBF: case DR: case DPHI: case DETA: case NUMOF: case FMT2: case FMTAUTAU:
            return particle_list_function(node);
        case HSTEP: case DELTA: case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: 
            return expression_function(node);
        default:
            return literal_value(node);
    }
}

void ALIConverter::visit_condition(PNode node) {
    AnalysisCommand condition(BEGIN_EXPRESSION);

    std::stringstream cond_name;
    cond_name << "$COND_" << current_scope_name;
    condition.add_argument(cond_name.str());

    command_list.push_back(condition);

    std::string final = visit_expression(node->get_children()[0]);

    AnalysisCommand end_condition(END_EXPRESSION);
    end_condition.add_argument(cond_name.str());
    end_condition.add_argument(final);

    command_list.push_back(end_condition);
    last_condition_name = cond_name.str();
}

void ALIConverter::visit_if(PNode node) {
    if (node->get_children().size() == 1) {

        visit_children(node);

    } else {

        std::string old_scope = current_scope_name;

        std::stringstream if_scope_1; 
        if_scope_1 << "$IF1_";

        AnalysisCommand if_statement(BEGIN_IF);



        AnalysisCommand end_if(END_IF);

        current_scope_name = old_scope;
    }
}

void ALIConverter::visit_object_select(PNode node) {

    std::string prev_name = current_scope_name;
    current_scope_name = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand limit_mask(LIMIT_MASK);
    limit_mask.add_argument(current_scope_name);
    limit_mask.add_argument(prev_name);
    limit_mask.add_argument(last_condition_name);

    command_list.push_back(limit_mask);
}

void ALIConverter::visit_object_reject(PNode node) {
    AnalysisCommand limit_mask(LIMIT_MASK);
    
}

std::string ALIConverter::union_list(PNode node, std::string prev) {

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token()) {
        case ELECTRON:
            inst = ADD_ELECTRON_TO_UNION; break;
        case MUON:
            inst = ADD_MUON_TO_UNION; break;
        case TAU:
            inst = ADD_TAU_TO_UNION; break;
        default:
            inst = ADD_NAMED_TO_UNION; break;
    }

    AnalysisCommand unite(inst);
    std::string dest = reserve_scoped_limit_name();

    unite.add_argument(dest);
    unite.add_argument(prev);

    if (inst == ADD_NAMED_TO_UNION) {
        unite.add_argument(node->get_token()->get_lexeme());
    }

    command_list.push_back(unite);

    return dest;
}

void ALIConverter::visit_union_type(PNode node) {

    std::string prev_name = current_scope_name;

    PNode union_node = node->get_children()[1];
    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream union_name;
    union_name << "$UNION_" << name;

    current_scope_name = union_name.str();

    std::string source = reserve_scoped_value_name();
    AnalysisCommand make_union(MAKE_EMPTY_UNION);
    make_union.add_argument(source);

    command_list.push_back(make_union);


    for (auto it = union_node->get_children().begin(); it != union_node->get_children().end(); ++it) {
        source = union_list(*it, source);
    }

    AnalysisCommand finalize_union(ADD_ALIAS);
    finalize_union.add_argument(current_scope_name);
    finalize_union.add_argument(source);

    command_list.push_back(finalize_union);

    current_scope_name = prev_name;

}

void ALIConverter::visit_object(PNode node) {


    
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    PNode source = node->get_children()[1];

    if (source->get_token()->get_token() == UNION) {

        visit_union_type(node);
        return;
    }

    std::string name_lexeme = name->get_token()->get_lexeme();
    std::string source_lexeme = source->get_token()->get_lexeme();

    // the CREATE_MASK command takes the new mask's name, the target object's name, and the source object's name

    AnalysisCommand create_mask(CREATE_MASK);
    
    std::stringstream mask_name;
    mask_name << "$MASK_" << name_lexeme;

    // move us into the "scope" of this object
    current_scope_name = mask_name.str();

    create_mask.add_argument(current_scope_name);
    create_mask.add_argument(source->get_token()->get_lexeme());
    command_list.push_back(create_mask);


    visit_children_after_index(node, 1);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand apply_mask(APPLY_MASK);

    apply_mask.add_argument(name_lexeme);
    apply_mask.add_argument(current_scope_name);
    apply_mask.add_argument(source_lexeme);

    command_list.push_back(apply_mask);
    current_scope_name = prev_name;
}

void ALIConverter::visit_region_select (PNode node) {
    std::string prev_name = current_scope_name;
    current_scope_name = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand cut_region(CUT_REGION);
    cut_region.add_argument(current_scope_name);
    cut_region.add_argument(prev_name);
    cut_region.add_argument(last_condition_name);

    command_list.push_back(cut_region);
}

void ALIConverter::visit_use(PNode node) {
    
    std::string prev_name = current_scope_name;
    current_scope_name = reserve_scoped_value_name();

    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::string dest = reserve_scoped_value_name();
    
    AnalysisCommand merge_regions(MERGE_REGIONS);

    merge_regions.add_argument(dest);
    merge_regions.add_argument(name);
    merge_regions.add_argument(prev_name);

    command_list.push_back(merge_regions);
}


void ALIConverter::visit_region(PNode node) {
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];

    std::string name_lexeme = name->get_token()->get_lexeme();

    AnalysisCommand create_region(CREATE_REGION);
    
    std::stringstream reg_name;
    reg_name << "$REG_" << name_lexeme;

    // move us into the "scope" of this region
    current_scope_name = reg_name.str();

    create_region.add_argument(current_scope_name);
    command_list.push_back(create_region);

    visit_children_after_index(node, 0);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand add_reg_name(ADD_ALIAS);

    add_reg_name.add_argument(name_lexeme);
    add_reg_name.add_argument(current_scope_name);

    command_list.push_back(add_reg_name);
    current_scope_name = prev_name;

}
void ALIConverter::visit_criteria(PNode node) {
    
}

void ALIConverter::vistitation(PNode root) {
    visit(root);
}

void ALIConverter::print_commands() {
    for (auto it = command_list.begin(); it != command_list.end(); ++it) {
        it->print_instruction();
    }
}

ALIConverter::ALIConverter(): highest_var_val(0) {}