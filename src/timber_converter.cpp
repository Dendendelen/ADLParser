#include "timber_converter.hpp"
#include "ali_converter.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "tokens.hpp"
#include "exceptions.hpp"
#include <ostream>
#include <sstream>
#include <iostream>
#include <string>


std::string TimberConverter::add_all_relevant_tags_for_object(AnalysisCommand command) {
    std::stringstream command_text;

    std::string add_target = var_mappings[command.get_argument(1)];
    std::string dest_vec = command.get_argument(0);
    std::string mask = command.get_argument(1);
    std::string src_vec = command.get_argument(2);

    command_text << "a.Apply(" <<add_target << ")\n";
    command_text << "a.SubCollection('" << dest_vec << "', '" << src_vec << "', '" << mask << "', useTake=False)\n";
    return command_text.str();

}

std::string TimberConverter::add_all_relevant_tags_for_union_empty(AnalysisCommand command) {
    std::stringstream command_text;

    std::string dest_vec = command.get_argument(0);

    empty_union_names.insert(dest_vec);
    return command_text.str();  
}


std::string TimberConverter::add_all_relevant_tags_for_union_merge(AnalysisCommand command, std::string adding_name) {
    std::stringstream command_text;

    std::string dest_vec = command.get_argument(0);
    std::string old_union = command.get_argument(1);

    if (empty_union_names.find(old_union) != empty_union_names.end()) {
        empty_union_names.erase(empty_union_names.find(old_union));
        command_text << "a.MergeCollections('" << dest_vec << "', ['"  << adding_name << "'])\n";
    } else {
        command_text << "a.MergeCollections('" << dest_vec << "', ['" << old_union << "', '" << adding_name << "'])\n";
    }

    return command_text.str();  
}

std::string TimberConverter::existing_definitions_string() {
    std::stringstream defs;
    defs << "[";
    
    for (auto it = existing_definitions.begin(); it != existing_definitions.end(); ++it) {
        defs << *it << ", ";
    }

    defs << "]";
    return defs.str();
}

/**
    Adds an index tag to a particle, as NanoAOD handles 4-vector indices in this way
*/
std::string TimberConverter::index_particle(AnalysisCommand command, bool is_named, std::string part_text) {
    if (command.get_num_arguments() - is_named >= 4) {
        std::stringstream idx_text;
        idx_text << "index_get(" << part_text << " ," << command.get_argument(2+is_named) << "," << command.get_argument(3+is_named) << ")";\
        return idx_text.str();

    } else if (command.get_num_arguments() - is_named >= 3) {
        std::stringstream idx_text;
        idx_text << "index_get(" << part_text << " ," << command.get_argument(2+is_named) << ")";
        return idx_text.str();
    } else {
        return part_text;
    }
}

void TimberConverter::add_particle(AnalysisCommand command, std::string name) {
    bool is_named = false;
    if (command.get_instruction() == ADD_PART_NAMED) is_named = true;
    std::stringstream command_text;

    std::string indexed_if_needed = index_particle(command, is_named, name);

    std::string source = var_mappings[command.get_argument(1+is_named)];

    if (is_lorentz_vector.count(source) != 0) {
        //TODO: finish the logic here
        // we want to keep track of what is just NanoAOD and what is a 4-vector object (like literally which numbered values in ALIL correspond to what),
        // and then from there handle them differently for when we are running functions on them
        
    }

    if (source != "") {
        command_text << "hardware::TLVector(" << generate_4vector_label(source, "pt");
        command_text << ", " << generate_4vector_label(source, "eta");
        command_text << ", " << generate_4vector_label(source, "phi");
        command_text << ", " << generate_4vector_label(source, "m");
        command_text << ")";
        command_text << " + ";
        command_text << "hardware::TLVector(" << generate_4vector_label(indexed_if_needed, "pt");
        command_text << ", " << generate_4vector_label(indexed_if_needed, "eta");
        command_text << ", " << generate_4vector_label(indexed_if_needed, "phi");
        command_text << ", " << generate_4vector_label(indexed_if_needed, "m");
        command_text << ") ";

    } else {
        command_text << indexed_if_needed;
    }

    command_text << '\x1d';
    
    var_mappings[command.get_argument(0)] = command_text.str();
}  

void TimberConverter::sub_particle(AnalysisCommand command, std::string name) {
    bool is_named = false;
    if (command.get_instruction() == ADD_PART_NAMED) is_named = true;
    std::stringstream command_text;

    std::string indexed_if_needed = index_particle(command, is_named, name);
    command_text << var_mappings[command.get_argument(1+is_named)] << " - " << indexed_if_needed;
}

std::string TimberConverter::generate_4vector_label(std::string input, std::string suffix) {
    std::stringstream delimit;
    std::stringstream command_text;

    delimit << input;
    std::vector<std::string> delimited_by_space;
    std::string buffer;
    while (std::getline(delimit, buffer, '\x1d')) {
        delimited_by_space.push_back(buffer);
    }

    for (auto it = delimited_by_space.begin(); it != delimited_by_space.end(); ++it) {
        command_text << *it;
        if (it->back() != '+' && it->back() != '-' && it->back() != ')') command_text << suffix;
    }
    return command_text.str();

}

void TimberConverter::append_4vector_label(AnalysisCommand command, std::string suffix) {

    std::string output = command.get_argument(0);
    std::string input = var_mappings[command.get_argument(1)];
    var_mappings[output] = generate_4vector_label(input, suffix);

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

        case HIST_1D:
            command_text << "\n_histogram" << command.get_argument(0) << " = []";
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << command.get_argument(0) << "')";
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << command.get_argument(1) << "')";
            for (int i = 2; i < 5; i++) {
                command_text << "\n_histogram" << command.get_argument(0) << ".append(" << var_mappings[command.get_argument(i)] << ")";
            }
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << var_mappings[command.get_argument(5)] << "')";
            return command_text.str();
        case HIST_2D:
            command_text << "\n_histogram" << command.get_argument(0) << " = []";
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << command.get_argument(0) << "')";
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << command.get_argument(1) << "')";
            for (int i = 2; i < 5; i++) {
                command_text << "\n_histogram" << command.get_argument(0) << ".append(" << var_mappings[command.get_argument(i)] << ")";
            }
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << var_mappings[command.get_argument(5)] << "')";
            for (int i = 6; i < 9; i++) {
                command_text << "\n_histogram" << command.get_argument(0) << ".append(" << var_mappings[command.get_argument(i)] << ")";
            }
            command_text << "\n_histogram" << command.get_argument(0) << ".append('" << var_mappings[command.get_argument(9)] << "')";

            return command_text.str();      
        case USE_HIST:
            command_text << "\n_old_node = a.GetActiveNode()";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.Apply(" << var_mappings[command.get_argument(1)] << "[0])";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.AddCorrections(" << var_mappings[command.get_argument(1)] << "[1])";
            command_text << "\nuse_histo(_histogram" << command.get_argument(0) << ", _histogram_node_" << command.get_argument(1) << ")";
            command_text << "\na.SetActiveNode(_old_node)";
            return command_text.str();
        case CREATE_HIST_LIST:
            command_text << "\n_histogram_list" << command.get_argument(0) << " = []";
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        case ADD_HIST_TO_LIST:
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            command_text << "\n_histogram_list" << var_mappings[command.get_argument(1)] << ".append(_histogram" << command.get_argument(2) << ")";
            return command_text.str();
        case USE_HIST_LIST:
            command_text << "\n_old_node = a.GetActiveNode()";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.Apply(" << var_mappings[command.get_argument(1)] << ")";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.AddCorrections(" << var_mappings[command.get_argument(1)] << "[1])";
            command_text << "\nuse_histo_list(_histogram_list" << var_mappings[command.get_argument(0)] << ", _histogram_node_" << command.get_argument(1) << ")";
            command_text << "\na.SetActiveNode(_old_node)";
            return command_text.str();

        case CREATE_REGION:
            command_text << command.get_argument(0) << " = [CutGroup('" << command.get_argument(0) << "'), []]\n";
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        case MERGE_REGIONS:
            command_text << var_mappings[command.get_argument(2)] << "[0] = " << var_mappings[command.get_argument(2)] << "[0] + " << var_mappings[command.get_argument(1)] << "[0]\n";
            command_text << var_mappings[command.get_argument(2)] << "[1] = " << var_mappings[command.get_argument(2)] << "[1] + " << var_mappings[command.get_argument(1)] << "[1]\n";
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(2)];
            return command_text.str();
        case CUT_REGION:
            command_text << var_mappings[command.get_argument(1)] << "[0].Add('" << command.get_argument(0) << "', '" << var_mappings[command.get_argument(2)] << "')"; 
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case RUN_REGION:
            return "RUN_REGION";
        case ADD_ALIAS:
        {
            if (var_mappings.count(command.get_argument(1)) == 0) var_mappings[command.get_argument(1)] = command.get_argument(1);
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return "";
        }
        case ADD_EXTERNAL:
        {
            std::string fn_name_with_quotes = command.get_argument(1);
            std::string fn_name_wo_quotes = fn_name_with_quotes.substr(1,fn_name_with_quotes.size()-2);
            var_mappings[command.get_argument(0)] = fn_name_wo_quotes;
            return "";
        }
        //TODO: check that this is consistant with the overall syntax
        case SORT_ASCEND:
            command_text << "VecOps::Sort(" << command.get_argument(1) << ")";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case SORT_DESCEND:
            command_text << "VecOps::Reverse(VecOps::Sort(" << command.get_argument(1) << "))";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case ADD_OBJECT:
            return "ADD_OBJECT";
        case CREATE_MASK:
        {
            command_text << "\n" << command.get_argument(0) << " = VarGroup('" << command.get_argument(0) << "')\n"; 
            command_text << command.get_argument(0) << ".Add('" << command.get_argument(0) << "', 'create_mask(" << command.get_argument(1) << "_pt)')";            
            var_mappings[command.get_argument(0)] = command.get_argument(0);

            existing_definitions.push_back(command.get_argument(0));
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
            command_text << add_all_relevant_tags_for_object(command);

            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        }
        case CREATE_TABLE:
        {
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            command_text << command.get_argument(0) << "_nvars = " << command.get_argument(0) << "\n";
            command_text << command.get_argument(0) << "_lower_bounds = []\n";
            command_text << command.get_argument(0) << "_upper_bounds = []\n";
            command_text << command.get_argument(0) << "_values = []\n";
            return command_text.str();
        }
        case CREATE_TABLE_LOWER_BOUNDS:
        case CREATE_TABLE_UPPER_BOUNDS:
            if (command.get_num_arguments() < 3) {
                var_mappings[command.get_argument(0)] = command.get_argument(1); 
                return "";
            }
            command_text << "[" << command.get_argument(1);
            for (int i = 2; i < command.get_num_arguments(); i++) {
                command_text << "," << command.get_argument(i); 
            }
            command_text << "]";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case CREATE_TABLE_VALUE:

            if (command.get_num_arguments() < 3) {
                var_mappings[command.get_argument(0)] = command.get_argument(1);
            } else {
                command_text << "[" << command.get_argument(1) << "," << command.get_argument(2) << "," << command.get_argument(3) << "]";
                var_mappings[command.get_argument(0)] = command_text.str();
            }
            return "";
        case APPEND_TO_TABLE:
        {
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            command_text << var_mappings[command.get_argument(1)] << "_values.append(" << var_mappings[command.get_argument(2)] << "\n";
            command_text << var_mappings[command.get_argument(1)] << "_lower_bounds.append(" << var_mappings[command.get_argument(3)] << "\n";
            command_text << var_mappings[command.get_argument(1)] << "_upper_bounds.append(" << var_mappings[command.get_argument(4)] << "\n";
            return command_text.str();
        }
        case FINISH_TABLE:
        {
            std::string old_name = var_mappings[command.get_argument(1)];
            command_text << old_name << "_values_array = ROOT.VecOps.AsRVec(np.array(" << old_name << "_values, dtype=np.float32))\n";
            command_text << old_name << "_lower_bounds_array = ROOT.VecOps.AsRVec(np.array(" << old_name << "_lower_bounds, dtype=np.float32))\n";
            command_text << old_name << "_upper_bounds_array = ROOT.VecOps.AsRVec(np.array(" << old_name << "_upper_bounds, dtype=np.float32))\n"; 

            command_text << "ROOT.gInterpreter.Declare('" << command.get_argument(0);
            command_text << " = create_table_function(' + str(" << old_name << "_nvars) + '," << old_name << "_lower_bound_array," << old_name << "_upper_bound_array," << old_name << "_values_array);')";
        }
        case WEIGHT_APPLY:
            command_text << var_mappings[command.get_argument(1)] << "[1].append(Correction('" << command.get_argument(2) << "', '', '" << var_mappings[command.get_argument(2)] << "')"; 
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
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
            command_text << "raise_power(" << var_mappings[command.get_argument(1)] << "," << var_mappings[command.get_argument(2)] <<  ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case EXPR_MULTIPLY:
            var_mappings[command.get_argument(0)] = binary_command(command, "*");
            return "";
        case EXPR_DIVIDE:
            var_mappings[command.get_argument(0)] = binary_command(command, "/");
            return "";
        case EXPR_ADD:
            var_mappings[command.get_argument(0)] = binary_command(command, "+");
            return "";
        case EXPR_SUBTRACT:
            var_mappings[command.get_argument(0)] = binary_command(command, "-");
            return "";
        case EXPR_LT:
            var_mappings[command.get_argument(0)] = binary_command(command, "<");
            return "";
        case EXPR_LE:
            var_mappings[command.get_argument(0)] = binary_command(command, "<=");
            return "";
        case EXPR_GT:
            var_mappings[command.get_argument(0)] = binary_command(command, ">");
            return "";
        case EXPR_GE:
            var_mappings[command.get_argument(0)] = binary_command(command, ">=");
            return "";
        case EXPR_EQ:
            var_mappings[command.get_argument(0)] = binary_command(command, "==");
            return "";
        case EXPR_NE:
            var_mappings[command.get_argument(0)] = binary_command(command, "!=");
            return "";
        case EXPR_AMPERSAND:
            var_mappings[command.get_argument(0)] = binary_command(command, "&");
            return "";
        case EXPR_PIPE:
            var_mappings[command.get_argument(0)] = binary_command(command, "|");
            return "";
        case EXPR_AND:
            var_mappings[command.get_argument(0)] = binary_command(command, "&&");
            return "";
        case EXPR_OR:
            var_mappings[command.get_argument(0)] = binary_command(command, "||");
            return "";
        case EXPR_WITHIN:
            command_text << "((" << var_mappings[command.get_argument(1)] << ">=" << var_mappings[command.get_argument(2)] << ")&&(" << var_mappings[command.get_argument(1)] << "<=" << var_mappings[command.get_argument(3)] << "))";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case EXPR_OUTSIDE:
            command_text << "((" << var_mappings[command.get_argument(1)] << "<=" << var_mappings[command.get_argument(2)] << ")||(" << var_mappings[command.get_argument(1)] << ">=" << var_mappings[command.get_argument(3)] << "))";
            var_mappings[command.get_argument(0)] = command_text.str();           
            return "";
        case EXPR_NEGATE:
            command_text << "-(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case EXPR_LOGICAL_NOT:
            command_text << "!(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";

        case FUNC_BTAG:
        {
            append_4vector_label(command, "_btagDeepFlavB");
            command_text << "(" << var_mappings[command.get_argument(0)] << " > 0.3040)";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        }
        case FUNC_PT:
            append_4vector_label(command, "_pt");
            return "";
        case FUNC_ETA:
            append_4vector_label(command, "_eta");
            return "";
        case FUNC_PHI:
            append_4vector_label(command, "_phi");
            return "";
        case FUNC_MASS:
            append_4vector_label(command, "_mass");
            return "";
        case FUNC_ENERGY:
            raise_non_implemented_conversion_exception("FUNC_ENERGY");
            return "FUNC_E"; 
        case FUNC_CHARGE:
            append_4vector_label(command, "_charge");
            return "";
        case FUNC_MSOFTDROP:
            append_4vector_label(command, "_msoftdrop");
            return "";
        case FUNC_IS_TIGHT:
            append_4vector_label(command, "_tightId");
            return "";
        case FUNC_IS_MEDIUM:
            append_4vector_label(command, "_mediumId");
            return "";
        case FUNC_IS_LOOSE:
            append_4vector_label(command, "_looseId");
            return "";

        case MAKE_EMPTY_PARTICLE:
        {
            var_mappings[command.get_argument(0)] = "";
            return "";
        }
        case ADD_PART_ELECTRON:
            add_particle(command, "Electron");
            return "";
        case ADD_PART_MUON:
            add_particle(command, "Muon");
            return "";
        case ADD_PART_TAU:
            add_particle(command, "Tau");
            return "";
        case ADD_PART_TRACK:
            add_particle(command, "IsoTrack");
            return "";
        case ADD_PART_LEPTON:
            add_particle(command, "Lepton"); //TODO: change?
            return "";
        case ADD_PART_PHOTON:
            add_particle(command, "Photon"); 
            return "";
        case ADD_PART_BJET:
            add_particle(command, "BJet"); //TODO: change?
            return "";
        case ADD_PART_QGJET:
            add_particle(command, "QGJet"); //TODO: change?
            return "";
        case ADD_PART_NUMET:
            add_particle(command, "MET");
            return "";
        case ADD_PART_METLV:
            sub_particle(command, "METLV");
            return "";
        case ADD_PART_GEN:
            add_particle(command, "GenPart");
            return "";
        case ADD_PART_JET:
            add_particle(command, "Jet");
            return "";
        case ADD_PART_FJET:
            add_particle(command, "FatJet");
            return "";
        case ADD_PART_NAMED:
            if (var_mappings.count(command.get_argument(1)) == 0) var_mappings[command.get_argument(1)] = command.get_argument(1);
            add_particle(command, var_mappings[command.get_argument(1)]);
            return "";
        case SUB_PART_ELECTRON:
            sub_particle(command, "Electron");
            return "";
        case SUB_PART_MUON:
            sub_particle(command, "Muon");
            return "";
        case SUB_PART_TAU:
            sub_particle(command, "Tau");
            return "";
        case SUB_PART_TRACK:
            sub_particle(command, "IsoTrack");
            return "";
        case SUB_PART_LEPTON:
            sub_particle(command, "Lepton");
            return "";
        case SUB_PART_PHOTON:
            sub_particle(command, "Photon");
            return "";
        case SUB_PART_BJET:
            sub_particle(command, "BJet");
            return "";
        case SUB_PART_QGJET:
            sub_particle(command, "QGJet");
            return "";
        case SUB_PART_NUMET:
            sub_particle(command, "MET");
            return "";
        case SUB_PART_METLV:
            sub_particle(command, "METLV");
            return "";
        case SUB_PART_GEN:
            sub_particle(command, "GenPart");
            return "";
        case SUB_PART_JET:
            sub_particle(command, "Jet");
            return "";
        case SUB_PART_FJET:
            sub_particle(command, "FatJet");
            return "";
        case SUB_PART_NAMED:
            sub_particle(command, command.get_argument(1));
            return "";
        case FUNC_HSTEP:
            raise_non_implemented_conversion_exception("FUNC_HSTEP");
            return "FUNC_HSTEP";
        case FUNC_DELTA:
            raise_non_implemented_conversion_exception("FUNC_DELTA");
            return "FUNC_DELTA";
        case FUNC_ANYOF:
            raise_non_implemented_conversion_exception("FUNC_ANYOF");
            return "FUNC_ANYOF";
        case FUNC_ALLOF:
            raise_non_implemented_conversion_exception("FUNC_ALLOF");
            return "FUNC_ALLOF";
        case FUNC_SQRT:
            command_text << "sqrt(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_ABS:
            command_text << "abs(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_COS:
            command_text << "cos(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_SIN:
            command_text << "sin(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_TAN:
            command_text << "tan(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_SINH:
            command_text << "sinh(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_COSH:
            command_text << "cosh(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_TANH:
            command_text << "tanh(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_EXP:
            command_text << "exp(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_LOG:
            command_text << "log(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_AVE:
            raise_non_implemented_conversion_exception("FUNC_AVE");
            return "FUNC_AVE";
        case FUNC_SUM:
            raise_non_implemented_conversion_exception("FUNC_SUM");
            return "FUNC_SUM";

        case FUNC_SORT_ASCEND:
            command_text << "VecOps::Sort(" << command.get_argument(1) << ")";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case FUNC_SORT_DESCEND:
            command_text << "VecOps::Reverse(VecOps::Sort(" << command.get_argument(1) << "))";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";

        case FUNC_NAMED:
            raise_non_implemented_conversion_exception("FUNC_NAMED");
            return "FUNC_NAMED";
        case MAKE_EMPTY_UNION:
            // command_text << "\n" << command.get_argument(0) << " = VarGroup('" << command.get_argument(0) << "')\n"; 
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            command_text << add_all_relevant_tags_for_union_empty(command);

            existing_definitions.push_back(command.get_argument(0));
            return command_text.str();        
        case ADD_NAMED_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, var_mappings[command.get_argument(2)]);
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();        
        case ADD_ELECTRON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Electron");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_MUON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Muon");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_TAU_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Tau");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_TRACK_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "IsoTrack");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_LEPTON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Lepton");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_PHOTON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Photon");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_BJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "BJet");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_QGJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "QGJet");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_NUMET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "MET");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_METLV_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "METLV");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_GEN_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "GenPart");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_JET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Jet");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();
        case ADD_FJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "FatJet");
            var_mappings[command.get_argument(0)] = var_mappings[command.get_argument(1)];
            return command_text.str();

        case FUNC_FLAVOR:
            append_4vector_label(command, "_partonFlavor");
            return "";
        case FUNC_JET_ID:
            append_4vector_label(command, "_jetId");
            return "";
        case FUNC_CONSTITUENTS:
            raise_non_implemented_conversion_exception("FUNC_CONSTITUENTS");
            return "FUNC_CONSTITUENTS";
        case FUNC_PDG_ID:
            append_4vector_label(command, "_pdgId");
            return "";
        case FUNC_IDX:
            raise_non_implemented_conversion_exception("FUNC_IDX");
            return "FUNC_IDX";
        case FUNC_TAUTAG:
            raise_non_implemented_conversion_exception("FUNC_TAUTAG");
            return "FUNC_TAUTAG";
        case FUNC_CTAG:
            raise_non_implemented_conversion_exception("FUNC_CTAG");
            return "FUNC_CTAG";
        case FUNC_DXY:
            append_4vector_label(command, "_dxy");
            return "";
        case FUNC_EDXY:
            raise_non_implemented_conversion_exception("FUNC_EDXY");
            return "FUNC_EDXY";
        case FUNC_EDZ:
            raise_non_implemented_conversion_exception("FUNC_EDZ");
            return "FUNC_EDZ";
        case FUNC_DZ:
            raise_non_implemented_conversion_exception("FUNC_DZ");
            return "FUNC_DZ";
        case FUNC_ABS_ETA:
            raise_non_implemented_conversion_exception("FUNC_ABS_ETA");
            return "FUNC_ABS_ETA";
        case FUNC_THETA:
            raise_non_implemented_conversion_exception("FUNC_THETA");
            return "FUNC_THETA";
        case FUNC_PT_CONE:
            raise_non_implemented_conversion_exception("FUNC_PT_CONE");
            return "FUNC_PT_CONE";
        case FUNC_ET_CONE:
            raise_non_implemented_conversion_exception("FUNC_ET_CONE");
            return "FUNC_ET_CONE";
        case FUNC_ABS_ISO:
            raise_non_implemented_conversion_exception("FUNC_ABS_ISO");
            return "FUNC_ABS_ISO";
        case FUNC_MINI_ISO:
            raise_non_implemented_conversion_exception("FUNC_MINI_ISO");
            return "FUNC_MINI_ISO";
        case FUNC_PZ:
            raise_non_implemented_conversion_exception("FUNC_PZ");
            return "FUNC_PZ";
        case FUNC_NBF:
            raise_non_implemented_conversion_exception("FUNC_NBF");
            return "FUNC_NBF";
        case FUNC_DR:
            command_text << "hardware::DeltaR(" << var_mappings[command.get_argument(1)] << ", " << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DPHI:
            command_text << "hardware::DeltaPhi(" << var_mappings[command.get_argument(1)] << ", " << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DETA:
            command_text << "hardware::DeltaEta(" << var_mappings[command.get_argument(1)] << ", " << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_SIZE: //TODO: check this does not conflict with a valid use case
            command_text << "size(" << var_mappings[command.get_argument(1)] << "_pt)";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case CREATE_BIN:
        case FUNC_VER_TR:
        case FUNC_VER_Z:
        case FUNC_VER_Y:
        case FUNC_VER_X:
        case FUNC_VER_T:
        case FUNC_GEN_PART_IDX:
        case FUNC_RAPIDITY:
        case FUNC_FMT2:
        case FUNC_TAUTAU:
        case FUNC_HT:
        case FUNC_SPHERICITY:
        case FUNC_APLANARITY:
            raise_non_implemented_conversion_exception("Function not implemented");
            return "";
        default:
            std::stringstream error;
            error << AnalysisCommand::instruction_to_text(inst);
            raise_non_implemented_conversion_exception(error.str());
            return "";
        }
}

void TimberConverter::print_timber() {

    std::string preliminary = 
        "from TIMBER.Analyzer import *\nfrom TIMBER.Tools.Common import *\nimport ROOT\nimport sys, os\nfrom adl_helpers import combine_without_duplicates, use_histo, use_histo_list\nCompileCpp('adl_helpers.cc')\na = analyzer('filename.root')\nout = ROOT.TFile.Open('adl_out.root','UPDATE')";
    std::cout << preliminary << std::endl;

    std::string definitions =
        "\na.Define('MET', 'MET_pt')\n";

    std::cout << definitions << std::endl;

    while (alil->clear_to_next()) {
        std::string out = command_convert(alil->next_command());
        if (out == "") continue;
        std::cout << out << std::endl;
    }

    std::string postscriptum = 
        "\nout.Close()\n";
    std::cout << postscriptum << std::endl;
}

TimberConverter::TimberConverter(ALIConverter *alil_in): alil(alil_in) {}