#ifndef TIMBER_CONVERTER_H
#define TIMBER_CONVERTER_H

#include "ali_converter.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class TimberConverter : public ALILToFrameworkCompiler {

    private:

        std::vector<std::string> existing_definitions;
        std::unordered_map<std::string, std::string> var_mappings;
        std::unordered_map<std::string, std::vector<std::string>> region_groups;

        std::unordered_set<std::string> empty_union_names;
        std::unordered_set<std::string> is_lorentz_vector;
        std::unordered_set<std::string> comb_already_made;
        std::unordered_set<std::string> already_applied_globally;

        std::unordered_map<std::string, std::vector<std::string>> comb_map;

        std::string met_name;
 
        std::string command_convert(AnalysisCommand command);


        std::string lorentzify(std::string name);
        std::string binary_command(AnalysisCommand command, std::string op);

        std::string generate_4vector_label(std::string input, std::string prefix, std::string suffix);
        std::string generate_4vector_label(std::string input, std::string suffix);

        void append_4vector_label(AnalysisCommand command, std::string suffix, std::string suffix_if_lv = "");
        void append_4vector_label(AnalysisCommand command, std::string prefix, std::string suffix, std::string prefix_if_lv, std::string suffix_if_lv);

        void sub_particle(AnalysisCommand command, std::string name);
        void add_particle(AnalysisCommand command, std::string name);
        std::string index_particle(AnalysisCommand command, bool is_named, std::string part_text);
        std::string existing_definitions_string();
        std::string add_all_relevant_tags_for_object(AnalysisCommand command);

        std::string add_all_relevant_tags_for_union_merge(AnalysisCommand command, std::string adding_name);
        std::string add_all_relevant_tags_for_union_empty(AnalysisCommand command);

        std::string add_structure_for_comb_empty(AnalysisCommand command);
        std::string add_structure_for_comb_merge(AnalysisCommand command, std::string adding_name);
        std::string add_comb_argument(std::string new_name, std::string name_of_comb, std::string val, bool disjoint=false);


        std::string get_mapping_if_exists(std::string str);


    public:
        using ALILToFrameworkCompiler::ALILToFrameworkCompiler;
        void print_timber();
        void print() override;
};


#endif