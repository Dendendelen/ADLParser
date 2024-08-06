#include "timber_converter.h"
#include "lexer.h"
#include "node.h"
#include "tokens.h"
#include "exceptions.h"
#include <sstream>
#include <iostream>
#include <string>

void TimberConverter::append_4vector_label(AnalysisCommand command, std::string suffix) {

    std::string output = command.get_argument(0);
    std::string input = var_mappings[command.get_argument(1)];

    std::stringstream delimit;
    std::stringstream command_text;

    delimit << input;
    std::vector<std::string> delimited_by_space;
    std::string buffer;
    while (std::getline(delimit, buffer, ' ')) {
        delimited_by_space.push_back(buffer);
    }

    for (auto it = delimited_by_space.begin(); it != delimited_by_space.end(); ++it) {
        command_text << *it;
        if (it->back() != '+' && it->back() != '-') command_text << suffix;
    }

    var_mappings[output] = command_text.str();

}

std::string TimberConverter::binary_command(AnalysisCommand command, std::string op) {
    std::stringstream text;
    text << var_mappings[command.get_argument(1)] << op << var_mappings[command.get_argument(2)];
    return text.str();
}

std::string TimberConverter::command_convert(AnalysisCommand command) {

    AnalysisLevelInstruction inst = command.get_instruction();
    std::stringstream command_text;

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
        {
            if (var_mappings.count(command.get_argument(1)) == 0) var_mappings[command.get_argument(1)] = command.get_argument(1);
            var_mappings[command.get_argument(0)] = command.get_argument(1);
            return "";
        }
        case ADD_OBJECT:
            return "ADD_OBJECT";
        case CREATE_MASK:
        {
            command_text << command.get_argument(0) << " = VarGroup('" << command.get_argument(0) << "')\n"; 
            command_text << command.get_argument(0) << ".Add('" << command.get_argument(0) << "', 'create_mask(" << command.get_argument(1) << ")')";            
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        }
        case LIMIT_MASK:
        {   
            command_text << var_mappings[command.get_argument(1)] << ".Add('" << command.get_argument(0) << "', 'limit_mask(" << command.get_argument(1) << ", " << var_mappings[command.get_argument(2)] << ")')";
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        }
        case APPLY_MASK:
        {
            command_text << var_mappings[command.get_argument(1)] << ".Add('" << command.get_argument(0) << "', 'apply_mask(" << command.get_argument(1) << ", " << command.get_argument(2) << ")')"; 
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        }
        case BEGIN_EXPRESSION:
            return "";
        case END_EXPRESSION:
        {
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return "";
        }
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
            var_mappings[command.get_argument(0)] = binary_command(command, "<");
            return "";
        case EXPR_LE:
            return "EXPR_LE";
        case EXPR_GT:
            var_mappings[command.get_argument(0)] = binary_command(command, ">");
            return "";
        case EXPR_GE:
            return "EXPR_GE";
        case EXPR_EQ:
            var_mappings[command.get_argument(0)] = binary_command(command, "==");
            return "";
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
            append_4vector_label(command, "_pt");
            return "";
        case FUNC_ETA:
            append_4vector_label(command, "_eta");
            return "";
        case FUNC_PHI:
            append_4vector_label(command, "_phi");
            return "";
        case FUNC_M:
            append_4vector_label(command, "_mass");
            return "";
        case FUNC_E:
            return "FUNC_E"; 
        case MAKE_EMPTY_PARTICLE:
        {
            var_mappings[command.get_argument(0)] = "";
            return "";
        }
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
        {
            command_text << var_mappings[command.get_argument(1)] << (var_mappings[command.get_argument(1)] != "" ? " + Jet" : "Jet");
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        }
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
            command_text << "abs(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
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
    while (alil->clear_to_next()) {
        std::string out = command_convert(alil->next_command());
        if (out == "") continue;
        std::cout << out << std::endl;
    }
}

TimberConverter::TimberConverter(ALIConverter *alil_in): alil(alil_in) {}