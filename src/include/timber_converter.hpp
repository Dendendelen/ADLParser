#ifndef TIMBER_CONVERTER_H
#define TIMBER_CONVERTER_H

#include "ali_converter.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class TimberConverter {

    private:
        
        std::unique_ptr<ALIConverter> alil;

        std::vector<std::string> existing_definitions;
        std::unordered_map<std::string, std::string> var_mappings;
        std::unordered_map<std::string, std::vector<std::string>> region_groups;

        std::unordered_set<std::string> needs_btag;

        std::string command_convert(AnalysisCommand command);
        std::string binary_command(AnalysisCommand command, std::string op);
        void append_4vector_label(AnalysisCommand command, std::string suffix);
        void sub_particle(AnalysisCommand command, std::string name);
        void add_particle(AnalysisCommand command, std::string name);
        void index_particle(AnalysisCommand command, bool is_named, std::string part_text);
        std::string existing_definitions_string();
        std::string add_all_relevant_tags_for_object(AnalysisCommand command);
        std::string add_all_relevant_tags_for_union_merge(AnalysisCommand command, std::string adding_name);
        std::string add_all_relevant_tags_for_union_empty(AnalysisCommand command);


    public:
        TimberConverter(ALIConverter *alil_in);

        void print_timber();
};


#endif