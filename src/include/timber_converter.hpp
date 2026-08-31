#ifndef TIMBER_CONVERTER_H
#define TIMBER_CONVERTER_H

#include "alil.hpp"
#include "alil_converter.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#define CONVERTER_FUNCS_DECLARE(ENUM, NAME) \
    std::string convert_##NAME(const AnalysisCommand &) override;

class TimberConverter : public ALILToFrameworkCompiler {

    private:

        std::unordered_map<std::string, std::string> var_mappings;

        std::unordered_set<std::string> particle_already_has_provenance;
        std::unordered_set<std::string> is_attribute;
        std::unordered_set<std::string> is_lorentz;

        std::string met_name;
        std::string attribute_delimiter;
 
        void add_mapping(std::string source, std::string dest);
        std::string get_mapping(std::string);
        std::string get_mapped_source(const AnalysisCommand &command, size_t pos);
        std::string get_mapped_dest(const AnalysisCommand &command);

        std::string list_append(std::string list_end, std::string delimiter, const AnalysisCommand &command, std::string to_add = "");
        std::string attribute(std::string attr, std::string object, std::string separator_chars);
        std::string attribute(std::string attr, std::string object);
        std::string lorentzify(std::string object);
        std::string multi_arg_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok = "", bool is_lorentz = false);
        std::string multi_arg_lorentz_function(std::string func_name, int num_args, const AnalysisCommand &command, std::string ending_tok = "");
        std::string binary_infix_operation(std::string op_name, const AnalysisCommand &command);
        std::string interval(std::string left_bound_op, std::string right_bound_op, const AnalysisCommand &command);
        std::string add_subtract_particles(const AnalysisCommand &command, bool is_subtraction = false);
        std::string use_within_region(std::string fun_within_node, const AnalysisCommand &command);

        template<typename... Args>
        void emit(Args... args) {
            (std::cout << ... << args) << '\n' << std::flush;
        };
        void emit_newline() {
            emit("");
        };

        template<typename... Args>
        void emit_comment(Args... args) {
            emit("# ", args...);
        }

        void handle_command(const AnalysisCommand &command);
        
    protected:
        ALIL_INSTRUCTION_LIST(CONVERTER_FUNCS_DECLARE);

    public:
        using ALILToFrameworkCompiler::ALILToFrameworkCompiler;
        void print() override;
};

#undef CONVERTER_FUNCS_DECLARE

#endif