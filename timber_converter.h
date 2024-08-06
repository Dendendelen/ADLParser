#ifndef TIMBER_CONVERTER_H
#define TIMBER_CONVERTER_H

#include "aliconverter.h"
#include <string>
#include <unordered_map>
#include <vector>


class TimberConverter {

    private:
        std::unordered_map<std::string, std::string> var_mappings;
        std::unique_ptr<ALIConverter> alil;
        std::vector<std::string> existing_definitions;

        std::string command_convert(AnalysisCommand command);
        std::string binary_command(AnalysisCommand command, std::string op);
        void append_4vector_label(AnalysisCommand command, std::string suffix);


    public:
        TimberConverter(ALIConverter *alil_in);

        void print_timber();
};


#endif