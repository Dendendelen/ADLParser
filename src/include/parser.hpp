#ifndef PARSER_H
#define PARSER_H

#include "lexer.hpp"
#include "node.hpp"

class Parser {
    private:
        std::unique_ptr<Lexer> lexer;
        Tree tree;

        void parse_blocks(PNode parent);

        void parse_info(PNode parent);
        void parse_initializations(PNode parent);
        void parse_initialization(PNode parent);

        void parse_definition(PNode parent);
        void parse_def_rvalue(PNode parent);

        void parse_composite(PNode parent);
        void parse_composite_rvalue(PNode parent);
        void parse_composite_criteria(PNode parent);
        void parse_composite_criterion(PNode parent);

        void parse_object(PNode parent);
        void parse_obj_rvalue(PNode parent);
        void parse_obj_criteria(PNode parent);
        void parse_obj_criterion(PNode parent);

        void parse_table(PNode parent);
        void parse_table_header(PNode parent);

        void parse_region(PNode parent);
        void parse_region_commands(PNode parent);
        void parse_region_command(PNode parent);

        void parse_region_command_weight(PNode parent);
        void parse_region_command_histo(PNode parent);

        void parse_histo_list(PNode parent);
        void parse_histo_entries(PNode parent);
        void parse_histo_entry(PNode parent);
        void parse_histogram(PNode parent);
        void parse_binning(PNode parent);

        void parse_bool(PNode parent);
        void parse_id(PNode parent);
        void parse_assignment();

        void parse_particle_sum(PNode parent);
        void parse_particle_list(PNode parent);
        void parse_named_particle_list(PNode parent);
        void parse_literal_number_list(PNode parent);
        void parse_string_list(PNode parent);
        void parse_variable_list(PNode parent);

        void parse_particle(PNode parent);
        void parse_index(PNode parent);


        PNode precedence_climber(PNode parent, int min_precedence);
        PNode parse_primary_expression(PNode parent);
        PNode parse_expression(PNode parent);


        void print_children_and_yourself(PNode node, int *top_number);


    public:
        Parser(Lexer *lex);
        
        void parse();

        void parse_input();

        void print_parse_dot();

        PNode get_root();


};


#endif