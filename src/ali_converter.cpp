#include "ali_converter.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <iomanip>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>


AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok): instruction(inst), source_token(tok), has_dest_argument_yet(false), has_source_token(true)  {

}

AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst): instruction(inst), has_dest_argument_yet(false), source_token(), has_source_token(false)  {

}

void AnalysisCommand::add_dest_argument(std::string arg) {
    assert(!has_dest_argument_yet);
    has_dest_argument_yet = true;
    dest_argument = arg;
}

void AnalysisCommand::add_source_argument(std::string arg) {
    source_arguments.push_back(arg);
}

AnalysisLevelInstruction AnalysisCommand::get_instruction() {
    return instruction;
}
std::string AnalysisCommand::get_argument(int pos) {
    assert(pos >= 0);
    
    if (pos == 0) {
        if (has_dest_argument_yet) return dest_argument;
        assert(source_arguments.size() >= 1);
        return source_arguments[0];
    } else if (has_dest_argument_yet) {
        int pos_in_vec = pos - 1;
        assert(pos_in_vec < source_arguments.size());
        return source_arguments[pos_in_vec];
    } else {
        assert(pos < source_arguments.size());
        return source_arguments[pos];
    }

}

bool AnalysisCommand::has_dest_argument() {
    return has_dest_argument_yet;
}

std::string AnalysisCommand::get_dest_argument() {
    assert(has_dest_argument_yet);
    return dest_argument;
}

std::string AnalysisCommand::get_source_argument(int pos) {
    assert(pos < source_arguments.size()); 
    return source_arguments[pos];
}

int AnalysisCommand::get_num_arguments() {
    return static_cast<int>(has_dest_argument_yet) + source_arguments.size();
}


std::string AnalysisCommand::instruction_to_text(AnalysisLevelInstruction inst) {

    switch (inst) {
        case CREATE_REGION:
            return "CREATE_REGION";
        case MERGE_REGIONS:
            return "MERGE_REGIONS";
        case CUT_REGION:
            return "CUT_REGION";
        case ADD_ALIAS:
            return "ADD_ALIAS";
        case ADD_EXTERNAL:
            return "ADD_EXTERNAL";
        case ADD_CORRECTIONLIB:
            return "ADD_CORRECTIONLIB";
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
        case DO_EVENTLIST_ON_REGION:
            return "DO_EVENTLIST_ON_REGION";

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
        case FUNC_DZ:
            return "FUNC_DZ";
        case FUNC_IS_TIGHT:
            return "FUNC_IS_TIGHT";
        case FUNC_IS_MEDIUM:
            return "FUNC_IS_MEDIUM";
        case FUNC_IS_LOOSE:
            return "FUNC_IS_LOOSE";
        case FUNC_THETA:
            return "FUNC_THETA";
        case FUNC_ABS_ISO:
            return "FUNC_ABS_ISO";
        case FUNC_MINI_ISO:
            return "FUNC_MINI_ISO";
        case FUNC_DR:
            return "FUNC_DR";
        case FUNC_DPHI:
            return "FUNC_DPHI";
        case FUNC_DETA:
            return "FUNC_DETA";
        case FUNC_DR_HADAMARD:
            return "FUNC_DR_HADAMARD";
        case FUNC_DPHI_HADAMARD:
            return "FUNC_DPHI_HADAMARD";
        case FUNC_DETA_HADAMARD:
            return "FUNC_DETA_HADAMARD";
        case FUNC_SIZE:
            return "FUNC_SIZE";
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

        case EXPR_WITHIN_EXCLUSIVE:
            return "EXPR_WITHIN_EXCLUSIVE";
        case EXPR_WITHIN_LEFT_EXCLUSIVE:
            return "EXPR_WITHIN_LEFT_EXCLUSIVE";
        case EXPR_WITHIN_RIGHT_EXCLUSIVE:
            return "EXPR_WITHIN_RIGHT_EXCLUSIVE";
        case NAME_ELEMENT_OF_COMB:
            return "NAME_ELEMENT_OF_COMB";
        case MAKE_EMPTY_DISJOINT:
            return "MAKE_EMPTY_DISJOINT";
        case ADD_NAMED_TO_DISJOINT:
            return "ADD_NAMED_TO_DISJOINT";
        case ADD_ELECTRON_TO_DISJOINT:
            return "ADD_ELECTRON_TO_DISJOINT";
        case ADD_MUON_TO_DISJOINT:
            return "ADD_MUON_TO_DISJOINT";
        case ADD_TAU_TO_DISJOINT:
            return "ADD_TAU_TO_DISJOINT";
        case ADD_TRACK_TO_DISJOINT:
            return "ADD_TRACK_TO_DISJOINT";
        case ADD_LEPTON_TO_DISJOINT:
            return "ADD_LEPTON_TO_DISJOINT";
        case ADD_PHOTON_TO_DISJOINT:
            return "ADD_PHOTON_TO_DISJOINT";
        case ADD_BJET_TO_DISJOINT:
            return "ADD_BJET_TO_DISJOINT";
        case ADD_QGJET_TO_DISJOINT:
            return "ADD_QGJET_TO_DISJOINT";
        case ADD_NUMET_TO_DISJOINT:
            return "ADD_NUMET_TO_DISJOINT";
        case ADD_METLV_TO_DISJOINT:
            return "ADD_METLV_TO_DISJOINT";
        case ADD_GEN_TO_DISJOINT:
            return "ADD_GEN_TO_DISJOINT";
        case ADD_JET_TO_DISJOINT:
            return "ADD_JET_TO_DISJOINT";
        case ADD_FJET_TO_DISJOINT:
            return "ADD_FJET_TO_DISJOINT";
        case NAME_ELEMENT_OF_DISJOINT:
            return "NAME_ELEMENT_OF_DISJOINT";
          break;
        }
}

void AnalysisCommand::print_instruction(int width_of_dest, int width_of_inst) {
    
    if (instruction == MAKE_EMPTY_PARTICLE || instruction == MAKE_EMPTY_UNION || instruction == MAKE_EMPTY_COMB || instruction == CREATE_REGION || instruction == CREATE_MASK) std::cout << std::endl;


    std::cout << std::left << std::setw(width_of_dest) << (std::stringstream() << "(" << (has_dest_argument_yet ? dest_argument : "") << ") ").str() << std::left << std::setw(2) << " <- ";

    std::cout << std::left << std::setw(width_of_inst) << instruction_to_text(instruction);

    std::stringstream args;

    for (auto it = source_arguments.begin(); it != source_arguments.end(); ++it) {
        args << " (";
        args << *it;
        args << ")";
    }

    std::cout << std::left << args.str();
    std::cout << std::endl;

    if (instruction == END_EXPRESSION || instruction == ADD_HIST_TO_LIST) std::cout << std::endl;

}

void AnalysisCommand::print_instruction() {
    print_instruction(0,0);
}

void visit_if(PNode node) {

}

std::string ALILConverter::reserve_scoped_value_name() {
    std::stringstream new_var_name;
    new_var_name << "_V" << highest_var_val++ << "" << current_scope_name;
    last_value_name = new_var_name.str();
    return last_value_name;
}

std::string ALILConverter::reserve_scoped_limit_name() {
    std::stringstream new_var_name;
    new_var_name << "_L" << highest_var_val++ << "" << current_scope_name;
    return new_var_name.str();
}

std::string ALILConverter::reserve_scoped_region_name() {
    std::stringstream new_var_name;
    new_var_name << "_R" << highest_var_val++ << "" << current_scope_name;
    return new_var_name.str();
}

std::string ALILConverter::if_operator(PNode node) {
    
}




std::string ALILConverter::handle_particle(PNode node, std::string last_part) {
    AnalysisLevelInstruction inst;

    PToken tok = node->get_token();
    std::string lexeme = tok->get_lexeme();

    if (tok->get_token_type() == ARROW_INDEX) {
        lexeme = binary_operator(node);
    }

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


    AnalysisCommand next_part(inst, tok);

    std::string part_name = reserve_scoped_value_name();
    next_part.add_dest_argument(part_name);

    if (inst == ADD_PART_NAMED || inst == SUB_PART_NAMED) {
        next_part.add_source_argument(lexeme);
    }

    next_part.add_source_argument(last_part);

    if (node->get_children().size() > 0 && node->get_children()[0]->get_ast_type() == INDEX) {
        next_part.add_source_argument(node->get_children()[0]->get_children()[0]->get_token()->get_lexeme());
        if (node->get_children()[0]->get_children().size() > 1) {
            next_part.add_source_argument(node->get_children()[0]->get_children()[1]->get_token()->get_lexeme());
        }
    }

    command_list.push_back(next_part);

    return part_name;
}

void ALILConverter::visit_bin(PNode node) {
    AnalysisCommand bin(CREATE_BIN);

    visit_children(node);

    // add condition result as input
    bin.add_source_argument(current_scope_name);

    // add region within which we are binning
    bin.add_source_argument(current_region);

    command_list.push_back(bin);

}

void ALILConverter::visit_bin_list(PNode node) {
    visit_expression(node->get_children()[0]);

    std::string exp_value_name = current_scope_name;

    if (node->get_children().size() < 3) raise_analysis_conversion_exception("Binning needs at least 2 values to proceed", node->get_children()[1]->get_token());

    for (int i = 2; i < node->get_children().size(); i++) {
        AnalysisCommand within(EXPR_WITHIN);

        std::string condition_result = reserve_scoped_value_name();
        within.add_dest_argument(condition_result);
        within.add_source_argument(node->get_children()[i-1]->get_token()->get_lexeme());
        within.add_source_argument(node->get_children()[i]->get_token()->get_lexeme());

        command_list.push_back(within);

        AnalysisCommand bin(CREATE_BIN);
        bin.add_source_argument(condition_result);
        bin.add_source_argument(current_region);

        command_list.push_back(bin);
    }

}

void ALILConverter::visit_weight(PNode node) {
    std::string prev_name = current_region;
    current_region = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand weight_apply(WEIGHT_APPLY, node->get_children()[0]->get_token());
    weight_apply.add_dest_argument(current_region);
    weight_apply.add_source_argument(prev_name);
    weight_apply.add_source_argument(node->get_children()[0]->get_token()->get_lexeme());
    weight_apply.add_source_argument(last_condition_name);

    command_list.push_back(weight_apply);
}

std::string ALILConverter::handle_particle_list(PNode node) {

    AnalysisCommand start(MAKE_EMPTY_PARTICLE);

    std::string last_part = reserve_scoped_value_name();
    start.add_dest_argument(last_part);
    command_list.push_back(start);

    for (auto it = node->get_children().begin(); it != node->get_children().end(); ++it) {

        last_part = handle_particle(*it, last_part);

    }

    return last_part;
}

void ALILConverter::visit_particle_sum(PNode node) {
    handle_particle_list(node);
}



std::string ALILConverter::particle_list_function(PNode node) {

    AnalysisLevelInstruction inst;
    auto function_node = node->get_token()->get_token_type() == DOT_INDEX ? node->get_children()[1] : node;

    switch (function_node->get_token()->get_token_type()) {
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
        case DZ: 
            inst = FUNC_DZ; break;
        case GENPART_IDX: 
            inst = FUNC_GEN_PART_IDX; break;
        case PHI: 
            inst = FUNC_PHI; break;
        case RAPIDITY: 
            inst = FUNC_RAPIDITY; break;
        case ETA: 
            inst = FUNC_ETA; break;
        case THETA: 
            inst = FUNC_THETA; break;
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
        case DR: 
            inst = FUNC_DR; break;
        case DPHI: 
            inst = FUNC_DPHI; break;
        case DETA: 
            inst = FUNC_DETA; break;
        case DR_HADAMARD:
            inst = FUNC_DR_HADAMARD; break;
        case DPHI_HADAMARD:
            inst = FUNC_DPHI_HADAMARD; break;
        case DETA_HADAMARD:
            inst = FUNC_DETA_HADAMARD; break;
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
            raise_analysis_conversion_exception("Undefined particle function", function_node->get_token());
            break;

    }

    AnalysisCommand func(inst, function_node->get_token());

    std::string dest = reserve_scoped_value_name();
    func.add_dest_argument(dest);

    visit_children(node);

    if (node->get_token()->get_token_type() == DOT_INDEX) {
        // in this case we definitely have only one particle here by the nature of a dot attribute.
        auto child_node = node->get_children()[0];
        bool needs_numeric_index = node->get_children().size() > 0 && node->get_children()[0]->get_ast_type() == INDEX;

        //TODO: improve this
        // make an empty particle structure
        AnalysisCommand make_empty(MAKE_EMPTY_PARTICLE);
        std::string empty_name = reserve_scoped_value_name();
        make_empty.add_dest_argument(empty_name);
        command_list.push_back(make_empty);

        std::string name_of_particle_variable = handle_particle(child_node, empty_name);
        
        func.add_source_argument(name_of_particle_variable);
    } else {
        // iterate through all children of this function e.g. the relevant particles
            
        PNode particle_list_node = node->get_children()[0];
        for (auto it = particle_list_node->get_children().begin(); it != particle_list_node->get_children().end(); ++it) {

            bool needs_numeric_index = ((*it)->get_children().size() > 0) && ((*it)->get_children()[0]->get_ast_type() == INDEX);

            // make an empty particle structure
            AnalysisCommand make_empty(MAKE_EMPTY_PARTICLE);
            std::string empty_name = reserve_scoped_value_name();
            make_empty.add_dest_argument(empty_name);
            command_list.push_back(make_empty);

            std::string name_of_particle_variable = handle_particle(*it, empty_name);
            
            func.add_source_argument(name_of_particle_variable);
        }
    }

    command_list.push_back(func);

    return dest;
}


std::string ALILConverter::unary_operator(PNode node) {

    std::string source = handle_expression(node->get_children()[0]);

    AnalysisLevelInstruction inst;

    if (node->get_ast_type() == NEGATE) {
        inst = EXPR_NEGATE;
    } else if (node->get_token()->get_token_type() == NOT) {
        inst = EXPR_LOGICAL_NOT;
    }

    AnalysisCommand unary_exp(inst, node->get_token());

    std::string dest = reserve_scoped_value_name();

    unary_exp.add_dest_argument(dest);
    unary_exp.add_source_argument(source);

    command_list.push_back(unary_exp);
    return dest;
    
}

std::string ALILConverter::interval_operator(PNode node) {

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

    AnalysisCommand interval_exp(inst, node->get_token());

    std::string dest = reserve_scoped_value_name();

    interval_exp.add_dest_argument(dest);
    interval_exp.add_source_argument(lhs);
    interval_exp.add_source_argument(interval_lhs);
    interval_exp.add_source_argument(interval_rhs);

    command_list.push_back(interval_exp);

    return dest;

}


std::string ALILConverter::comparison_operator(PNode node) {

    Token_type lhs_tok = node->get_children()[0]->get_token()->get_token_type();
    Token_type rhs_tok = node->get_children()[1]->get_token()->get_token_type();

    bool lhs_is_comparator = lhs_tok == GT || lhs_tok == LT || lhs_tok == LE || lhs_tok == GE;
    bool rhs_is_comparator = rhs_tok == GT || rhs_tok == LT || rhs_tok == LE || rhs_tok == GE;

    // in case of normal comparison with non-comparator inputs, proceed as normal
    if (!lhs_is_comparator && !rhs_is_comparator) {
        return binary_operator(node);
    } else if (lhs_is_comparator && rhs_is_comparator) {
        raise_analysis_conversion_exception("Invalid chained comparison interval, too many comparisons in a row", node->get_token());
        return "";
    }

    // at this point, exactly one child is the comparator
    PNode child_node;
    PNode left_bound;
    PNode right_bound;
    PNode center_operation;

    if (lhs_is_comparator) {
        // if the left child is the comparator, we have its right child as the center and left as the left end  
        child_node = node->get_children()[0];
        left_bound = child_node->get_children()[0];
        center_operation = child_node->get_children()[1];
        right_bound = node->get_children()[1];
    } else {
        // if the right child is the comparator, we have its left child as the center and right as the right end 
        child_node = node->get_children()[1];
        left_bound = node->get_children()[0];
        center_operation = child_node->get_children()[0];
        right_bound = child_node->get_children()[1];
    }

    //TODO: add an error condition for when we have more iterated comparisons within either side

    std::string primary = handle_expression(center_operation);

    std::string interval_lhs = handle_expression(left_bound);
    std::string interval_rhs = handle_expression(right_bound);

    AnalysisCommand interval_exp(EXPR_WITHIN, center_operation->get_token());

    std::string dest = reserve_scoped_value_name();

    interval_exp.add_dest_argument(dest);
    interval_exp.add_source_argument(primary);
    interval_exp.add_source_argument(interval_lhs);
    interval_exp.add_source_argument(interval_rhs);

    command_list.push_back(interval_exp);

    return dest;
}


std::string ALILConverter::binary_operator(PNode node) {

    Token_type type = node->get_token()->get_token_type();

    if (type == ARROW_INDEX) {
        // simply combine the names again - this is not a real operation in ALIL, and is just sugar from ADL
        std::string left_name = node->get_children()[0]->get_token()->get_lexeme();
        std::string right_name = node->get_children()[1]->get_token()->get_lexeme();

        std::stringstream full_name;
        full_name << left_name << "->" << right_name;
        return full_name.str();
    } 

    std::string lhs = handle_expression(node->get_children()[0]);
    std::string rhs = handle_expression(node->get_children()[1]);

    AnalysisLevelInstruction inst;

    switch(type) {
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

    AnalysisCommand binary_exp(inst, node->get_token());

    std::string dest = reserve_scoped_value_name();

    binary_exp.add_dest_argument(dest);
    binary_exp.add_source_argument(lhs);
    binary_exp.add_source_argument(rhs);

    command_list.push_back(binary_exp);
    return dest;
}

std::string ALILConverter::literal_value(PNode node) {
    AnalysisCommand assign(ADD_ALIAS, node->get_token());

    std::string dest = reserve_scoped_value_name();
    assign.add_dest_argument(dest);
    assign.add_source_argument(node->get_token()->get_lexeme());

    command_list.push_back(assign);

    return dest;
}


std::string ALILConverter::function_handler(PNode node) {

    Token_type type = node->get_token()->get_token_type();

    // if we have a dot function, the rhs tells us what kind it is
    if (type == DOT_INDEX) type = node->get_children()[1]->get_token()->get_token_type();

    switch (type) {
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: case CHARGE: case MASS:
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case DZ:
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case THETA: 
        case ABS_ISO: case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case  MSOFTDROP: case JET_ID:
        case PT: case PZ: case DR: case DPHI: case DETA: case DR_HADAMARD: case DPHI_HADAMARD: case DETA_HADAMARD: case NUMOF: case FMT2: case FMTAUTAU: case HT: case APLANARITY: case SPHERICITY: case FIRST: case SECOND:
            return particle_list_function(node);

        case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: case SORT: case MIN: case MAX: case ANYOCCURRENCES:
        default:
            return expression_function(node);
    }
}



std::string ALILConverter::expression_function(PNode node) {

    std::string source = handle_expression(node->get_children()[0]);
    std::string dest = reserve_scoped_limit_name();

    AnalysisLevelInstruction inst;

    auto function_node = node->get_token()->get_token_type() == DOT_INDEX ? node->get_children()[1] : node ;

    switch (function_node->get_token()->get_token_type()) {
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
        if (function_node->get_children().size() > 1) inst = FUNC_MAX_LIST;
    } else if (inst == FUNC_MIN) {
        if (function_node->get_children().size() > 1) inst = FUNC_MIN_LIST;
    }

    AnalysisCommand func(inst, function_node->get_token());
    func.add_dest_argument(dest);
    func.add_source_argument(source);

    if (inst == FUNC_ANYOCCURRENCES || inst == FUNC_MAX_LIST || inst == FUNC_MIN_LIST) {
        std::string second_source = handle_expression(function_node->get_children()[1]);   
        func.add_source_argument(second_source);
    } 


    command_list.push_back(func);
    return dest;
}

void ALILConverter::visit_expression(PNode node) {
    std::string resultant_name = handle_expression(node->get_children()[0]);
    last_value_name = resultant_name;
}

std::string ALILConverter::handle_expression(PNode node) {

    if (node->get_ast_type() == USER_FUNCTION) {
        std::string source = handle_expression(node->get_children()[0]);
        std::string dest = reserve_scoped_limit_name();
        AnalysisCommand func(FUNC_NAMED);
        func.add_dest_argument(dest);
        func.add_source_argument(source);
        func.add_source_argument(node->get_children()[1]->get_token()->get_lexeme());

        command_list.push_back(func);
    } else if (node->get_ast_type() == NEGATE) {
        return unary_operator(node);
    } else if (node->get_ast_type() == EXPRESSION) {
        return handle_expression(node->get_children()[0]);
    }

    switch(node->get_token()->get_token_type()) {
        case GT: case LT: case LE: case GE:
            return comparison_operator(node);
        case BWL: case BWR: case RAISED_TO_POWER: case MULTIPLY: case DIVIDE: case PLUS: case MINUS: case IRG: case ERG: case MAXIMIZE: case MINIMIZE:  case EQ: case NE: case AMPERSAND: case PIPE: case AND: case OR: case ARROW_INDEX:
            return binary_operator(node);
        case WITHIN: case OUTSIDE:
            return interval_operator(node);
        case NOT:
            return unary_operator(node);
        case LETTER_E: case LETTER_P: case LETTER_M: case LETTER_Q: case CHARGE: case MASS:
        case FLAVOR: case CONSTITUENTS: case PDG_ID: case IDX: case IS_TAUTAG: case IS_CTAG: case IS_BTAG: 
        case DXY: case DZ:
        case GENPART_IDX: case PHI: case RAPIDITY: case ETA: case THETA: 
        case ABS_ISO: case MINI_ISO: case IS_TIGHT: case IS_MEDIUM: case IS_LOOSE: 
        case  MSOFTDROP: case JET_ID:
        case PT: case PZ: case DR: case DPHI: case DETA: case DR_HADAMARD: case DPHI_HADAMARD: case DETA_HADAMARD: case NUMOF: case FMT2: case FMTAUTAU: case HT: case APLANARITY: case SPHERICITY: case FIRST: case SECOND:
        case DOT_INDEX:
            return function_handler(node);
        case ANYOF: case ALLOF: case SQRT: case ABS: case COS:  case SIN: case TAN: case SINH: case COSH: case TANH: case EXP: case LOG: case AVE: case SUM: case SORT: case MIN: case MAX: case ANYOCCURRENCES:
            return function_handler(node);
        default:
            return literal_value(node);
    }
}

void ALILConverter::visit_condition(PNode node) {

    std::stringstream cond_name;
    cond_name << reserve_scoped_value_name() << "_COND";

    std::string final = handle_expression(node->get_children()[0]);

    AnalysisCommand end_condition(END_EXPRESSION);
    end_condition.add_dest_argument(cond_name.str());
    end_condition.add_source_argument(final);

    command_list.push_back(end_condition);
    last_condition_name = cond_name.str();
}

void ALILConverter::visit_if(PNode node) {
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

void ALILConverter::visit_sort(PNode node) {
    std::string to_be_sorted = handle_expression(node->get_children()[0]);
    AnalysisLevelInstruction which_way;
    if (node->get_children()[1]->get_token()->get_token_type() == ASCEND) {
        which_way = SORT_ASCEND;
    } else {
        which_way = SORT_DESCEND;
    }

    AnalysisCommand sort(which_way, node->get_children()[1]->get_token());
    sort.add_dest_argument(reserve_scoped_value_name());
    sort.add_source_argument(to_be_sorted);

    command_list.push_back(sort);
}

void ALILConverter::visit_histo_use(PNode node) {
    AnalysisCommand hist_use(USE_HIST_LIST, node->get_children()[0]->get_token());
    hist_use.add_source_argument(node->get_children()[0]->get_token()->get_lexeme());

    // add the name of the most recent cut, since we are using that for our histogram's working region
    hist_use.add_source_argument(current_region);
    command_list.push_back(hist_use);
}

void ALILConverter::visit_histo_list(PNode node) {

    std::string prev_scope = current_scope_name;

    std::string histo_list_name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream histo_list_scope_name;
    histo_list_scope_name << "_HL" << node->get_children()[0]->get_token()->get_lexeme();
    current_scope_name = histo_list_scope_name.str();

    AnalysisCommand hist_list_create(CREATE_HIST_LIST, node->get_children()[0]->get_token());
    hist_list_create.add_dest_argument(current_scope_name);

    command_list.push_back(hist_list_create);

    visit_children_after_index(node, 0);

    AnalysisCommand hist_list_finalize(ADD_ALIAS);
    hist_list_finalize.add_dest_argument(histo_list_name);
    hist_list_finalize.add_source_argument(current_scope_name);

    command_list.push_back(hist_list_finalize);

    current_scope_name = prev_scope;
}

void ALILConverter::visit_histogram(PNode node) {

    std::string prev_scope = current_scope_name;

    std::string histo_name = node->get_children()[0]->get_token()->get_lexeme();
    std::stringstream histo_scope_name;
    histo_scope_name << "_H" << histo_name;
    current_scope_name = histo_scope_name.str();

    bool is_2d = false;

    if (node->get_children().size() > 6) is_2d = true;

    AnalysisLevelInstruction inst = HIST_1D;
    if (is_2d) inst = HIST_2D;

    AnalysisCommand hist(inst, node->get_children()[0]->get_token());

    hist.add_source_argument(node->get_children()[0]->get_token()->get_lexeme());
    
    hist.add_source_argument(node->get_children()[1]->get_token()->get_lexeme());

    std::string binning_1d = handle_expression(node->get_children()[2]);
    std::string lower_1d = handle_expression(node->get_children()[3]);
    std::string upper_1d = handle_expression(node->get_children()[4]);

    std::string input_1d;

    if (!is_2d) {
        input_1d = handle_expression(node->get_children()[5]);
    } else {
        input_1d = handle_expression(node->get_children()[8]);
    }

    hist.add_source_argument(binning_1d);
    hist.add_source_argument(lower_1d);
    hist.add_source_argument(upper_1d);
    hist.add_source_argument(input_1d);

    if (is_2d) {
        std::string binning_2d = handle_expression(node->get_children()[5]);
        std::string lower_2d = handle_expression(node->get_children()[6]);
        std::string upper_2d = handle_expression(node->get_children()[7]);

        std::string input_2d = handle_expression(node->get_children()[9]);

        hist.add_source_argument(binning_2d);
        hist.add_source_argument(lower_2d);
        hist.add_source_argument(upper_2d);
        hist.add_source_argument(input_2d);
    }

    command_list.push_back(hist);

    if (node->get_ast_type() == HISTOLIST_HISTOGRAM) {
        AnalysisCommand add_to_list(ADD_HIST_TO_LIST);

        add_to_list.add_dest_argument(current_scope_name);
        add_to_list.add_source_argument(prev_scope);
        add_to_list.add_source_argument(histo_name);

        command_list.push_back(add_to_list);

    } else {

        AnalysisCommand use_hist(USE_HIST);

        use_hist.add_source_argument(histo_name);
        use_hist.add_source_argument(current_region);

        command_list.push_back(use_hist);

        current_scope_name = prev_scope;
    }
}

void ALILConverter::visit_object_select(PNode node) {

    std::string prev_name = current_limit;
    current_limit = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand limit_mask(LIMIT_MASK);
    limit_mask.add_dest_argument(current_limit);
    limit_mask.add_source_argument(prev_name);


    if (node->get_children()[0]->get_ast_type() == TERMINAL) {
        Token_type tok = node->get_children()[0]->get_token()->get_token_type();
        if (tok == ALL) last_condition_name = "ALL";
        else if (tok == NONE) last_condition_name = "NONE";
    }

    limit_mask.add_source_argument(last_condition_name);
    command_list.push_back(limit_mask);
}

void ALILConverter::visit_object_reject(PNode node) {
    std::string prev_name = current_limit;
    current_limit = reserve_scoped_limit_name();

    visit_children(node);

    if (node->get_children()[0]->get_ast_type() == TERMINAL) {
        Token_type tok = node->get_children()[0]->get_token()->get_token_type();

        if (tok == ALL) last_condition_name = "ALL";
        else if (tok == NONE) last_condition_name = "NONE"; 
    }


    AnalysisCommand invert_mask(EXPR_LOGICAL_NOT);
    std::string newly_inverted = reserve_scoped_value_name();
    invert_mask.add_dest_argument(newly_inverted);
    invert_mask.add_source_argument(last_condition_name);
    
    command_list.push_back(invert_mask);

    AnalysisCommand end_this_expression(END_EXPRESSION);
    std::string final_limit = reserve_scoped_value_name();
    end_this_expression.add_dest_argument(final_limit);
    end_this_expression.add_source_argument(newly_inverted);

    command_list.push_back(end_this_expression);

    AnalysisCommand limit_mask(LIMIT_MASK);
    limit_mask.add_dest_argument(current_limit);
    limit_mask.add_source_argument(prev_name);
    limit_mask.add_source_argument(final_limit);

    command_list.push_back(limit_mask);
    
}

std::string ALILConverter::comb_list(PNode node, std::string prev, bool is_comb) {
    AnalysisLevelInstruction inst;

    switch (node->get_token()->get_token_type()) {
        case ELECTRON:
            inst = is_comb ? ADD_ELECTRON_TO_COMB : ADD_ELECTRON_TO_DISJOINT; break;
        case MUON:
            inst = is_comb ? ADD_MUON_TO_COMB : ADD_MUON_TO_DISJOINT; break;
        case TAU:
            inst = is_comb ? ADD_TAU_TO_COMB : ADD_TAU_TO_DISJOINT; break;
        case TRACK:
            inst = is_comb ? ADD_TRACK_TO_COMB : ADD_TRACK_TO_DISJOINT; break;
        case LEPTON:
            inst = is_comb ? ADD_LEPTON_TO_COMB : ADD_LEPTON_TO_DISJOINT; break;
        case PHOTON:
            inst = is_comb ? ADD_PHOTON_TO_COMB : ADD_PHOTON_TO_DISJOINT; break;
        case BJET:
            inst = is_comb ? ADD_BJET_TO_COMB : ADD_BJET_TO_DISJOINT; break;
        case QGJET:
            inst = is_comb ? ADD_QGJET_TO_COMB : ADD_QGJET_TO_DISJOINT; break;
        case NUMET:
            inst = is_comb ? ADD_NUMET_TO_COMB : ADD_NUMET_TO_DISJOINT; break;
        case METLV:
            inst = is_comb ? ADD_METLV_TO_COMB : ADD_METLV_TO_DISJOINT; break;
        case GEN:
            inst = is_comb ? ADD_GEN_TO_COMB : ADD_GEN_TO_DISJOINT; break;
        case JET:
            inst = is_comb ? ADD_JET_TO_COMB : ADD_JET_TO_DISJOINT; break;
        case FJET:
            inst = is_comb ? ADD_FJET_TO_COMB : ADD_FJET_TO_DISJOINT; break;
        default:
            inst = is_comb ? ADD_NAMED_TO_COMB : ADD_NAMED_TO_DISJOINT; break;
    }

    AnalysisCommand combine(inst);
    std::string dest = reserve_scoped_limit_name();

    combine.add_dest_argument(dest);
    combine.add_source_argument(prev);

    if (inst == ADD_NAMED_TO_COMB || inst == ADD_NAMED_TO_DISJOINT) {
        combine.add_source_argument(node->get_token()->get_lexeme());
    }

    command_list.push_back(combine);

    return dest;
}

std::string ALILConverter::union_list(PNode node, std::string prev) {

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

    AnalysisCommand unite(inst, node->get_token());
    std::string dest = reserve_scoped_limit_name();

    unite.add_dest_argument(dest);
    unite.add_source_argument(prev);

    if (inst == ADD_NAMED_TO_UNION) {
        unite.add_source_argument(node->get_token()->get_lexeme());
    }

    command_list.push_back(unite);

    return dest;
}

void ALILConverter::visit_union_type(PNode node) {

    std::string prev_name = current_scope_name;

    PNode union_node = node->get_children()[1]->get_children()[0];
    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream union_name;
    union_name << "_UNION" << name;

    current_scope_name = union_name.str();

    std::string source = reserve_scoped_value_name();
    AnalysisCommand make_union(MAKE_EMPTY_UNION);
    make_union.add_dest_argument(source);

    command_list.push_back(make_union);


    for (auto it = union_node->get_children().begin(); it != union_node->get_children().end(); ++it) {
        source = union_list(*it, source);
    }

    AnalysisCommand finalize_union(ADD_ALIAS);
    finalize_union.add_dest_argument(name);
    finalize_union.add_source_argument(source);

    command_list.push_back(finalize_union);

    current_scope_name = prev_name;

}

void ALILConverter::visit_comb_type(PNode node) {

    current_defined_variables_within_comb.clear();

    bool is_comb = node->get_children()[1]->get_token()->get_token_type() == COMB;

    std::string prev_name = current_scope_name;

    PNode comb_node = node->get_children()[1]->get_children()[0];
    std::string name = node->get_children()[0]->get_token()->get_lexeme();

    std::stringstream comb_name;
    comb_name << (is_comb ? "_COMB" : "_DISJOINT") << name;

    current_scope_name = comb_name.str();

    std::string source = reserve_scoped_value_name();
    AnalysisCommand make_comb(is_comb ? MAKE_EMPTY_COMB : MAKE_EMPTY_DISJOINT);
    make_comb.add_dest_argument(source);

    command_list.push_back(make_comb);

    // iterate by 2 so we get only the particle bits
    for (auto it = comb_node->get_children().begin(); it != comb_node->get_children().end(); ++(++it)) {
        source = comb_list(*it, source, is_comb);
    }

    AnalysisCommand finalize_comb(ADD_ALIAS);
    finalize_comb.add_dest_argument(name);
    finalize_comb.add_source_argument(source);

    command_list.push_back(finalize_comb);

    // give names to all the arguments of the comb
    PNode names_node = node->get_children()[1]->get_children()[0];

    int i = 0;
    for (auto it = names_node->get_children().begin(); it != names_node->get_children().end(); ++it) {
        // ignore even children, as those are particles
        ++it;
        AnalysisCommand name_comb_arg(is_comb ? NAME_ELEMENT_OF_COMB : NAME_ELEMENT_OF_DISJOINT);

        std::string dest_comb = (*it)->get_token()->get_lexeme();

        name_comb_arg.add_dest_argument(dest_comb);
        name_comb_arg.add_source_argument(name);
        name_comb_arg.add_source_argument(std::to_string(i++));

        current_defined_variables_within_comb.push_back(dest_comb);

        command_list.push_back(name_comb_arg);
    }

    current_scope_name = prev_name;

    return;
}

void ALILConverter::visit_object_first_second(PNode node) {

    std::string name = node->get_children()[0]->get_token()->get_lexeme();
    PNode index = node->get_children()[1];
    PNode source = index->get_children()[0];

    AnalysisCommand indexing(index->get_token()->get_token_type() == FIRST ? FUNC_FIRST : FUNC_SECOND, index->get_token());
    std::string dest_for_index = reserve_scoped_value_name();
    indexing.add_dest_argument(dest_for_index);

    std::string lexeme_for_child = source->get_children()[0]->get_token()->get_lexeme();

    indexing.add_source_argument(lexeme_for_child);
    command_list.push_back(indexing);

    AnalysisCommand finalize_indexing(ADD_ALIAS);
    finalize_indexing.add_dest_argument(name);
    finalize_indexing.add_source_argument(dest_for_index);

    command_list.push_back(finalize_indexing);
}

void ALILConverter::visit_composite(PNode node) {
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    PNode source = node->get_children()[1];

    current_defined_variables_within_comb.clear();

    if (!(source->get_token()->get_token_type() == COMB) && !(source->get_token()->get_token_type() == DISJOINT)) {
        raise_analysis_conversion_exception("Invalid operation to create a composite", source->get_token());
    }

    visit_comb_type(node);
    std::string name_lexeme = name->get_token()->get_lexeme();
    std::string source_lexeme = current_defined_variables_within_comb[0];


    // AnalysisCommand make_empty(MAKE_EMPTY_PARTICLE);
    // std::string empty_name = reserve_scoped_value_name();
    // make_empty.add_dest_argument(empty_name);
    // command_list.push_back(make_empty);

    // std::string source_name = handle_particle(source, empty_name);

    current_object_token = source->get_token()->get_token_type();
    current_object_particle_if_named = source_lexeme;


    // the CREATE_MASK command takes the new mask's name, the target object's name, and the source object's name

    AnalysisCommand create_mask(CREATE_MASK, node->get_children()[0]->get_token());
    
    std::stringstream mask_name;
    mask_name << "_MASK" << name_lexeme;

    // move us into the "scope" of this object
    current_scope_name = mask_name.str();
    current_limit = current_scope_name;

    create_mask.add_dest_argument(current_scope_name);
    create_mask.add_source_argument(source_lexeme);
    command_list.push_back(create_mask);


    visit_children_after_index(node, 1);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    for (auto it = current_defined_variables_within_comb.begin(); it != current_defined_variables_within_comb.end(); ++it) {
        AnalysisCommand apply_mask(APPLY_MASK);

        std::stringstream dest_arg;
        dest_arg << name_lexeme << "->" << *it;

        apply_mask.add_dest_argument(dest_arg.str());
        apply_mask.add_source_argument(current_limit);
        apply_mask.add_source_argument(*it);

        command_list.push_back(apply_mask);
    }

    current_scope_name = prev_name;
}

void ALILConverter::visit_object(PNode node) {


    
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    PNode source = node->get_children()[1];

    if (source->get_token()->get_token_type() == UNION) {

        visit_union_type(node);
        return;
    } else if (source->get_token()->get_token_type() == COMB || source->get_token()->get_token_type() == DISJOINT) {
        visit_comb_type(node);
        return;
    } else if (source->get_token()->get_token_type() == FIRST || source->get_token()->get_token_type() == SECOND) {
        visit_object_first_second(node);
        return;
    }

    std::string name_lexeme = name->get_token()->get_lexeme();
    std::string source_lexeme = source->get_token()->get_lexeme();


    AnalysisCommand make_empty(MAKE_EMPTY_PARTICLE);
    std::string empty_name = reserve_scoped_value_name();
    make_empty.add_dest_argument(empty_name);
    command_list.push_back(make_empty);

    std::string source_name = handle_particle(source, empty_name);

    current_object_token = source->get_token()->get_token_type();
    current_object_particle_if_named = source_lexeme;


    // the CREATE_MASK command takes the new mask's name, the target object's name, and the source object's name

    AnalysisCommand create_mask(CREATE_MASK, node->get_children()[0]->get_token());
    
    std::stringstream mask_name;
    mask_name << "_MASK" << name_lexeme;

    // move us into the "scope" of this object
    current_scope_name = mask_name.str();
    current_limit = current_scope_name;

    create_mask.add_dest_argument(current_scope_name);
    create_mask.add_source_argument(source_name);
    command_list.push_back(create_mask);


    visit_children_after_index(node, 1);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand apply_mask(APPLY_MASK);

    apply_mask.add_dest_argument(name_lexeme);
    apply_mask.add_source_argument(current_limit);
    apply_mask.add_source_argument(source_lexeme);

    command_list.push_back(apply_mask);
    current_scope_name = prev_name;
}

void ALILConverter::visit_region_select (PNode node) {
    std::string prev_name = current_region;
    current_region = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand cut_region(CUT_REGION);
    cut_region.add_dest_argument(current_region);
    cut_region.add_source_argument(prev_name);
    cut_region.add_source_argument(last_condition_name);

    command_list.push_back(cut_region);
}

void ALILConverter::visit_region_reject(PNode node) {

    std::string prev_name = current_region;
    current_region = reserve_scoped_limit_name();

    visit_children(node);

    AnalysisCommand invert_cut(EXPR_LOGICAL_NOT);
    std::string newly_inverted = reserve_scoped_value_name();
    invert_cut.add_dest_argument(newly_inverted);
    invert_cut.add_source_argument(last_condition_name);

    command_list.push_back(invert_cut);

    AnalysisCommand end_this_expression(END_EXPRESSION);
    std::string final_cut = reserve_scoped_value_name();
    end_this_expression.add_dest_argument(final_cut);
    end_this_expression.add_source_argument(newly_inverted);

    command_list.push_back(end_this_expression);

    AnalysisCommand cut_region(CUT_REGION);
    cut_region.add_dest_argument(current_region);
    cut_region.add_source_argument(prev_name);
    cut_region.add_source_argument(final_cut);

    command_list.push_back(cut_region);

}

void ALILConverter::visit_use(PNode node) {
    
    std::string prev_name = current_region;
    current_region = reserve_scoped_value_name();

    std::string name = node->get_children()[0]->get_token()->get_lexeme();
    
    AnalysisCommand merge_regions(MERGE_REGIONS, node->get_children()[0]->get_token());

    merge_regions.add_dest_argument(current_region);
    merge_regions.add_source_argument(name);
    merge_regions.add_source_argument(prev_name);

    command_list.push_back(merge_regions);

}


void ALILConverter::visit_region(PNode node) {
    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];

    std::string name_lexeme = name->get_token()->get_lexeme();

    AnalysisCommand create_region(CREATE_REGION, name->get_token());
    
    std::stringstream reg_name;
    reg_name << "_REG" << name_lexeme;

    // move us into the "scope" of this region
    current_scope_name = reg_name.str();
    current_region = reg_name.str();

    create_region.add_dest_argument(current_scope_name);
    command_list.push_back(create_region);

    visit_children_after_index(node, 0);

    // APPLY_MASK takes the target object's name, the mask's name, and the source object's name

    AnalysisCommand add_reg_name(ADD_ALIAS);

    add_reg_name.add_dest_argument(name_lexeme);
    add_reg_name.add_source_argument(current_region);

    command_list.push_back(add_reg_name);

    // add a cutflow statement and eventlist statement here - we will remove it later if it is unneeded
    AnalysisCommand cutflow(DO_CUTFLOW_ON_REGION);
    cutflow.add_source_argument(current_region);
    command_list.push_back(cutflow);

    AnalysisCommand eventlist(DO_EVENTLIST_ON_REGION);
    eventlist.add_source_argument(current_region);
    command_list.push_back(eventlist);


    current_scope_name = prev_name;

}


void ALILConverter::visit_definition(PNode node) {

    std::string prev_name = current_scope_name;

    PNode name = node->get_children()[0];
    std::string name_lexeme = name->get_token()->get_lexeme();

    std::stringstream def_name;
    def_name << "_DEF" << name_lexeme;

    current_scope_name = def_name.str();


    if (node->get_children()[1]->get_ast_type() == TERMINAL && node->get_children()[1]->get_token()->get_token_type() == EXTERNAL) {
        PNode func = node->get_children()[1]->get_children()[0];
        std::string func_name = func->get_token()->get_lexeme();

        AnalysisCommand add_extern_name(ADD_EXTERNAL, node->get_children()[0]->get_token());
        add_extern_name.add_dest_argument(name_lexeme);
        add_extern_name.add_source_argument(func_name);

        command_list.push_back(add_extern_name);
    } else if (node->get_children()[1]->get_ast_type() == TERMINAL && node->get_children()[1]->get_token()->get_token_type() == CORRECTIONLIB) {
        PNode filename_node = node->get_children()[1]->get_children()[0];
        std::string filename = filename_node->get_token()->get_lexeme();

        PNode keyname_node = node->get_children()[1]->get_children()[1];
        std::string keyname = keyname_node->get_token()->get_lexeme();

        AnalysisCommand add_correction(ADD_CORRECTIONLIB, name->get_token());
        add_correction.add_dest_argument(name_lexeme);
        add_correction.add_source_argument(filename);
        add_correction.add_source_argument(keyname);

        command_list.push_back(add_correction);
    } else {
        visit_children_after_index(node, 0);

        AnalysisCommand add_def_name(ADD_ALIAS, name->get_token());

        add_def_name.add_dest_argument(name_lexeme);
        add_def_name.add_source_argument(last_value_name);

        command_list.push_back(add_def_name);
    }
    
    // add this to the running list of all variables that have been defined within the current block (if we are not in a block, this is not relevant)
    current_defined_variables_within_comb.push_back(name_lexeme);

    current_scope_name = prev_name;
    
}

// we precompile the table to get it to have the right dimensionality for easy array operations
void ALILConverter::visit_table_def(PNode node) {
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

    AnalysisCommand create_table(CREATE_TABLE, name->get_token());
    create_table.add_dest_argument(prev_name);
    create_table.add_source_argument(nvars->get_token()->get_lexeme());

    command_list.push_back(create_table);

    int entry = 4;
    for (int row = 0; row < num_entries; row++) {
        AnalysisCommand append_to_table(APPEND_TO_TABLE);

        current_name = reserve_scoped_value_name();
        append_to_table.add_dest_argument(current_name);
        append_to_table.add_source_argument(prev_name);

        AnalysisCommand create_table_value(CREATE_TABLE_VALUE);
        std::string values_name = reserve_scoped_value_name();
        create_table_value.add_dest_argument(values_name);

        AnalysisCommand create_table_lower_bounds(CREATE_TABLE_LOWER_BOUNDS);
        std::string lower_bound_name = reserve_scoped_value_name();
        create_table_lower_bounds.add_dest_argument(lower_bound_name);

        AnalysisCommand create_table_upper_bounds(CREATE_TABLE_UPPER_BOUNDS);
        std::string upper_bound_name = reserve_scoped_value_name();
        create_table_upper_bounds.add_dest_argument(upper_bound_name);

        append_to_table.add_source_argument(values_name);
        append_to_table.add_source_argument(lower_bound_name);
        append_to_table.add_source_argument(upper_bound_name);

        for (int col = 0; col < num_columns_per_row; col++) {
            PToken cur_tok = node->get_children()[entry]->get_token();
            std::string cur_lexeme = cur_tok->get_lexeme();
            if (col <= (do_errors ? 2 : 0)) {
                create_table_value.add_source_argument(cur_lexeme);
            } else if (col % 2 == 0) {
                create_table_upper_bounds.add_source_argument(cur_lexeme);
            } else {
                create_table_lower_bounds.add_source_argument(cur_lexeme);
            }
            
            entry += 1;
        }

        command_list.push_back(create_table_value);
        command_list.push_back(create_table_lower_bounds);
        command_list.push_back(create_table_upper_bounds);
        command_list.push_back(append_to_table);

        prev_name = current_name;
    }

    AnalysisCommand finish_table(FINISH_TABLE, name->get_token());
    finish_table.add_dest_argument(name->get_token()->get_lexeme());
    finish_table.add_source_argument(prev_name);
    command_list.push_back(finish_table);

}

void ALILConverter::visit_criteria(PNode node) {
    
}

void ALILConverter::clean_command_list() {
    std::deque<AnalysisCommand> new_list;

    bool do_last_cutflow = false;
    bool do_every_cutflow = false;

    bool do_last_eventlist = false;
    bool do_every_eventlist = false;

    if (config.get_argument("cutflow") == "all") do_every_cutflow = true;
    if (config.get_argument("cutflow") == "last") do_last_cutflow = true;

    if (config.get_argument("eventlist") == "all") do_every_eventlist = true;
    if (config.get_argument("eventlist") == "last") do_last_eventlist = true;


    // backwards iteration pass
    for (auto it = command_list.rbegin(); it != command_list.rend(); ++it) {
        if (it->get_instruction() == DO_CUTFLOW_ON_REGION && !do_every_cutflow) {
            if (do_last_cutflow) do_last_cutflow = false;
            else continue;
        } else if (it->get_instruction() == DO_EVENTLIST_ON_REGION && !do_every_eventlist) {
            if (do_last_eventlist) do_last_eventlist = false;
            else continue;
        }

        new_list.push_front(*it);
    }
    command_list = std::vector<AnalysisCommand>(new_list.begin(), new_list.end());

}

void ALILConverter::visitation(PNode root) {
    visit(root);
    clean_command_list();
}

void ALILConverter::print_commands() {

    int top_size_of_dest = 0;
    int top_size_of_inst = 0;

    for (auto it = command_list.begin(); it != command_list.end(); ++it) {
        if (it->has_dest_argument()) {
            top_size_of_dest = std::max(top_size_of_dest, static_cast<int>(it->get_dest_argument().size()));
        }
        top_size_of_inst = std::max(top_size_of_inst, static_cast<int>(AnalysisCommand::instruction_to_text(it->get_instruction()).size()));
    }

    for (auto it = command_list.begin(); it != command_list.end(); ++it) {
        it->print_instruction(top_size_of_dest+4, top_size_of_inst+1);
    }
}

bool ALILConverter::clear_to_next() {
    if (iter_command >= command_list.size()) return false;
    return true;
}

AnalysisCommand ALILConverter::next_command() {
    return command_list[iter_command++];
}

ALILConverter::ALILConverter(Config &conf): highest_var_val(0), iter_command(0),  config(conf){}

ALILToFrameworkCompiler::ALILToFrameworkCompiler(ALILConverter *alil_in, Config &conf): alil(alil_in), config(conf) {}