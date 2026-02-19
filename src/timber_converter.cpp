#include "timber_converter.hpp"
#include "ali_converter.hpp"
#include "exceptions.hpp"
#include <filesystem>
#include <ostream>
#include <regex>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>


std::string TimberConverter::add_all_relevant_tags_for_object(AnalysisCommand command) {
    std::stringstream command_text;

    std::string add_target = get_mapping_if_exists(command.get_argument(1));
    std::string dest_vec = command.get_argument(0);
    std::string mask = command.get_argument(1);
    std::string src_vec = command.get_argument(2);

    if (already_applied_globally.count(add_target) == 0) {
        already_applied_globally.emplace(add_target);
        command_text << "a.Apply(" <<add_target << ")\n";
    }
    command_text << "a.SubCollection('" << dest_vec << "', '" << generate_4vector_label(get_mapping_if_exists(src_vec), "") << "', '" << mask << "', useTake=False, skip=[\"idx\"])\n";
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

std::string TimberConverter::add_structure_for_comb_empty(AnalysisCommand command) {

    std::string dest_vec = command.get_argument(0);

    comb_map[dest_vec] = std::vector<std::string>();

    return "";
}

std::string TimberConverter::add_structure_for_comb_merge(AnalysisCommand command, std::string adding_name) {

    std::string dest_vec = command.get_argument(0);
    std::string old_comb = command.get_argument(1);

    comb_map[get_mapping_if_exists(old_comb)].push_back(generate_4vector_label(get_mapping_if_exists(adding_name), "_pt"));
    var_mappings[dest_vec] = get_mapping_if_exists(old_comb);

    return "";
}

std::string TimberConverter::add_comb_argument(std::string new_name, std::string name_of_comb, std::string val, bool is_disjoint) {
    std::stringstream command_text;

    if (comb_already_made.count(name_of_comb) == 0) {
        comb_already_made.insert(name_of_comb);

        command_text << "\na.Define('vORIG";
        if (is_disjoint) command_text << "DISJOINT";
        else command_text << "COMB";
        
        
        command_text << name_of_comb << "', 'General" ;
        if (is_disjoint) command_text << "Disjoint";
        else command_text << "Comb";
        
        command_text << "({";

        bool is_first = true;
        for (std::string comb_entry : comb_map[get_mapping_if_exists(name_of_comb)]) {
            if (!is_first) {
                command_text << ",";
            } else {
                is_first = false;
            }
            command_text << comb_entry;
        }

        command_text << "})')";
    }

    int index = std::stoi(val);
    std::string relevant_comb_entry_variable = comb_map[get_mapping_if_exists(name_of_comb)][index];
    //TODO: check that this works always, or replace it
    relevant_comb_entry_variable.erase(relevant_comb_entry_variable.length() - 3);

    command_text << "\na.SubCollection('" << new_name << "', '" << relevant_comb_entry_variable << "', 'vORIG";
    
    if (is_disjoint) command_text << "DISJOINT";
    else command_text << "COMB";
    command_text << name_of_comb << "[" << val << "]', useTake=True)\n";

    return command_text.str();

}


std::string TimberConverter::get_mapping_if_exists(std::string str) {
    if (var_mappings.count(str) == 0) {
        var_mappings[str] = str;
    } 
    return var_mappings[str];
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
    std::stringstream idx_text;
    
    std::regex e ("\x1d"); 
    part_text = std::regex_replace(part_text, e, "");

    if (command.get_num_arguments() - is_named >= 4) {
        idx_text << "index_get(" << part_text << '\x1d' << " ," << command.get_argument(2+is_named) << "," << command.get_argument(3+is_named) << ")";\
    } else if (command.get_num_arguments() - is_named >= 3) {
        // idx_text << "index_get(" << part_text << " ," << command.get_argument(2+is_named) << ")";
        idx_text << part_text << '\x1d' << "[" << command.get_argument(2+is_named) << "]";   
    } else {
        idx_text << part_text << '\x1d';
    }
    return idx_text.str();
}

std::string TimberConverter::lorentzify(std::string name) {
    if (is_lorentz_vector.count(name) != 0) {
        return get_mapping_if_exists(name);
    }
    // this is not already a lorentz vector - we would like it to become so
    std::stringstream command_text;
    command_text << "(";
    command_text << "TLV(" << generate_4vector_label(name, "_pt");
    command_text << ", " << generate_4vector_label(name, "_eta");
    command_text << ", " << generate_4vector_label(name, "_phi");
    command_text << ", " << generate_4vector_label(name, "_mass");
    command_text << "))";
    return command_text.str();
}

void TimberConverter::add_particle(AnalysisCommand command, std::string name) {
    bool is_named = false;
    if (command.get_instruction() == ADD_PART_NAMED) is_named = true;
    std::stringstream command_text;

    std::string indexed_if_needed = index_particle(command, is_named, name);

    std::string source = get_mapping_if_exists(command.get_argument(1+is_named));

    if (is_lorentz_vector.count(source) != 0) {
        //TODO: finish the logic here
        // we want to keep track of what is just NanoAOD and what is a 4-vector object (like literally which numbered values in ALIL correspond to what),
        // and then from there handle them differently for when we are running functions on them
        

        // implies that 1) the source is not empty and 2) that it is in a 4-vector state. We thus add to it and our result will also be a 4-vector

        command_text << "(";

        std::regex e ("\x1d"); 
        command_text << std::regex_replace (source,e,"");
        command_text << " + ";
        command_text << lorentzify(indexed_if_needed);
        command_text << ")";
        command_text << '\x1d';
        is_lorentz_vector.insert(command_text.str());


    } else if (source != "") {
        // the source is not a 4-vector but is also non-empty. This implies that we need to create a 4-vector out of it, and proceed to add it to the new particle
        command_text << "(";
        command_text << lorentzify(source);
        command_text << " + ";
        command_text << lorentzify(indexed_if_needed);
        command_text << ")";
        command_text << '\x1d';
        is_lorentz_vector.insert(command_text.str());
    } else {
        // the source is empty, and so we simply add this as a particle without any special actions
        command_text << get_mapping_if_exists(indexed_if_needed);
    }
    
    var_mappings[command.get_argument(0)] = command_text.str();
}  

void TimberConverter::sub_particle(AnalysisCommand command, std::string name) {
    bool is_named = false;
    if (command.get_instruction() == ADD_PART_NAMED) is_named = true;
    std::stringstream command_text;

    std::string indexed_if_needed = index_particle(command, is_named, name);
    command_text << var_mappings[command.get_argument(1+is_named)] << " - " << indexed_if_needed;
}

std::string TimberConverter::generate_4vector_label(std::string input, std::string prefix, std::string suffix) {
    std::stringstream delimit;
    std::stringstream command_text;

    delimit << input;
    std::vector<std::string> delimited_by_space;
    std::string buffer;
    while (std::getline(delimit, buffer, '\x1d')) {
        delimited_by_space.push_back(buffer);
    }

    if (delimited_by_space.size() == 1) {
        // if we have only one element, presume that we need to append to the end regardless of anything else
        command_text << prefix << delimited_by_space[0] << suffix;
        return command_text.str();
    }

    for (auto it = delimited_by_space.begin(); it != std::prev(delimited_by_space.end()); ++it) {
        if (it->back() != '+' && it->back() != '-' && it->back() != ')') command_text << prefix;
        command_text << *it;
        if (it->back() != '+' && it->back() != '-' && it->back() != ')') command_text << suffix;
    }
    command_text << delimited_by_space.back();
    return command_text.str();

}

std::string TimberConverter::generate_4vector_label(std::string input, std::string suffix) {
    return generate_4vector_label(input, "", suffix);
}

void TimberConverter::append_4vector_label(AnalysisCommand command, std::string prefix, std::string suffix, std::string prefix_if_lv, std::string suffix_if_lv) {
    std::string output = command.get_argument(0);
    std::string input = get_mapping_if_exists(command.get_argument(1));
    if (is_lorentz_vector.count(input) != 0) {
        // in this case, this input is a lorentz vector object, and we want to get its traits via an object attribute
        if (suffix_if_lv == "" && prefix_if_lv == "") {
            raise_non_implemented_conversion_exception(AnalysisCommand::instruction_to_text(command.get_instruction()), "this function makes sense only when used on a 4-vector, but is being used on a raw NanoAOD value");
        }
        var_mappings[output] = generate_4vector_label(input, prefix_if_lv, suffix_if_lv);

    } else {
        if (suffix == "" && prefix == "") {
            raise_non_implemented_conversion_exception(AnalysisCommand::instruction_to_text(command.get_instruction()), "this function makes sense only when used on raw NanoAOD values, but is being used on an added 4-vector");        }
        var_mappings[output] = generate_4vector_label(input, prefix, suffix);
    }
}


void TimberConverter::append_4vector_label(AnalysisCommand command, std::string suffix, std::string suffix_if_lv) {
    append_4vector_label(command, "", suffix, "", suffix_if_lv);
}

std::string TimberConverter::binary_command(AnalysisCommand command, std::string op) {
    std::stringstream text;
    text << var_mappings[command.get_argument(1)] << op << var_mappings[command.get_argument(2)];
    return text.str();
}

std::string TimberConverter::command_convert(AnalysisCommand command) {

    AnalysisCommand new_command(command.get_instruction());

    for (int i = 0; i < command.get_num_arguments(); i++) {
        std::regex e("[_\\->]");
        std::string new_arg = command.get_argument(i);
        if (new_arg[0] != '"') {
            new_arg = std::regex_replace(command.get_argument(i), e, "w");
        }
        if (i == 0) 
            new_command.add_dest_argument(new_arg);
        else
            new_command.add_source_argument(new_arg);
    }
    command = new_command;


    AnalysisLevelInstruction inst = command.get_instruction();
    std::stringstream command_text;

    switch (inst) {
        case DO_CUTFLOW_ON_REGION:
            command_text << "\n_old_node = a.GetActiveNode()";
            command_text << "\n_cutflow_node_" << command.get_argument(0) << " = a.Apply(" << get_mapping_if_exists(command.get_argument(0)) << "[0])";
            command_text << "\n_cutflow_node_" << command.get_argument(0) << " = a.AddCorrections(" << get_mapping_if_exists(command.get_argument(0)) << "[1])";
            
            command_text << "\nprint('\\n---\\nBeginning cutflow report for region ";
            {
                std::regex e(".*REGx");
                std::string clean_name = std::regex_replace(command.get_argument(0), e, "");
                command_text << clean_name;
            }
            command_text << "')";

            command_text << "\nfor _cutflow_k, _cutflow_v in CutflowDict(_cutflow_node_" << command.get_argument(0) << ").items():\n    print('After cut ' + _cutflow_k + ', ' + str(_cutflow_v) + ' remain')";
            command_text << "\nprint('\\n---\\n')";
            command_text << "\na.SetActiveNode(_old_node)\n";
            return command_text.str();
        case DO_EVENTLIST_ON_REGION:            
            command_text << "\n_old_node = a.GetActiveNode()";
            command_text << "\n_eventlist_node_" << command.get_argument(0) << " = a.Apply(" << get_mapping_if_exists(command.get_argument(0)) << "[0])";
            command_text << "\n_eventlist_node_" << command.get_argument(0) << " = a.AddCorrections(" << get_mapping_if_exists(command.get_argument(0)) << "[1])";
            
            command_text << "\nprint('\\n---\\nBeginning event list for region ";
            {
                std::regex e(".*REGx");
                std::string clean_name = std::regex_replace(command.get_argument(0), e, "");
                command_text << clean_name;
            }
            command_text << "')";

            command_text << "\n_eventlist_node_" << command.get_argument(0) << ".DataFrame.Display(columnList=['run', 'luminosityBlock', 'event'], nRows=1000).Print()";
            command_text << "\nprint('\\n---\\n')";
            command_text << "\na.SetActiveNode(_old_node)\n";
            return command_text.str();
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
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.Apply(" << get_mapping_if_exists(command.get_argument(1)) << "[0])";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.AddCorrections(" << get_mapping_if_exists(command.get_argument(1)) << "[1])";
            command_text << "\nuse_histo(_histogram" << command.get_argument(0) << ", _histogram_node_" << command.get_argument(1) << ")";
            command_text << "\na.SetActiveNode(_old_node)";
            return command_text.str();
        case CREATE_HIST_LIST:
            command_text << "\n_histogram_list" << command.get_argument(0) << " = []";
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            return command_text.str();
        case ADD_HIST_TO_LIST:
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
            command_text << "\n_histogram_list" << var_mappings[command.get_argument(1)] << ".append(_histogram" << command.get_argument(2) << ")";
            return command_text.str();
        case USE_HIST_LIST:
            command_text << "\n_old_node = a.GetActiveNode()";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.Apply(" << get_mapping_if_exists(command.get_argument(1)) << "[0])";
            command_text << "\n_histogram_node_" << command.get_argument(1) << " = a.AddCorrections(" << get_mapping_if_exists(command.get_argument(1)) << "[1])";
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
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(2));
            return command_text.str();
        case CUT_REGION:
            command_text << var_mappings[command.get_argument(1)] << "[0].Add('" << command.get_argument(0) << "', '" << var_mappings[command.get_argument(2)] << "')"; 
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
            return command_text.str();
        case ADD_ALIAS:
        {
            std::string source = get_mapping_if_exists(command.get_argument(1));
            std::string dest = command.get_argument(0);
            var_mappings[dest] = get_mapping_if_exists(source);

            // if we are aliasing a 4vector object, we should define it so that we do not do too much redundant work
            if (is_lorentz_vector.count(source) != 0) {
    
                std::regex e ("\x1d"); 
                source = std::regex_replace(source, e, "");

                char non_underscore_delimiter = 'w';

                command_text << "\na.Define('" << non_underscore_delimiter << "VEC" << non_underscore_delimiter << dest << "', '" << source << "')";
                // is_lorentz_vector.insert(command.get_argument(0));
                command_text << "\na.Define('" << dest << "_pt', 'Pt(" << non_underscore_delimiter << "VEC" << non_underscore_delimiter << dest << ")')";
                command_text << "\na.Define('" << dest << "_eta', 'Eta(" << non_underscore_delimiter << "VEC" << non_underscore_delimiter << dest << ")')";
                command_text << "\na.Define('" << dest << "_phi', 'Phi(" << non_underscore_delimiter << "VEC" << non_underscore_delimiter << dest << ")')";
                command_text << "\na.Define('" << dest << "_mass', 'M(" << non_underscore_delimiter << "VEC" << non_underscore_delimiter << dest << ")')";

                var_mappings[command.get_argument(0)] = command.get_argument(0);
                return command_text.str();
            }
            return "";
        }
        case ADD_EXTERNAL:
        {
            std::string fn_name_with_quotes = command.get_argument(1);
            std::string fn_name_wo_quotes = fn_name_with_quotes.substr(1,fn_name_with_quotes.size()-2);
            var_mappings[command.get_argument(0)] = fn_name_wo_quotes;
            return "";
        }
        case ADD_CORRECTIONLIB:
        {
            std::string filename_with_quotes = command.get_argument(1);

            std::string keyname_with_quotes = command.get_argument(2);
            
            std::stringstream correctionlib_func_name;
            correctionlib_func_name << command.get_argument(0) << "->evaluate";
            var_mappings[command.get_argument(0)] = correctionlib_func_name.str();

            command_text << "ROOT.gInterpreter.Declare('auto " << command.get_argument(0) << " = correction::CorrectionSet::from_file(" << filename_with_quotes << ")->at(" << keyname_with_quotes << ")')\n";
            return command_text.str();
        }
        //TODO: check that this is consistant with the overall syntax
        case SORT_ASCEND:
            command_text << "ROOT::VecOps::Sort(" << command.get_argument(1) << ")";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case SORT_DESCEND:
            command_text << "ROOT::VecOps::Reverse(ROOT::VecOps::Sort(" << command.get_argument(1) << "))";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case CREATE_MASK:
        {
            command_text << "\n" << command.get_argument(0) << " = VarGroup('" << command.get_argument(0) << "')\n"; 

            append_4vector_label(command, "", "_pt", "Pt(", ")");
            command_text << command.get_argument(0) << ".Add('" << command.get_argument(0) << "', 'create_mask(" << get_mapping_if_exists(command.get_argument(0)) << ")')";            
            var_mappings[command.get_argument(0)] = command.get_argument(0);

            existing_definitions.push_back(command.get_argument(0));
            return command_text.str();
        }
        case LIMIT_MASK:
        {   
            command_text << var_mappings[command.get_argument(1)] << ".Add('" << command.get_argument(0) << "', 'limit_mask(" << command.get_argument(1) << ", " << var_mappings[command.get_argument(2)] << ")')";
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
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
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
            command_text << var_mappings[command.get_argument(1)] << "_values.append(" << var_mappings[command.get_argument(2)] << "\n";
            command_text << var_mappings[command.get_argument(1)] << "_lower_bounds.append(" << var_mappings[command.get_argument(3)] << "\n";
            command_text << var_mappings[command.get_argument(1)] << "_upper_bounds.append(" << var_mappings[command.get_argument(4)] << "\n";
            return command_text.str();
        }
        case FINISH_TABLE:
        {
            std::string old_name = get_mapping_if_exists(command.get_argument(1));
            command_text << old_name << "_values_array = ROOT.ROOT::VecOps.AsRVec(np.array(" << old_name << "_values, dtype=np.float32))\n";
            command_text << old_name << "_lower_bounds_array = ROOT.ROOT::VecOps.AsRVec(np.array(" << old_name << "_lower_bounds, dtype=np.float32))\n";
            command_text << old_name << "_upper_bounds_array = ROOT.ROOT::VecOps.AsRVec(np.array(" << old_name << "_upper_bounds, dtype=np.float32))\n"; 

            command_text << "ROOT.gInterpreter.Declare('" << command.get_argument(0);
            command_text << " = create_table_function(' + str(" << old_name << "_nvars) + '," << old_name << "_lower_bound_array," << old_name << "_upper_bound_array," << old_name << "_values_array);')";
        }
        case WEIGHT_APPLY:
            command_text << var_mappings[command.get_argument(1)] << "[1].append(Correction('" << command.get_argument(2) << "', '', '" << var_mappings[command.get_argument(2)] << "')"; 
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
            return command_text.str();
        case BEGIN_EXPRESSION:
            return "";
        case END_EXPRESSION:
        {
            var_mappings[command.get_argument(0)] = get_mapping_if_exists(command.get_argument(1));
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
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        }
        case FUNC_PT:
            append_4vector_label(command, "", "_pt", "Pt(", ")");
            return "";
        case FUNC_ETA:
            append_4vector_label(command, "", "_eta", "Eta(", ")");
            return "";
        case FUNC_PHI:
            append_4vector_label(command, "", "_phi", "Phi(", ")");
            return "";
        case FUNC_MASS:
            append_4vector_label(command, "", "_mass", "M(", ")");
            return "";
        case FUNC_ENERGY:
            append_4vector_label(command, "", ".E()");
            return "";
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
            add_particle(command, met_name);
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
            add_particle(command, get_mapping_if_exists(command.get_argument(1)));
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
            sub_particle(command, met_name);
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
        case FUNC_ANYOF:
            command_text << "AnyOf(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_ALLOF:
            command_text << "ROOT::VecOps::All(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
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
            command_text << "ROOT::VecOps::Mean(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_SUM:
            command_text << "ROOT::VecOps::Sum(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";

        case FUNC_ANYOCCURRENCES:
            command_text << "AnyOccurrences(" << var_mappings[command.get_argument(1)] << "," << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
            
        case FUNC_MIN:
            command_text << "ROOT::VecOps::Min(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_MAX:
            command_text << "ROOT::VecOps::Max(" << var_mappings[command.get_argument(1)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";

        case FUNC_MAX_LIST:
            command_text << "std::max(" << var_mappings[command.get_argument(1)] << "," << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_MIN_LIST:
            command_text << "std::min(" << var_mappings[command.get_argument(1)] << "," << var_mappings[command.get_argument(2)] << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";

        case FUNC_FIRST:
            append_4vector_label(command, "xfirst\x1d");
            return "";
        case FUNC_SECOND:
            append_4vector_label(command, "xsecond\x1d");
            return "";
        case FUNC_SORT_ASCEND:
            command_text << "ROOT::VecOps::Sort(" << command.get_argument(1) << ")";
            var_mappings[command.get_argument(0)] = command_text.str(); return "";
        case FUNC_SORT_DESCEND:
            command_text << "ROOT::VecOps::Reverse(ROOT::VecOps::Sort(" << command.get_argument(1) << "))";
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
            command_text << add_all_relevant_tags_for_union_merge(command, get_mapping_if_exists(command.get_argument(2)));
            return command_text.str();        
        case ADD_ELECTRON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Electron");
            return command_text.str();
        case ADD_MUON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Muon");
            return command_text.str();
        case ADD_TAU_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Tau");
            return command_text.str();
        case ADD_TRACK_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "IsoTrack");
            return command_text.str();
        case ADD_LEPTON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Lepton");
            return command_text.str();
        case ADD_PHOTON_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Photon");
            return command_text.str();
        case ADD_BJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "BJet");
            return command_text.str();
        case ADD_QGJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "QGJet");
            return command_text.str();
        case ADD_NUMET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "MET");
            return command_text.str();
        case ADD_METLV_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, met_name);
            return command_text.str();
        case ADD_GEN_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "GenPart");
            return command_text.str();
        case ADD_JET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "Jet");
            return command_text.str();
        case ADD_FJET_TO_UNION:
            command_text << add_all_relevant_tags_for_union_merge(command, "FatJet");
            return command_text.str();

        case MAKE_EMPTY_COMB:
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            command_text << add_structure_for_comb_empty(command);
            existing_definitions.push_back(command.get_argument(0));
            return command_text.str();        
        case ADD_NAMED_TO_COMB:
            command_text << add_structure_for_comb_merge(command, get_mapping_if_exists(command.get_argument(2)));
            return command_text.str();        
        case ADD_ELECTRON_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Electron");
            return command_text.str();
        case ADD_MUON_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Muon");
            return command_text.str();
        case ADD_TAU_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Tau");
            return command_text.str();
        case ADD_TRACK_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "IsoTrack");
            return command_text.str();
        case ADD_LEPTON_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Lepton");
            return command_text.str();
        case ADD_PHOTON_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Photon");
            return command_text.str();
        case ADD_BJET_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "BJet");
            return command_text.str();
        case ADD_QGJET_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "QGJet");
            return command_text.str();
        case ADD_NUMET_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "MET");
            return command_text.str();
        case ADD_METLV_TO_COMB:
            command_text << add_structure_for_comb_merge(command, met_name);
            return command_text.str();
        case ADD_GEN_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "GenPart");
            return command_text.str();
        case ADD_JET_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "Jet");
            return command_text.str();
        case ADD_FJET_TO_COMB:
            command_text << add_structure_for_comb_merge(command, "FatJet");
            return command_text.str();

        case NAME_ELEMENT_OF_COMB:
            command_text << add_comb_argument(get_mapping_if_exists(command.get_argument(0)), get_mapping_if_exists(command.get_argument(1)), get_mapping_if_exists(command.get_argument(2)));
            return command_text.str();

//TODO: add DISJOINT

        case MAKE_EMPTY_DISJOINT:
            var_mappings[command.get_argument(0)] = command.get_argument(0);
            command_text << add_structure_for_comb_empty(command);
            existing_definitions.push_back(command.get_argument(0));
            return command_text.str();        
        case ADD_NAMED_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, get_mapping_if_exists(command.get_argument(2)));
            return command_text.str();        
        case ADD_ELECTRON_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Electron");
            return command_text.str();
        case ADD_MUON_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Muon");
            return command_text.str();
        case ADD_TAU_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Tau");
            return command_text.str();
        case ADD_TRACK_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "IsoTrack");
            return command_text.str();
        case ADD_LEPTON_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Lepton");
            return command_text.str();
        case ADD_PHOTON_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Photon");
            return command_text.str();
        case ADD_BJET_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "BJet");
            return command_text.str();
        case ADD_QGJET_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "QGJet");
            return command_text.str();
        case ADD_NUMET_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "MET");
            return command_text.str();
        case ADD_METLV_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, met_name);
            return command_text.str();
        case ADD_GEN_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "GenPart");
            return command_text.str();
        case ADD_JET_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "Jet");
            return command_text.str();
        case ADD_FJET_TO_DISJOINT:
            command_text << add_structure_for_comb_merge(command, "FatJet");
            return command_text.str();

        case NAME_ELEMENT_OF_DISJOINT:
            command_text << add_comb_argument(get_mapping_if_exists(command.get_argument(0)), get_mapping_if_exists(command.get_argument(1)), get_mapping_if_exists(command.get_argument(2)), true);
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
        case FUNC_DZ:
            append_4vector_label(command, "_dz");
            return "";
        case FUNC_THETA:
            raise_non_implemented_conversion_exception("FUNC_THETA");
            return "FUNC_THETA";
        case FUNC_ABS_ISO:
            raise_non_implemented_conversion_exception("FUNC_ABS_ISO");
            return "FUNC_ABS_ISO";
        case FUNC_MINI_ISO:
            raise_non_implemented_conversion_exception("FUNC_MINI_ISO");
            return "FUNC_MINI_ISO";
        case FUNC_DR:
            command_text << "LVDeltaR(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DPHI:
            command_text << "LVDeltaPhi(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DETA:
            command_text << "LVDeltaEta(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            return "";
        case FUNC_DR_HADAMARD:
            command_text << "LVDeltaRHadamard(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DPHI_HADAMARD:
            command_text << "LVDeltaPhiHadamard(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case FUNC_DETA_HADAMARD:
            command_text << "LVDeltaEtaHadamard(" << lorentzify(get_mapping_if_exists(command.get_argument(1))) << ", " << lorentzify(get_mapping_if_exists(command.get_argument(2))) << ")";
            return "";
        case FUNC_SIZE: //TODO: check this does not conflict with a valid use case
            command_text << "size(" << generate_4vector_label(get_mapping_if_exists(command.get_argument(1)), "_pt") << ")";
            var_mappings[command.get_argument(0)] = command_text.str();
            return "";
        case CREATE_BIN:
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

    met_name = config.get_argument("MET");
    std::string in_file = config.get_argument("infile");

    // get the path for our helper functions, relying on the "ROOT DIR" macro which we set during compile time
    std::filesystem::path abs_path = std::filesystem::absolute(ROOT_DIR);
    std::filesystem::path path_to_helper_cpp = abs_path / "helpers" / "adl_helpers.cc";
    std::filesystem::path path_to_helper_py = abs_path / "helpers";

    std::stringstream preliminary;
    // import our needed TIMBER and other python functions
    preliminary << 
        "from TIMBER.Analyzer import *\nfrom TIMBER.Tools.Common import *\nimport ROOT\nimport sys, os\n";
        
    // add the path to the python helper file as an import, regardless of its location
    preliminary << 
        "adl_help_dir = os.path.abspath('" << path_to_helper_py.string() << "')\nif adl_help_dir not in sys.path:\n    sys.path.append(adl_help_dir)\n";

    // import all our needed python helper functions
    preliminary <<
        "from adl_helpers import combine_without_duplicates, use_histo, use_histo_list\n";
        
    // compile the cpp helper functions into this
    preliminary <<
        "CompileCpp('" << path_to_helper_cpp.string() << "')\n";
        
    // open up the input file and an output file
    preliminary << 
        "a = analyzer('" << in_file << "')\nout = ROOT.TFile.Open('adl_out.root','UPDATE')";

    std::cout << preliminary.str() << std::endl;

    std::stringstream definitions;
    definitions <<
        "\na.Define('" << met_name << "_eta','" << met_name << "_pt - " << met_name << "_pt')\na.Define('" << met_name << "_mass', '" << met_name << "_eta')";

    std::cout << definitions.str() << std::endl;

    while (alil->clear_to_next()) {
        std::string out = command_convert(alil->next_command());
        if (out == "") continue;
        std::cout << out << std::endl;
    }

    std::string postscriptum = 
        "\nout.Close()\n";
    std::cout << postscriptum << std::endl;
}

void TimberConverter::print() {
    print_timber();
}
