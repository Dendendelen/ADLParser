#include "timber_converter.h"
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

void TimberConverter::print_timber() {
    
}