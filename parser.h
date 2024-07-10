#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "node.h"

class Parser {
    private:
        std::unique_ptr<Lexer> lexer;
        Tree tree;

        void parse_blocks(PNode parent);

        PNode parse_info(PNode parent);
        PNode parse_count_format(PNode parent);
        PNode parse_region(PNode parent);
        PNode parse_object(PNode parent);
        PNode parse_definition(PNode parent);
        PNode parse_table(PNode parent);

        void parse_initializations(PNode parent);
        void parse_count_processes(PNode parent);

        void parse_region_commands(PNode parent);
        PNode parse_region_command(PNode parent);

        PNode parse_initialization(PNode parent);

        PNode parse_obj_rvalue(PNode parent);
        PNode parse_def_rvalue(PNode parent);

        void parse_criteria(PNode parent);
        PNode parse_criterion(PNode parent);

        PNode parse_hamhum(PNode parent);
        PNode parse_index(PNode parent);

        void parse_particle_list(PNode parent);
        PNode parse_particle(PNode parent);

        PNode parse_action(PNode parent);
        PNode parse_if_or_condition(PNode parent);
        PNode parse_condition(PNode parent);
        void parse_bin_or_box_values(PNode parent);
        void parse_counts(PNode parent);
        void parse_count(PNode parent);

        PNode parse_expression(PNode parent);

        PNode parse_bool(PNode parent);
        PNode parse_id(PNode parent);

        PNode parse_description(PNode parent);
        PNode parse_count_process(PNode parent);
        PNode parse_err_type(PNode parent);


        void parse_variable_list(PNode parent);
        PNode parse_lepton(PNode parent);

        PNode parse_syst_vtype(PNode parent);

        PNode parse_region_command_cmd(PNode parent);
        PNode parse_region_command_bins(PNode parent);
        PNode parse_region_command_weight(PNode parent);
        PNode parse_region_command_histo(PNode parent);

    public:
        Parser(Lexer *lex);
        
        void parse();

        void parse_input();

        void print_parse_dot();


};


#endif