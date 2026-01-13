#include "ali_converter.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <sstream>
#include <iostream>
#include <string>


AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst): instruction(inst)  {

}

void AnalysisCommand::add_argument(std::string arg) {
    arguments.push_back(arg);
}

AnalysisLevelInstruction AnalysisCommand::get_instruction() {
    return instruction;
}
std::string AnalysisCommand::get_argument(int pos) {
    assert(pos < arguments.size());
    return arguments[pos];
}

int AnalysisCommand::get_num_arguments() {
    return arguments.size();
}

std::string AnalysisCommand::instruction_to_text(AnalysisLevelInstruction inst) {

    switch (inst) {
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
        case ADD_EXTERNAL:
            return "ADD_EXTERNAL";
        case ADD_CORRECTIONLIB:
            return "ADD_CORRECTIONLIB";
        case ADD_OBJECT:
            return "ADD_OBJECT";
        case CREATE_MASK:
            return "CREATE_MASK";
        case LIMIT_MASK:
            return "LIMIT_MASK";
        case APPLY_MASK:
            return "APPLY_MASK";

        case CREATE_HIST_LIST:
            return "CREATE_HIST_LIST";
        case ADD_HIST_TO_LIST:
            return "ADD_HIST_TO_LIST";
        case USE_HIST:
            return "USE_HIST";
        case USE_HIST_LIST:
            return "USE_HIST_LIST";
        case HIST_1D:
            return "HIST_1D";
        case HIST_2D:
            return "HIST_2D";
        case CREATE_BIN:
            return "CREATE_BIN";

        case DO_CUTFLOW_ON_REGION:
            return "DO_CUTFLOW_ON_REGION";

        case CREATE_TABLE:
            return "CREATE_TABLE";
        case CREATE_TABLE_LOWER_BOUNDS:
            return "CREATE_TABLE_LOWER_BOUNDS";
        case  CREATE_TABLE_UPPER_BOUNDS:
            return "CREATE_TABLE_UPPER_BOUNDS";
        case CREATE_TABLE_VALUE:
            return "CREATE_TABLE_VALUE";
        case APPEND_TO_TABLE:
            return "APPEND_TO_TABLE";
        case FINISH_TABLE:
            return "FINISH_TABLE";

        case SORT_ASCEND:
            return "SORT_ASCEND";
        case SORT_DESCEND:
            return "SORT_DESCEND";
        case WEIGHT_APPLY:
            return "WEIGHT_APPLY";

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
        case FUNC_MASS:
            return "FUNC_M"; 
        case FUNC_MSOFTDROP:
            return "FUNC_MSOFTDROP";
        case FUNC_ENERGY:
            return "FUNC_E"; 
        case MAKE_EMPTY_PARTICLE:
            return "MAKE_EMPTY_PARTICLE"; 
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
        case FUNC_MIN:
            return "FUNC_MIN";
        case FUNC_ANYOCCURRENCES:
            return "FUNC_ANYOCCURANCES";
        case FUNC_FIRST:
            return "FUNC_FIRST";
        case FUNC_SECOND:
            return "FUNC_SECOND";
        case FUNC_MAX:
            return "FUNC_MAX";
        case FUNC_MAX_LIST:
            return "FUNC_MAX";
        case FUNC_MIN_LIST:
            return "FUNC_MIN_LIST";
        case FUNC_SORT_ASCEND:
            return "FUNC_SORT_ASCEND";
        case FUNC_SORT_DESCEND:
            return "FUNC_SORT_DESCEND";
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
        case ADD_TRACK_TO_UNION:
            return "ADD_TRACK_TO_UNION"; 
        case ADD_LEPTON_TO_UNION:
            return "ADD_LEPTON_TO_UNION"; 
        case ADD_PHOTON_TO_UNION:
            return "ADD_PHOTON_TO_UNION"; 
        case ADD_BJET_TO_UNION:
            return "ADD_BJET_TO_UNION"; 
        case ADD_QGJET_TO_UNION:
            return "ADD_QGJET_TO_UNION"; 
        case ADD_NUMET_TO_UNION:
            return "ADD_NUMET_TO_UNION"; 
        case ADD_METLV_TO_UNION:
            return "ADD_METLV_TO_UNION"; 
        case ADD_GEN_TO_UNION:
            return "ADD_GEN_TO_UNION"; 
        case ADD_JET_TO_UNION:
            return "ADD_JET_TO_UNION"; 
        case ADD_FJET_TO_UNION:
            return "ADD_FJET_TO_UNION"; 

        case MAKE_EMPTY_COMB:
            return "MAKE_EMPTY_COMB";
        case ADD_NAMED_TO_COMB:
            return "ADD_NAMED_TO_COMB";
        case ADD_ELECTRON_TO_COMB:
            return "ADD_ELECTRON_TO_COMB";
        case ADD_MUON_TO_COMB:
            return "ADD_MUON_TO_COMB";
        case ADD_TAU_TO_COMB:
            return "ADD_TAU_TO_COMB";
        case ADD_TRACK_TO_COMB:
            return "ADD_TRACK_TO_COMB"; 
        case ADD_LEPTON_TO_COMB:
            return "ADD_LEPTON_TO_COMB"; 
        case ADD_PHOTON_TO_COMB:
            return "ADD_PHOTON_TO_COMB"; 
        case ADD_BJET_TO_COMB:
            return "ADD_BJET_TO_COMB"; 
        case ADD_QGJET_TO_COMB:
            return "ADD_QGJET_TO_COMB"; 
        case ADD_NUMET_TO_COMB:
            return "ADD_NUMET_TO_COMB"; 
        case ADD_METLV_TO_COMB:
            return "ADD_METLV_TO_COMB"; 
        case ADD_GEN_TO_COMB:
            return "ADD_GEN_TO_COMB"; 
        case ADD_JET_TO_COMB:
            return "ADD_JET_TO_COMB"; 
        case ADD_FJET_TO_COMB:
            return "ADD_FJET_TO_COMB"; 

        case FUNC_FLAVOR:
            return "FUNC_FLAVOR";
        case FUNC_CONSTITUENTS:
            return "FUNC_CONSTITUENTS";
        case FUNC_PDG_ID:
            return "FUNC_PDG_ID";
        case FUNC_JET_ID:
            return "FUNC_JET_ID";
        case FUNC_IDX:
            return "FUNC_IDX";
        case FUNC_TAUTAG:
            return "FUNC_TAUTAG";
        case FUNC_CTAG:
            return "FUNC_CTAG";
        case FUNC_DXY:
            return "FUNC_DXY";
        case FUNC_EDXY:
            return "FUNC_EDXY";
        case FUNC_EDZ:
            return "FUNC_EDZ";
        case FUNC_DZ:
            return "FUNC_DZ";
        case FUNC_IS_TIGHT:
            return "FUNC_IS_TIGHT";
        case FUNC_IS_MEDIUM:
            return "FUNC_IS_MEDIUM";
        case FUNC_IS_LOOSE:
            return "FUNC_IS_LOOSE";
        case FUNC_ABS_ETA:
            return "FUNC_ABS_ETA";
        case FUNC_THETA:
            return "FUNC_THETA";
        case FUNC_PT_CONE:
            return "FUNC_PT_CONE";
        case FUNC_ET_CONE:
            return "FUNC_ET_CONE";
        case FUNC_ABS_ISO:
            return "FUNC_ABS_ISO";
        case FUNC_MINI_ISO:
            return "FUNC_MINI_ISO";
        case FUNC_PZ:
            return "FUNC_PZ";
        case FUNC_NBF:
            return "FUNC_NBF";
        case FUNC_DR:
            return "FUNC_DR";
        case FUNC_DPHI:
            return "FUNC_DPHI";
        case FUNC_DETA:
            return "FUNC_DETA";
        case FUNC_SIZE:
            return "FUNC_SIZE";
        case FUNC_VER_TR:
            return "FUNC_VER_TR";
        case FUNC_VER_Z:
            return "FUNC_VER_Z";
        case FUNC_VER_Y:
            return "FUNC_VER_Y";
        case FUNC_VER_X:
            return "FUNC_VER_X";
        case FUNC_VER_T:
            return "FUNC_VER_T";
        case FUNC_GEN_PART_IDX:
            return "FUNC_GEN_PART_IDX";
        case FUNC_CHARGE:
            return "FUNC_CHARGE";
        case FUNC_RAPIDITY:
            return "FUNC_RAPIDITY";
        case FUNC_FMT2:
            return "FUNC_FMT2";
        case FUNC_TAUTAU:
            return "FUNC_TAUTAU";
        case FUNC_APLANARITY:
            return "FUNC_APLANARITY";
        case FUNC_SPHERICITY:
            return "FUNC_SPHERICITY";
        case FUNC_HT:
            return "FUNC_HT";
        }
}

void AnalysisCommand::print_instruction() {
    
    if (instruction == MAKE_EMPTY_PARTICLE || instruction == MAKE_EMPTY_UNION || instruction == MAKE_EMPTY_COMB || instruction == CREATE_REGION || instruction == CREATE_MASK) std::cout << std::endl;

    std::cout << instruction_to_text(instruction);

    for (auto it = arguments.begin(); it != arguments.end(); ++it) {
        std::cout << " (";
        std::cout << *it;
        std::cout << ")";
    }
    std::cout << std::endl;

    if (instruction == END_EXPRESSION || instruction == ADD_HIST_TO_LIST) std::cout << std::endl;

}

void visit_if(PNode node) {

}

std::string ALIConverter::reserve_scoped_value_name() {
    std::stringstream new_var_name;
    new_var_name << "xVx" << highest_var_val++ << "" << current_scope_name;
    last_value_name = new_var_name.str();
    return last_value_name;
}

std::string ALIConverter::reserve_scoped_limit_name() {
    std::stringstream new_var_name;
    new_var_name << "xLx" << highest_var_val++ << "x" << current_scope_name;
    return new_var_name.str();
}

std::string ALIConverter::reserve_scoped_region_name() {
    std::stringstream new_var_name;
    new_var_name << "xRx" << highest_var_val++ << "x" << current_scope_name;
    return new_var_name.str();
}

std::string ALIConverter::if_operator(PNode node) {
    
}

std::string ALIConverter::interval_operator(PNode node) {

    AnalysisLevelInstruction inst;

    if (node->get_token()->get_token_type() == OUTSIDE) {
        inst = EXPR_OUTSIDE;
    } else {
        inst = EXPR_WITHIN;
    }

    std::string lhs = handle_expression(node->get_children()[0]);

    PNode interval = node->get_children()[1];
    if (interval->get_ast_type() != INTERVAL) raise_analysis_conversion_exception("An interval must follow this token.", node->get_token());
    std::string interval_lhs = handle_expression(interval->get_children()[0]);
    std::string interval_rhs = handle_expression(interval->get_children()[1]);

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
    std::string lexeme = tok->get_lexeme();

    if (tok->get_token_type() == MINUS) {
        node = node->get_children()[0];
        tok = node->get_token();
        std::string lexeme = tok->get_lexeme();

        Token_type tok_type = tok->get_token_type();
        if (tok_type == THIS) {
            tok_type = current_object_token;
            lexeme = current_object_particle_if_named;
        }

        switch(tok_type) {
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

        Token_type tok_type = tok->get_token_type();
        if (tok_type == THIS) {
            tok_type = current_object_token;
            lexeme = current_object_particle_if_named;
        }

        switch(tok_type) {
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
        next_part.add_argument(lexeme);
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

void ALIConverter::visit_bin(PNode node) {
    AnalysisCommand bin(CREATE_BIN);

    visit_children(node);

    // add condition result as input
    bin.add_argument(current_scope_name);

    // add region within which we are binning
    bin.add_argument(current_region);

    command_list.push_back(bin);

}

void ALIConverter::visit_bin_list(PNode node) {
    visit_expression(node->get_children()[0]);

    std::string exp_value_name = current_scope_name;

    if (node->get_children().size() < 3) raise_analysis_conversion_exception("Binning needs at least 2 values to proceed", node->get_children()[1]->get_token());

    for (int i = 2; i < node->get_children().size(); i++) {
        AnalysisCommand within(EXPR_WITHIN);

        std::string condition_result = reserve_scoped_value_name();
        within.add_argument(condition_result);
        within.add_argument(node->get_children()[i-1]->get_token()->get_lexeme());
        within.add_argument(node->get_children()[i]->get_token()->get_lexeme());

        command_list.push_back(within);

        AnalysisCommand bin(CREATE_BIN);
        bin.add_argument(condition_result);
        bin.add_argument(current_region);

        command_list.push_back(bin);
    }

}

void ALIConverter::visit_weight(PNode node) {
    std::string prev_name = current_region;
    current_region = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand weight_apply(WEIGHT_APPLY);
    weight_apply.add_argument(current_region);
    weight_apply.add_argument(prev_name);
    weight_apply.add_argument(node->get_children()[0]->get_token()->get_lexeme());
    weight_apply.add_argument(last_condition_name);

    command_list.push_back(weight_apply);
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

void ALIConverter::visit_particle_sum(PNode node) {
    handle_particle_list(node);
}



std::string ALIConverter::particle_list_function(PNode node) {

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token_type()) {
        case LETTER_E: 
            inst = FUNC_ENERGY; break;
        case LETTER_P: case PT:
            inst = FUNC_PT; break;
        case LETTER_M: case MASS:
            inst = FUNC_MASS; break;
        case MSOFTDROP:
            inst = FUNC_MSOFTDROP; break;
        case LETTER_Q: case CHARGE:
            inst = FUNC_CHARGE; break;
        case FLAVOR: 
            inst = FUNC_FLAVOR; break;
        case CONSTITUENTS: 
            inst = FUNC_CONSTITUENTS; break;
        case PDG_ID: 
            inst = FUNC_PDG_ID; break;
        case JET_ID:
            inst = FUNC_JET_ID; break;
        case IDX: 
            inst = FUNC_IDX; break;
        case IS_TAUTAG: 
            inst = FUNC_TAUTAG; break;
        case IS_CTAG: 
            inst = FUNC_CTAG; break;
        case IS_BTAG: 
            inst = FUNC_BTAG; break;
        case DXY: 
            inst = FUNC_DXY; break;
        case EDXY: 
            inst = FUNC_EDXY; break;
        case EDZ: 
            inst = FUNC_EDZ; break;
        case DZ: 
            inst = FUNC_DZ; break;
        case VERTR: 
            inst = FUNC_VER_TR; break;
        case VERZ: 
            inst = FUNC_VER_Z; break;
        case VERY: 
            inst = FUNC_VER_Y; break;
        case VERX: 
            inst = FUNC_VER_X; break;
        case VERT: 
            inst = FUNC_VER_T; break;
        case GENPART_IDX: 
            inst = FUNC_GEN_PART_IDX; break;
        case PHI: 
            inst = FUNC_PHI; break;
        case RAPIDITY: 
            inst = FUNC_RAPIDITY; break;
        case ETA: 
            inst = FUNC_ETA; break;
        case ABS_ETA: 
            inst = FUNC_ABS_ETA; break;
        case THETA: 
            inst = FUNC_THETA; break;
        case PTCONE: 
            inst = FUNC_PT_CONE; break;
        case ETCONE: 
            inst = FUNC_ET_CONE; break;
        case ABS_ISO: 
            inst = FUNC_ABS_ISO; break;
        case MINI_ISO: 
            inst = FUNC_MINI_ISO; break;
        case IS_TIGHT: 
            inst = FUNC_IS_TIGHT; break;
        case IS_MEDIUM: 
            inst = FUNC_IS_MEDIUM; break;
        case IS_LOOSE: 
            inst = FUNC_IS_LOOSE; break;
        case PZ: 
            inst = FUNC_PZ; break;
        case NBF: 
            inst = FUNC_NBF; break;
        case DR: 
            inst = FUNC_DR; break;
        case DPHI: 
            inst = FUNC_DPHI; break;
        case DETA: 
            inst = FUNC_DETA; break;
        case NUMOF:
            inst = FUNC_SIZE; break;
        case FMT2: 
            inst = FUNC_FMT2; break;
        case FMTAUTAU:
            inst = FUNC_TAUTAU; break;
        case HT:
            inst = FUNC_HT; break;
        case SPHERICITY:
            inst = FUNC_SPHERICITY; break;
        case APLANARITY:
            inst = FUNC_APLANARITY; break;

        default:
            raise_analysis_conversion_exception("Undefined particle function", node->get_token());
            break;

    }

    AnalysisCommand func(inst);

    std::string dest = reserve_scoped_value_name();
    func.add_argument(dest);

    visit_children(node);

    PNode particle_list_node = node->get_children()[0];
    for (auto it = particle_list_node->get_children().begin(); it != particle_list_node->get_children().end(); ++it) {

        bool needs_numeric_index = ((*it)->get_children().size() > 0) && ((*it)->get_children()[0]->get_ast_type() == INDEX);
        std::string indexed_name_if_relevant;

        if (needs_numeric_index) {
            AnalysisCommand make_empty(MAKE_EMPTY_PARTICLE);
            std::string empty_name = reserve_scoped_value_name();
            make_empty.add_argument(empty_name);
            command_list.push_back(make_empty);

            indexed_name_if_relevant = handle_particle(*it, empty_name);
        }

        if ((*it)->get_token()->get_token_type() == FIRST || (*it)->get_token()->get_token_type() == SECOND) {
            AnalysisCommand indexing((*it)->get_token()->get_token_type() == FIRST ? FUNC_FIRST : FUNC_SECOND);
            std::string dest_for_index = reserve_scoped_value_name();
            indexing.add_argument(dest_for_index);

            std::string lexeme_for_child = (*it)->get_children()[0]->get_token()->get_lexeme();
            if ((*it)->get_children()[0]->get_token()->get_token_type() == THIS) lexeme_for_child = current_object_particle_if_named;

            indexing.add_argument(lexeme_for_child);
            command_list.push_back(indexing);
            func.add_argument(dest_for_index);
        } else if ((*it)->get_token()->get_token_type() == THIS) {
            if (needs_numeric_index) {func.add_argument(indexed_name_if_relevant);}
            else {func.add_argument(current_object_particle_if_named);}
        } else {
            if (needs_numeric_index) {func.add_argument(indexed_name_if_relevant);}
            else {func.add_argument((*it)->get_token()->get_lexeme());}
        }
    }
    // std::string particle_name = handle_particle_list(node->get_children()[0]);
    
    // func.add_argument(particle_name);

    command_list.push_back(func);

    return dest;
}


std::string ALIConverter::unary_operator(PNode node) {

    std::string source = handle_expression(node->get_children()[0]);

    AnalysisLevelInstruction inst;

    if (node->get_ast_type() == NEGATE) {
        inst = EXPR_NEGATE;
    } else if (node->get_token()->get_token_type() == NOT) {
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
    std::string lhs = handle_expression(node->get_children()[0]);

    std::string rhs = handle_expression(node->get_children()[1]);

    AnalysisLevelInstruction inst;

    switch(node->get_token()->get_token_type()) {
        case RAISED_TO_POWER:
            inst = EXPR_RAISE; break;
        case MULTIPLY:
            inst = EXPR_MULTIPLY; break;
        case DIVIDE:
            inst = EXPR_DIVIDE; break;
        case PLUS:
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
    std::string source = handle_expression(node->get_children()[0]);
    std::string dest = reserve_scoped_limit_name();

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token_type()) {
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
        case MIN:
            inst = FUNC_MIN; break;
        case MAX:
            inst = FUNC_MAX; break;
        case ANYOCCURRENCES:
            inst = FUNC_ANYOCCURRENCES; break;
        case SORT:
            if (node->get_children()[1]->get_token()->get_token_type() == DESCEND) {
                inst = FUNC_SORT_DESCEND; 
            } else {
                inst = FUNC_SORT_ASCEND;
            }
            break;
            
        default:
            raise_analysis_conversion_exception("Invalid function acting on an expression", node->get_token());
            break;
    }

    if (inst == FUNC_MAX) {
        if (node->get_children().size() > 1) inst = FUNC_MAX_LIST;
    } else if (inst == FUNC_MIN) {
        if (node->get_children().size() > 1) inst = FUNC_MIN_LIST;
    }

    AnalysisCommand func(inst);
    func.add_argument(dest);
    func.add_argument(source);

    if (inst == FUNC_ANYOCCURRENCES || inst == FUNC_MAX_LIST || inst == FUNC_MIN_LIST) {
        std::string second_source = handle_expression(node->get_children()[1]);   
        func.add_argument(second_source);
    } 


    command_list.push_back(func);
    return dest;
}

void ALIConverter::visit_expression(PNode node) {
    std::string resultant_name = handle_expression(node->get_children()[0]);
    last_value_name = resultant_name;
}

std::string ALIConverter::handle_expression(PNode node) {

    if (node->get_ast_type() == USER_FUNCTION) {
        std::string source = handle_expression(node->get_children()[0]);
        std::string dest = reserve_scoped_limit_name();
        AnalysisCommand func(FUNC_NAMED);
        func.add_argument(dest);
        func.add_argument(source);
        func.add_argument(node->get_children()[1]->get_token()->get_lexeme());

    command_list.push_back(func);
    } else if (node->get_ast_type() == NEGATE) {
        return unary_operator(node);
    } else if (node->get_ast_type() == EXPRESSION) {
        return handle_expression(node->get_children()[0]);
    }

    switch(node->get_token()->get_token_type()) {
        case BWL: case BWR: case RAISED_TO_POWER: case MULTIPLY: case DIVIDE: case PLUS: case MINUS: case IRG: case ERG: case MAXIMIZE: case MINIMIZE: case LT: case GT: case LE: case GE: case EQ: case NE: case AMPERSAND: case PIPE: case AND: case OR:
            return binary_operator(node);
        case WITHIN: case OUTSIDE:
            return interval_operator(node);
        case NOT:
            return unary_operator(node);
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: case CHARGE: case MASS:
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case EDXY: case EDZ: case DZ: case VERTR: case VERZ: case VERY: case VERX: case VERT: 
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case ABS_ETA: case THETA: 
        case PTCONE: case ETCONE: case ABS_ISO: case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case  MSOFTDROP: case JET_ID:
        case PT: case PZ: case NBF: case DR: case DPHI: case DETA: case NUMOF: case FMT2: case FMTAUTAU: case HT: case APLANARITY: case SPHERICITY: case FIRST: case SECOND:
            return particle_list_function(node);
        case HSTEP: case DELTA: case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: case SORT: case MIN: case MAX: case ANYOCCURRENCES:
            return expression_function(node);
        default:
            return literal_value(node);
    }
}

void ALIConverter::visit_condition(PNode node) {

    std::stringstream cond_name;
    cond_name << reserve_scoped_value_name() << "xCONDx" << current_scope_name;

    std::string final = handle_expression(node->get_children()[0]);

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
        if_scope_1 << "_IF1_";

        AnalysisCommand if_statement(BEGIN_IF);

        

        AnalysisCommand end_if(END_IF);

        current_scope_name = old_scope;
    }
}

void ALIConverter::visit_sort(PNode node) {
    std::string to_be_sorted = handle_expression(node->get_children()[0]);
    AnalysisLevelInstruction which_way;
    if (node->get_children()[1]->get_token()->get_token_type() == ASCEND) {
        which_way = SORT_ASCEND;
    } else {
        which_way = SORT_DESCEND;
    }

    AnalysisCommand sort(which_way);
    sort.add_argument(reserve_scoped_value_name());
    sort.add_argument(to_be_sorted);

    command_list.push_back(sort);
}

void ALIConverter::visit_histo_use(PNode node) {
    AnalysisCommand hist_use(USE_HIST_LIST);
    hist_use.add_argument(node->get_children()[0]->get_token()->get_lexeme());

    // add the name of the most recent cut, since we are using that for our histogram's working region
    hist_use.add_argument(current_region);
    command_list.push_back(hist_use);
}

void ALIConverter::visit_histo_list(PNode node) {

    std::string prev_scope = current_scope_name;

    std::string histo_list_name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream histo_list_scope_name;
    histo_list_scope_name << "xHLx" << node->get_children()[0]->get_token()->get_lexeme();
    current_scope_name = histo_list_scope_name.str();

    AnalysisCommand hist_list_create(CREATE_HIST_LIST);
    hist_list_create.add_argument(current_scope_name);

    command_list.push_back(hist_list_create);

    visit_children_after_index(node, 0);

    AnalysisCommand hist_list_finalize(ADD_ALIAS);
    hist_list_finalize.add_argument(histo_list_name);
    hist_list_finalize.add_argument(current_scope_name);

    command_list.push_back(hist_list_finalize);

    current_scope_name = prev_scope;
}

void ALIConverter::visit_histogram(PNode node) {

    std::string prev_scope = current_scope_name;

    std::string histo_name = node->get_children()[0]->get_token()->get_lexeme();
    std::stringstream histo_scope_name;
    histo_scope_name << "xHx" << node->get_children()[0]->get_token()->get_lexeme();
    current_scope_name = histo_scope_name.str();

    bool is_2d = false;

    if (node->get_children().size() > 6) is_2d = true;

    AnalysisLevelInstruction inst = HIST_1D;
    if (is_2d) inst = HIST_2D;

    AnalysisCommand hist(inst);

    hist.add_argument(node->get_children()[0]->get_token()->get_lexeme());
    
    hist.add_argument(node->get_children()[1]->get_token()->get_lexeme());

    std::string binning_1d = handle_expression(node->get_children()[2]);
    std::string lower_1d = handle_expression(node->get_children()[3]);
    std::string upper_1d = handle_expression(node->get_children()[4]);

    std::string input_1d;

    if (!is_2d) {
        input_1d = handle_expression(node->get_children()[5]);
    } else {
        input_1d = handle_expression(node->get_children()[8]);
    }

    hist.add_argument(binning_1d);
    hist.add_argument(lower_1d);
    hist.add_argument(upper_1d);
    hist.add_argument(input_1d);

    if (is_2d) {
        std::string binning_2d = handle_expression(node->get_children()[5]);
        std::string lower_2d = handle_expression(node->get_children()[6]);
        std::string upper_2d = handle_expression(node->get_children()[7]);

        std::string input_2d = handle_expression(node->get_children()[9]);

        hist.add_argument(binning_2d);
        hist.add_argument(lower_2d);
        hist.add_argument(upper_2d);
        hist.add_argument(input_2d);
    }

    command_list.push_back(hist);

    if (node->get_ast_type() == HISTOLIST_HISTOGRAM) {
        AnalysisCommand add_to_list(ADD_HIST_TO_LIST);

        add_to_list.add_argument(current_scope_name);
        add_to_list.add_argument(prev_scope);
        add_to_list.add_argument(histo_name);

        command_list.push_back(add_to_list);

    } else {

        AnalysisCommand use_hist(USE_HIST);

        use_hist.add_argument(histo_name);
        use_hist.add_argument(current_region);

        command_list.push_back(use_hist);

        current_scope_name = prev_scope;
    }
}

void ALIConverter::visit_cutflow_use(PNode node) {
    AnalysisCommand cutflow(DO_CUTFLOW_ON_REGION);
    cutflow.add_argument(current_region);
    command_list.push_back(cutflow);
}

void ALIConverter::visit_object_select(PNode node) {

    std::string prev_name = current_limit;
    current_limit = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand limit_mask(LIMIT_MASK);
    limit_mask.add_argument(current_limit);
    limit_mask.add_argument(prev_name);
    limit_mask.add_argument(last_condition_name);

    command_list.push_back(limit_mask);
}

void ALIConverter::visit_object_reject(PNode node) {
    std::string prev_name = current_limit;
    current_limit = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand invert_mask(EXPR_LOGICAL_NOT);
    std::string newly_inverted = reserve_scoped_value_name();
    invert_mask.add_argument(newly_inverted);
    invert_mask.add_argument(last_condition_name);
    
    command_list.push_back(invert_mask);

    AnalysisCommand end_this_expression(END_EXPRESSION);
    std::string final_limit = reserve_scoped_value_name();
    end_this_expression.add_argument(final_limit);
    end_this_expression.add_argument(newly_inverted);

    command_list.push_back(end_this_expression);

    AnalysisCommand limit_mask(LIMIT_MASK);
    limit_mask.add_argument(current_limit);
    limit_mask.add_argument(prev_name);
    limit_mask.add_argument(final_limit);

    command_list.push_back(limit_mask);
    
}

std::string ALIConverter::comb_list(PNode node, std::string prev) {
    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token_type()) {
        case ELECTRON:
            inst = ADD_ELECTRON_TO_COMB; break;
        case MUON:
            inst = ADD_MUON_TO_COMB; break;
        case TAU:
            inst = ADD_TAU_TO_COMB; break;
        case TRACK:
            inst = ADD_TRACK_TO_COMB; break;
        case LEPTON:
            inst = ADD_LEPTON_TO_COMB; break;
        case PHOTON:
            inst = ADD_PHOTON_TO_COMB; break;
        case BJET:
            inst = ADD_BJET_TO_COMB; break;
        case QGJET:
            inst = ADD_QGJET_TO_COMB; break;
        case NUMET:
            inst = ADD_NUMET_TO_COMB; break;
        case METLV:
            inst = ADD_METLV_TO_COMB; break;
        case GEN:
            inst = ADD_GEN_TO_COMB; break;
        case JET:
            inst = ADD_JET_TO_COMB; break;
        case FJET:
            inst = ADD_FJET_TO_COMB; break;
        default:
            inst = ADD_NAMED_TO_COMB; break;
    }

    AnalysisCommand combine(inst);
    std::string dest = reserve_scoped_limit_name();

    combine.add_argument(dest);
    combine.add_argument(prev);

    if (inst == ADD_NAMED_TO_COMB) {
        combine.add_argument(node->get_token()->get_lexeme());
    }

    command_list.push_back(combine);

    return dest;
}

std::string ALIConverter::union_list(PNode node, std::string prev) {

    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token_type()) {
        case ELECTRON:
            inst = ADD_ELECTRON_TO_UNION; break;
        case MUON:
            inst = ADD_MUON_TO_UNION; break;
        case TAU:
            inst = ADD_TAU_TO_UNION; break;
        case TRACK:
            inst = ADD_TRACK_TO_UNION; break;
        case LEPTON:
            inst = ADD_LEPTON_TO_UNION; break;
        case PHOTON:
            inst = ADD_PHOTON_TO_UNION; break;
        case BJET:
            inst = ADD_BJET_TO_UNION; break;
        case QGJET:
            inst = ADD_QGJET_TO_UNION; break;
        case NUMET:
            inst = ADD_NUMET_TO_UNION; break;
        case METLV:
            inst = ADD_METLV_TO_UNION; break;
        case GEN:
            inst = ADD_GEN_TO_UNION; break;
        case JET:
            inst = ADD_JET_TO_UNION; break;
        case FJET:
            inst = ADD_FJET_TO_UNION; break;
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

    PNode union_node = node->get_children()[1]->get_children()[0];
    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream union_name;
    union_name << "xUNIONx" << name;

    current_scope_name = union_name.str();

    std::string source = reserve_scoped_value_name();
    AnalysisCommand make_union(MAKE_EMPTY_UNION);
    make_union.add_argument(source);

    command_list.push_back(make_union);


    for (auto it = union_node->get_children().begin(); it != union_node->get_children().end(); ++it) {
        source = union_list(*it, source);
    }

    AnalysisCommand finalize_union(ADD_ALIAS);
    finalize_union.add_argument(name);
    finalize_union.add_argument(source);

    command_list.push_back(finalize_union);

    current_scope_name = prev_name;

}

void ALIConverter::visit_comb_type(PNode node) {
    std::string prev_name = current_scope_name;

    PNode comb_node = node->get_children()[1]->get_children()[0];
    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream comb_name;
    comb_name << "xCOMBx" << name;

    current_scope_name = comb_name.str();

    std::string source = reserve_scoped_value_name();
    AnalysisCommand make_comb(MAKE_EMPTY_COMB);
    make_comb.add_argument(source);

    command_list.push_back(make_comb);


    for (auto it = comb_node->get_children().begin(); it != comb_node->get_children().end(); ++it) {
        source = comb_list(*it, source);
    }

    AnalysisCommand finalize_comb(ADD_ALIAS);
    finalize_comb.add_argument(name);
    finalize_comb.add_argument(source);

    command_list.push_back(finalize_comb);

    current_scope_name = prev_name;
}

void ALIConverter::visit_object_first_second(PNode node) {

    std::string name = node->get_children()[0]->get_token()->get_lexeme();
    PNode index = node->get_children()[1];
    PNode source = index->get_children()[0];

    AnalysisCommand indexing(index->get_token()->get_token_type() == FIRST ? FUNC_FIRST : FUNC_SECOND);
    std::string dest_for_index = reserve_scoped_value_name();
    indexing.add_argument(dest_for_index);

    std::string lexeme_for_child = source->get_children()[0]->get_token()->get_lexeme();

    indexing.add_argument(lexeme_for_child);
    command_list.push_back(indexing);

    AnalysisCommand finalize_indexing(ADD_ALIAS);
    finalize_indexing.add_argument(name);
    finalize_indexing.add_argument(dest_for_index);

    command_list.push_back(finalize_indexing);
}

void ALIConverter::visit_object(PNode node) {


    
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    PNode source = node->get_children()[1];

    if (source->get_token()->get_token_type() == UNION) {

        visit_union_type(node);
        return;
    } else if (source->get_token()->get_token_type() == COMB) {
        visit_comb_type(node);
        return;
    } else if (source->get_token()->get_token_type() == FIRST || source->get_token()->get_token_type() == SECOND) {
        visit_object_first_second(node);
        return;
    }

    std::string name_lexeme = name->get_token()->get_lexeme();
    std::string source_lexeme = source->get_token()->get_lexeme();

    current_object_token = source->get_token()->get_token_type();
    current_object_particle_if_named = source_lexeme;

    switch (current_object_token) {
        case ELECTRON: 
            source_lexeme = "Electron"; break;
        case MUON: 
            source_lexeme = "Muon"; break;
        case TAU: 
            source_lexeme = "Tau"; break;
        case GEN: 
            source_lexeme = "Gen"; break;
        case PHOTON: 
            source_lexeme = "Photon"; break;
        case JET: 
            source_lexeme = "Jet"; break;
        case FJET: 
            source_lexeme = "FatJet"; break;
        case LEPTON:
            source_lexeme = "Lepton"; break;
        default:
            break;
    }

    // the CREATE_MASK command takes the new mask's name, the target object's name, and the source object's name

    AnalysisCommand create_mask(CREATE_MASK);
    
    std::stringstream mask_name;
    mask_name << "xMASKx" << name_lexeme;

    // move us into the "scope" of this object
    current_scope_name = mask_name.str();
    current_limit = current_scope_name;

    create_mask.add_argument(current_scope_name);
    create_mask.add_argument(source_lexeme);
    command_list.push_back(create_mask);


    visit_children_after_index(node, 1);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand apply_mask(APPLY_MASK);

    apply_mask.add_argument(name_lexeme);
    apply_mask.add_argument(current_limit);
    apply_mask.add_argument(source_lexeme);

    command_list.push_back(apply_mask);
    current_scope_name = prev_name;
}

void ALIConverter::visit_region_select (PNode node) {
    std::string prev_name = current_region;
    current_region = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand cut_region(CUT_REGION);
    cut_region.add_argument(current_region);
    cut_region.add_argument(prev_name);
    cut_region.add_argument(last_condition_name);

    command_list.push_back(cut_region);
}

void ALIConverter::visit_use(PNode node) {
    
    std::string prev_name = current_region;
    current_region = reserve_scoped_value_name();

    std::string name = node->get_children()[0]->get_token()->get_lexeme();
    
    AnalysisCommand merge_regions(MERGE_REGIONS);

    merge_regions.add_argument(current_region);
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
    reg_name << "xREGx" << name_lexeme;

    // move us into the "scope" of this region
    current_scope_name = reg_name.str();
    current_region = reg_name.str();

    create_region.add_argument(current_scope_name);
    command_list.push_back(create_region);

    visit_children_after_index(node, 0);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand add_reg_name(ADD_ALIAS);

    add_reg_name.add_argument(name_lexeme);
    add_reg_name.add_argument(current_region);

    command_list.push_back(add_reg_name);
    current_scope_name = prev_name;

}


void ALIConverter::visit_definition(PNode node) {

    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    std::string name_lexeme = name->get_token()->get_lexeme();

    std::stringstream def_name;
    def_name << "xDEFx" << name_lexeme;

    current_scope_name = def_name.str();


    if (node->get_children()[1]->get_ast_type() == TERMINAL && node->get_children()[1]->get_token()->get_token_type() == EXTERNAL) {
        PNode func = node->get_children()[1]->get_children()[0];
        std::string func_name = func->get_token()->get_lexeme();

        AnalysisCommand add_extern_name(ADD_EXTERNAL);
        add_extern_name.add_argument(name_lexeme);
        add_extern_name.add_argument(func_name);

        command_list.push_back(add_extern_name);
    } else if (node->get_children()[1]->get_ast_type() == TERMINAL && node->get_children()[1]->get_token()->get_token_type() == CORRECTIONLIB) {
        PNode filename_node = node->get_children()[1]->get_children()[0];
        std::string filename = filename_node->get_token()->get_lexeme();

        PNode keyname_node = node->get_children()[1]->get_children()[1];
        std::string keyname = keyname_node->get_token()->get_lexeme();

        AnalysisCommand add_correction(ADD_CORRECTIONLIB);
        add_correction.add_argument(name_lexeme);
        add_correction.add_argument(filename);
        add_correction.add_argument(keyname);

        command_list.push_back(add_correction);
    } else {
        visit_children_after_index(node, 0);

        AnalysisCommand add_def_name(ADD_ALIAS);

        add_def_name.add_argument(name_lexeme);
        add_def_name.add_argument(last_value_name);

        command_list.push_back(add_def_name);
    }
    
    current_scope_name = prev_name;
    
}

// we precompile the table to get it to have the right dimensionality for easy array operations
void ALIConverter::visit_table_def(PNode node) {
    PNode name = node->get_children()[0];
    PNode nvars = node->get_children()[2];
    PNode do_errors_node = node->get_children()[3];
    
    bool do_errors = false;
    if (do_errors_node->get_token()->get_token_type() == TRUE) do_errors = true;

    // size of the actual table
    int table_size = node->get_children().size() - 4;

    int num_vars = std::stoi(nvars->get_token()->get_lexeme());
    
    // get the number of entries this table has implicitly - it should be table_size / ((1 or 3) + 2*num_vars)
    int num_columns_per_row = (do_errors ? 3 : 1) + 2*num_vars;
    if ((table_size % num_columns_per_row) != 0) {
        raise_analysis_conversion_exception("Invalid table, it is not square: likely at least one row is missing at least one component", nvars->get_token());
    }
    int num_entries = table_size / num_columns_per_row;

    std::string current_name = "";
    std::string prev_name = reserve_scoped_value_name();

    AnalysisCommand create_table(CREATE_TABLE);
    create_table.add_argument(prev_name);
    create_table.add_argument(nvars->get_token()->get_lexeme());

    command_list.push_back(create_table);

    int entry = 4;
    for (int row = 0; row < num_entries; row++) {
        AnalysisCommand append_to_table(APPEND_TO_TABLE);

        current_name = reserve_scoped_value_name();
        append_to_table.add_argument(current_name);
        append_to_table.add_argument(prev_name);

        AnalysisCommand create_table_value(CREATE_TABLE_VALUE);
        std::string values_name = reserve_scoped_value_name();
        create_table_value.add_argument(values_name);

        AnalysisCommand create_table_lower_bounds(CREATE_TABLE_LOWER_BOUNDS);
        std::string lower_bound_name = reserve_scoped_value_name();
        create_table_lower_bounds.add_argument(lower_bound_name);

        AnalysisCommand create_table_upper_bounds(CREATE_TABLE_UPPER_BOUNDS);
        std::string upper_bound_name = reserve_scoped_value_name();
        create_table_upper_bounds.add_argument(upper_bound_name);

        append_to_table.add_argument(values_name);
        append_to_table.add_argument(lower_bound_name);
        append_to_table.add_argument(upper_bound_name);

        for (int col = 0; col < num_columns_per_row; col++) {
            std::string cur_lexeme = node->get_children()[entry]->get_token()->get_lexeme();
            if (col <= (do_errors ? 2 : 0)) {
                create_table_value.add_argument(cur_lexeme);
            } else if (col % 2 == 0) {
                create_table_upper_bounds.add_argument(cur_lexeme);
            } else {
                create_table_lower_bounds.add_argument(cur_lexeme);
            }
            
            entry += 1;
        }

        command_list.push_back(create_table_value);
        command_list.push_back(create_table_lower_bounds);
        command_list.push_back(create_table_upper_bounds);
        command_list.push_back(append_to_table);

        prev_name = current_name;
    }

    AnalysisCommand finish_table(FINISH_TABLE);
    finish_table.add_argument(name->get_token()->get_lexeme());
    finish_table.add_argument(prev_name);
    command_list.push_back(finish_table);

}

void ALIConverter::visit_criteria(PNode node) {
    
}

void ALIConverter::visitation(PNode root) {
    visit(root);
}

void ALIConverter::print_commands() {
    for (auto it = command_list.begin(); it != command_list.end(); ++it) {
        it->print_instruction();
    }
}

bool ALIConverter::clear_to_next() {
    if (iter_command >= command_list.size()) return false;
    return true;
}

AnalysisCommand ALIConverter::next_command() {
    return command_list[iter_command++];
}

ALIConverter::ALIConverter(): highest_var_val(0), iter_command(0) {}