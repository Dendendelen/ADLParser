#include "alil_converter.hpp"
#include "alil.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <iostream>
#include <string>



std::string ALILConverter::reserve_scoped_value_name() {
    std::stringstream new_var_name;
    new_var_name << "_V" << highest_var_val++ << "_" << current_scope_name;
    std::string value_name = new_var_name.str();
    return value_name;
}


ALILConverter::NameScope::NameScope(std::string type_name, ALILConverter *converter) {
    this_converter = converter;
    old_name = converter->current_scope_name;

    std::stringstream scope_name;
    scope_name << type_name << "_" << old_name;

    converter->current_scope_name = scope_name.str();
}

ALILConverter::NameScope::NameScope(std::string type_name, PNode id_node, ALILConverter *converter) {
    this_converter = converter;
    old_name = converter->current_scope_name;

    std::string name_lexeme = id_node->get_token()->get_lexeme();
    std::stringstream scope_name;
    scope_name << type_name << "_" << name_lexeme << "_" << old_name;

    converter->current_scope_name = scope_name.str();
}

ALILConverter::NameScope::~NameScope() {
    this_converter->current_scope_name = old_name;
}

void ALILConverter::visit_info(PNode node) {
    // INFO -> ID
    //      -> INITIALIZATIONS

    PNode info_id_node = node->get_child(0);
    PNode initializations_node = node->get_child(1);

    NameScope info_scope("INFO", info_id_node, this);

    visit_children(node);

    AnalysisCommandBuilder give_init_name(ALIL::ADD_ALIAS);
    std::string name_of_init = info_id_node->consume_associated_string();
    give_init_name.add_dest_argument(name_of_init);
    give_init_name.add_source_argument(initializations_node->consume_associated_string());

    give_init_name.collect_into(commands);

    AnalysisCommandBuilder display_info(ALIL::DISPLAY_INFO);
    display_info.add_source_argument(name_of_init);
    display_info.add_empty_dest();

    display_info.collect_into(commands);
}

void ALILConverter::visit_definition(PNode node) {
    // DEFINITION -> ID
    //            -> EXTERN_ATTR |_| EXTERN_PARTICLE |_| EXTERN_FUN |_| CORRECTIONLIB |_| PARTICLE_SUM |_| EXPRESSION

    PNode def_id_node = node->get_child(0);
    NameScope def_scope("DEF", def_id_node, this);

    PNode def_type_node = node->get_child(1);

    visit_children(node);

    std::optional<AnalysisCommandBuilder> operation;

    switch (def_type_node->get_ast_type()) {
        case AST::EXTERN_ATTR:
            operation.emplace(ALIL::ADD_EXTERN_ATTR);
            operation->add_source_argument(def_type_node->get_child(0)->consume_associated_string());
            break;
        case AST::EXTERN_PARTICLE:
            operation.emplace(ALIL::ADD_EXTERN_PARTICLE);
            operation->add_source_argument(def_type_node->get_child(0)->consume_associated_string());
            break;
        case AST::EXTERN_FUN:
            operation.emplace(ALIL::ADD_EXTERNAL);
            operation->add_source_argument(def_type_node->get_child(0)->consume_associated_string());
            break;
        case AST::CORRECTIONLIB:
            operation.emplace(ALIL::ADD_CORRECTIONLIB);
            operation->add_source_argument(def_type_node->get_child(0)->consume_associated_string());
            operation->add_source_argument(def_type_node->get_child(1)->consume_associated_string());
            break;
        case AST::PARTICLE_SUM: case AST::EXPRESSION:
            operation.emplace(ALIL::ADD_ALIAS);
            operation->add_source_argument(def_type_node->consume_associated_string());
            break;
        default:
            raise_analysis_conversion_exception("Unexpected rvalue of a definition", def_type_node->get_token());
    
    }

    operation->add_dest_argument(def_id_node->consume_associated_string());
    operation->collect_into(commands);
}

void ALILConverter::visit_composite(PNode node) {
    // COMPOSITE -> ID
    //           -> COMPOSITE_CARTESIAN |_| COMPOSITE_DISJOINT |_| COMPOSITE_DIRECT
    //           -> NAMED_PARTICLE_LIST
    //           -> COMP_CRITERIA

    PNode comp_id_node = node->get_child(0);
    NameScope comp_scope("COMP", comp_id_node, this);

    PNode comp_type_node = node->get_child(1);
    PNode comp_naming_elements = node->get_child(2);

    visit_children_before_index(node, 2);

    std::string overall_name_of_composite = comp_id_node->consume_associated_string();
    std::optional<AnalysisCommandBuilder> make_empty_composite; 
    
    switch (comp_type_node->get_ast_type()) {
        case AST::COMPOSITE_CARTESIAN:
            make_empty_composite.emplace(AnalysisLevelInstruction::CREATE_EMPTY_CARTESIAN);
            break;
        case AST::COMPOSITE_DISJOINT:
            make_empty_composite.emplace(AnalysisLevelInstruction::CREATE_EMPTY_DISJOINT);
            break;
        case AST::COMPOSITE_DIRECT:
            make_empty_composite.emplace(AnalysisLevelInstruction::CREATE_EMPTY_DIRECT);
            break;
        default:
            raise_analysis_conversion_exception("Invalid type for a composite", comp_type_node->get_token());
            break;
    }
    make_empty_composite->add_empty_source();
    std::string last_name_of_comp = make_empty_composite->reserve_dest_arg_value(this);
    make_empty_composite->collect_into(commands);

    what_global_name_for_this_comp_name.clear();
    int index = 0;
    bool is_particle_step = true;
    visit(comp_naming_elements);

    std::queue<std::string> input_parts;

    for (PNode particle : comp_naming_elements->get_children()) {
        if (is_particle_step) {
            AnalysisCommandBuilder add_part_to_comp(ALIL::ADD_PART_TO_COMPOSITE);
            add_part_to_comp.add_source_argument(last_name_of_comp); 

            std::string new_input_part = particle->consume_associated_string();
            add_part_to_comp.add_source_argument(new_input_part);
            input_parts.push(new_input_part);

            // new name of the composite that now includes this particle
            last_name_of_comp = add_part_to_comp.reserve_dest_arg_value(this);
            
            add_part_to_comp.collect_into(commands);
        }
        is_particle_step = !is_particle_step;
    }

    assert(is_particle_step);

    bool is_name_step = false;
    for (PNode name_part : comp_naming_elements->get_children()) {
        if (is_name_step) {
            AnalysisCommandBuilder naming_command(ALIL::NAME_ELEMENT_OF_COMPOSITE);
            naming_command.add_source_argument(last_name_of_comp);
            naming_command.add_source_argument(std::to_string(index));
            naming_command.add_source_argument(input_parts.front());
            input_parts.pop();

            // get the name by which we will locally refer to this
            std::string local_name = name_part->consume_associated_string();

            NameScope naming_scope(local_name, this);

            // associate this local name with the global name we have reserved
            what_global_name_for_this_comp_name.emplace(local_name, naming_command.reserve_dest_arg_value(this));
            naming_command.collect_into(commands);
            index++;
        } 
        is_name_step = !is_name_step;
    }

    assert(!is_name_step);

    visit_children_after_index(node, 2);

    std::string last_mask = node->get_child(3)->consume_associated_string();

    for (auto local_global_pair : what_global_name_for_this_comp_name) {

        std::string local_name = local_global_pair.first;
        std::string global_name = local_global_pair.second;

        std::stringstream cut_down_global_name;
        cut_down_global_name << overall_name_of_composite << "->" << local_name;

        AnalysisCommandBuilder cut_down_element(ALIL::APPLY_MASK);
        cut_down_element.add_dest_argument(cut_down_global_name.str());
        cut_down_element.add_source_argument(last_mask);
        cut_down_element.add_source_argument(global_name);

        cut_down_element.collect_into(commands);
    }

    what_global_name_for_this_comp_name.clear();

}

void ALILConverter::visit_object(PNode node) {
    // OBJECT -> ID
    //        -> OBJ_UNION |_| OBJ_SORT |_| PARTICLE
    //        -> OBJECT_CRITERIA

    PNode obj_id_node = node->get_child(0);
    NameScope obj_scope("OBJ", obj_id_node, this);

    PNode obj_type_node = node->get_child(1);
    PNode obj_criteria_node = node->get_child(2);

    visit_children_before_index(node, 2);

    // the this keyword should point to this object until we are done
    what_object_is_this = obj_type_node->consume_associated_string();

    visit(obj_criteria_node);

    AnalysisCommandBuilder cut_down_object(ALIL::APPLY_MASK);
    cut_down_object.add_dest_argument(obj_id_node->consume_associated_string());
    cut_down_object.add_source_argument(obj_criteria_node->consume_associated_string());
    cut_down_object.add_source_argument(what_object_is_this);

    cut_down_object.collect_into(commands);

    what_object_is_this = "";
}


void ALILConverter::visit_table_def(PNode node) {
    PNode table_id_node = node->get_child(0);
    NameScope table_scope("TABLE", table_id_node, this);

    PNode nvars_node = node->get_child(2);
    PNode do_errors_node = node->get_child(3);
    PNode table_list_node = node->get_child(4);

    visit_children(node);

    bool do_errors = false;
    if (do_errors_node->get_ast_type() == AST::TRUE) do_errors = true;


    // size of the actual table
    int table_size = table_list_node->get_children().size();

    std::string num_vars_string = nvars_node->consume_associated_string();
    int num_vars = std::stoi(num_vars_string);
    
    // get the number of entries this table has implicitly - it should be table_size / ((1 or 3) + 2*num_vars)
    // 1 or 3 for actual values, 2*num_vars for the upper and lower bounds for every variable
    int num_columns_per_row = (do_errors ? 3 : 1) + 2*num_vars;
    if ((table_size % num_columns_per_row) != 0) {
        raise_analysis_conversion_exception("Invalid table, it is not square: likely at least one row is missing at least one component", nvars_node->get_token());
    }
    int num_entries = table_size / num_columns_per_row;


    AnalysisCommandBuilder create_table(ALIL::CREATE_TABLE, table_id_node->get_token());

    std::string current_table = create_table.reserve_dest_arg_value(this);
    create_table.add_empty_source();
    // create_table.add_source_argument(num_vars_string);

    create_table.collect_into(commands);

    auto table_node_iterator = table_list_node->get_children().begin();

    for (int row = 0; row < num_entries; row++) {
        AnalysisCommandBuilder append_to_table(ALIL::APPEND_TO_TABLE);

        append_to_table.add_source_argument(current_table);
        current_table = append_to_table.reserve_dest_arg_value(this);

        AnalysisCommandBuilder create_table_value(do_errors ? ALIL::CREATE_TABLE_ERRORED_VALUE : ALIL::CREATE_TABLE_VALUE);
        std::string values_name = create_table_value.reserve_dest_arg_value(this);
        
        AnalysisCommandBuilder lower_bound_list(ALIL::CREATE_EMPTY_VALUE_LIST);
        lower_bound_list.add_empty_source();
        std::string last_lower_bound = lower_bound_list.reserve_dest_arg_value(this);

        lower_bound_list.collect_into(commands);

        AnalysisCommandBuilder upper_bound_list(ALIL::CREATE_EMPTY_VALUE_LIST);
        upper_bound_list.add_empty_source();
        std::string last_upper_bound = upper_bound_list.reserve_dest_arg_value(this);

        upper_bound_list.collect_into(commands);

        for (int col = 0; col < num_columns_per_row; col++, table_node_iterator++) {

            assert(table_node_iterator != table_list_node->get_children().end());

            std::string current_arg_text = (*table_node_iterator)->consume_associated_string();
            if (col <= (do_errors ? 2 : 0)) {
                create_table_value.add_source_argument(current_arg_text);
            } else if (col % 2 == 0) {

                AnalysisCommandBuilder new_upper_bound(ALIL::ADD_VALUE_TO_LIST);
                new_upper_bound.add_source_argument(last_upper_bound);
                new_upper_bound.add_source_argument(current_arg_text);
                last_upper_bound = new_upper_bound.reserve_dest_arg_value(this);
                new_upper_bound.collect_into(commands);
            } else {
                AnalysisCommandBuilder new_lower_bound(ALIL::ADD_VALUE_TO_LIST);
                new_lower_bound.add_source_argument(last_lower_bound);
                new_lower_bound.add_source_argument(current_arg_text);
                last_lower_bound = new_lower_bound.reserve_dest_arg_value(this);
                new_lower_bound.collect_into(commands);
            }   
        }

        
        append_to_table.add_source_argument(values_name);
        append_to_table.add_source_argument(last_lower_bound);
        append_to_table.add_source_argument(last_upper_bound);

        create_table_value.collect_into(commands);
        append_to_table.collect_into(commands);
    }

    AnalysisCommandBuilder final_naming(ALIL::FINISH_TABLE);
    final_naming.add_dest_argument(table_id_node->consume_associated_string());
    final_naming.add_source_argument(current_table);

    final_naming.collect_into(commands);
}


void ALILConverter::visit_region(PNode node) {
    // REGION -> ID
    //        -> REGION_COMMANDS

    PNode region_id_node = node->get_child(0);
    NameScope region_scope("REG", region_id_node, this);

    PNode region_commands_node = node->get_child(1);

    visit_children(node);

    AnalysisCommandBuilder final_name_of_region(ALIL::ADD_ALIAS);
    std::string final_reg_name = region_id_node->consume_associated_string();
    final_name_of_region.add_dest_argument(final_reg_name);
    final_name_of_region.add_source_argument(region_commands_node->consume_associated_string());

    final_name_of_region.collect_into(commands);

    // add cutflow and eventlist outputs here, we will remove them later if we do not need them.
    AnalysisCommandBuilder do_cutflow(ALIL::DO_CUTFLOW_ON_REGION);
    do_cutflow.add_source_argument(final_reg_name);
    do_cutflow.add_empty_dest();

    do_cutflow.collect_into(commands);

    AnalysisCommandBuilder do_eventlist(ALIL::DO_EVENTLIST_ON_REGION);
    do_eventlist.add_source_argument(final_reg_name);
    do_eventlist.add_empty_dest();

    do_eventlist.collect_into(commands);
}


void ALILConverter::visit_histo_list(PNode node) {
    PNode histolist_id_node = node->get_child(0);
    NameScope histolist_scope("HISTOLIST", histolist_id_node, this);

    AnalysisCommandBuilder create_histo_list(ALIL::CREATE_EMPTY_HIST_LIST);
    create_histo_list.add_empty_source();
    std::string last_list = create_histo_list.reserve_dest_arg_value(this);

    create_histo_list.collect_into(commands);

    PNode histo_entries_list = node->get_child(1);
    for (PNode histo : histo_entries_list->get_children()) {
        visit_children(histo);

        // this node is a HISTOLIST_HISTOGRAM node, and so its child is a histogram node
        std::string histo_produced = histo->get_child(0)->consume_associated_string();
        AnalysisCommandBuilder add_to_list(ALIL::ADD_HIST_TO_LIST);
        add_to_list.add_source_argument(last_list);
        add_to_list.add_source_argument(histo_produced);
        last_list = add_to_list.reserve_dest_arg_value(this);

        add_to_list.collect_into(commands);
    }

    AnalysisCommandBuilder finish_list(ALIL::ADD_ALIAS);
    finish_list.add_dest_argument(histolist_id_node->consume_associated_string());
    finish_list.add_source_argument(last_list);

    finish_list.collect_into(commands);
}

void ALILConverter::visit_initializations(PNode node) {
    // INITIALIZATIONS -> N x INITIALIZATION

    AnalysisCommandBuilder create_info_list(ALIL::CREATE_EMPTY_INFO_LIST);
    std::string source = create_info_list.reserve_dest_arg_value(this);
    create_info_list.add_empty_source();

    create_info_list.collect_into(commands);

    visit_children(node);

    for (auto initialization : node->get_children()) {
        AnalysisCommandBuilder add_info_to_list(AnalysisLevelInstruction::ADD_TO_INFO_LIST);
        add_info_to_list.add_source_argument(source);
        add_info_to_list.add_source_argument(initialization->get_child(0)->consume_associated_string());
        add_info_to_list.add_source_argument(initialization->get_child(1)->consume_associated_string());
        source = add_info_to_list.reserve_dest_arg_value(this);

        add_info_to_list.collect_into(commands);
    }

    // set the associated string to the final value name that has accumulated all infos thus far
    node->set_associated_string(source); 
}

void ALILConverter::visit_comp_criteria(PNode node) {
    // COMP_CRITERIA -> N x DEFINITION |_| OBJ_SELECT |_| OBJ_REJECT
    AnalysisCommandBuilder create_mask(ALIL::CREATE_MASK);
    std::string global_name_of_first_object = (*what_global_name_for_this_comp_name.begin()).second;
    create_mask.add_source_argument(global_name_of_first_object);
    std::string source;
    {
        NameScope mask_name("MASK", this);
        source = create_mask.reserve_dest_arg_value(this);
    }

    create_mask.collect_into(commands);

    for (PNode criterion : node->get_children()) {
        if (criterion->get_ast_type() == AST_type::DEFINITION) {

            PNode id_node = criterion->get_child(0);
            PNode sum_node = criterion->get_child(1);
            NameScope cand_scope("CAND", id_node, this);

            visit_children(criterion);

            AnalysisCommandBuilder add_def_name(ALIL::ADD_ALIAS);
            add_def_name.add_source_argument(sum_node->consume_associated_string());
            
            std::string local_name = id_node->consume_associated_string();
            {
                NameScope naming_scope(local_name, this);
                std::string global_name = add_def_name.reserve_dest_arg_value(this);
                what_global_name_for_this_comp_name.emplace(local_name,global_name);
            }

            add_def_name.collect_into(commands);

        } else {
            visit(criterion);
            AnalysisCommandBuilder limit_mask(ALIL::LIMIT_MASK);
            limit_mask.add_source_argument(source);
            limit_mask.add_source_argument(criterion->consume_associated_string());
            source = limit_mask.reserve_dest_arg_value(this);

            limit_mask.collect_into(commands);
        }

    }

    node->set_associated_string(source);
}

void ALILConverter::visit_object_criteria(PNode node) {
    // CRITERIA -> N x OBJ_SELECT |_| OBJ_REJECT

    AnalysisCommandBuilder create_mask(ALIL::CREATE_MASK);
    std::string source;
    {
        NameScope mask_name("MASK", this);
        source = create_mask.reserve_dest_arg_value(this);
    }


    create_mask.add_source_argument(what_object_is_this);

    create_mask.collect_into(commands);

    visit_children(node);

    for (auto criterion : node->get_children()) {

        AnalysisCommandBuilder limit_mask(ALIL::LIMIT_MASK);
        limit_mask.add_source_argument(source);
        limit_mask.add_source_argument(criterion->consume_associated_string());
        source = limit_mask.reserve_dest_arg_value(this);

        limit_mask.collect_into(commands);
    }

    node->set_associated_string(source);
}

void ALILConverter::visit_obj_union(PNode node) {
    // UNION -> PARTICLE_LIST
    // PARTICLE_LIST -> n x ID |_| ...
    NameScope union_scope("UNION", this);

    AnalysisCommandBuilder make_empty_union(AnalysisLevelInstruction::CREATE_EMPTY_UNION);
    std::string source = make_empty_union.reserve_dest_arg_value(this);
    make_empty_union.add_empty_source();

    make_empty_union.collect_into(commands);

    PNode particle_list_node = node->get_child(0);

    visit_children(node);

    particle_list_node->get_children();

    auto children = particle_list_node->get_children();
    for (auto particle : children) {
        AnalysisCommandBuilder add_part_to_union(ALIL::ADD_PART_TO_UNION);

        add_part_to_union.add_source_argument(source);
        add_part_to_union.add_source_argument(particle->consume_associated_string());
        
        source = add_part_to_union.reserve_dest_arg_value(this);

        add_part_to_union.collect_into(commands);
    }

    // set the associated string to the final value name that has accumulated all infos thus far
    node->set_associated_string(source);
}

void ALILConverter::visit_obj_sort(PNode node) {
    visit_children(node);

    std::optional<AnalysisCommandBuilder> sorter;

    if (node->get_children().size() >= 3 && node->get_child(2)->get_ast_type() == AST::DESCEND) {
        sorter.emplace(ALIL::OBJ_SORT_DESCEND);
    } else {
        sorter.emplace(ALIL::OBJ_SORT_ASCEND);
    }

    sorter->add_source_argument(node->get_child(0)->consume_associated_string());
    sorter->add_source_argument(node->get_child(1)->consume_associated_string());
    node->set_associated_string(sorter->reserve_dest_arg_value(this));

    sorter->collect_into(commands);
}

void ALILConverter::visit_object_select(PNode node) {
    visit_children(node);
    node->set_associated_string(node->get_child(0)->consume_associated_string());
}

void ALILConverter::visit_object_reject(PNode node) {
    visit_children(node);

    AnalysisCommandBuilder invert_mask_expr(ALIL::EXPR_LOGICAL_NOT);
    invert_mask_expr.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(invert_mask_expr.reserve_dest_arg_value(this));

    invert_mask_expr.collect_into(commands);
 
}


void ALILConverter::visit_region_commands(PNode node) {
    // N x REGION_SELECT |_| REGION_REJECT |_| REGION_USE |_| REGION_WEIGHT |_| REGION_BIN |_| REGION_BINS |_| REGION_HISTO_USE |_| REGION_HISTOGRAM
    
    AnalysisCommandBuilder create_region(ALIL::CREATE_REGION);
    std::string source = create_region.reserve_dest_arg_value(this);
    create_region.add_empty_source();

    create_region.collect_into(commands);

    for (auto command : node->get_children()) {

        command->set_associated_string(source);
        visit(command);
        source = command->consume_associated_string();
    }

    // set the associated string to the final value name that has accumulated all infos thus far
    node->set_associated_string(source); 
}

void ALILConverter::visit_region_select(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommandBuilder select(ALIL::CUT_REGION);
    select.add_source_argument(last_region);
    select.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(select.reserve_dest_arg_value(this)); 

    select.collect_into(commands);
}

void ALILConverter::visit_region_reject(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommandBuilder invert(ALIL::EXPR_LOGICAL_NOT);
    invert.add_source_argument(node->get_child(0)->consume_associated_string());
    std::string dest = invert.reserve_dest_arg_value(this);

    invert.collect_into(commands);

    AnalysisCommandBuilder select(ALIL::CUT_REGION);
    select.add_source_argument(last_region);
    select.add_source_argument(dest);

    node->set_associated_string(select.reserve_dest_arg_value(this));

    select.collect_into(commands);
}

void ALILConverter::visit_region_use(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommandBuilder use(ALIL::MERGE_REGIONS);
    use.add_source_argument(node->get_child(0)->consume_associated_string());
    use.add_source_argument(last_region);

    node->set_associated_string(use.reserve_dest_arg_value(this));

    use.collect_into(commands);
}

void ALILConverter::visit_region_weight(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommandBuilder weight(ALIL::WEIGHT_APPLY);
    weight.add_source_argument(node->get_child(0)->consume_associated_string());
    weight.add_source_argument(node->get_child(1)->consume_associated_string());
    
    node->set_associated_string(weight.reserve_dest_arg_value(this));

    weight.collect_into(commands);
}


void ALILConverter::visit_region_bin(PNode node) {

    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommandBuilder make_bin(ALIL::CREATE_BIN_OF_REGION);
    make_bin.add_source_argument(last_region);

    std::string dest;
    if (node->get_children().size() > 1) {
        dest = node->get_child(0)->consume_associated_string();
        make_bin.add_dest_argument(dest);
        make_bin.add_source_argument(node->get_child(1)->consume_associated_string());
    } else {
        dest = make_bin.reserve_dest_arg_value(this);
        make_bin.add_source_argument(node->get_child(0)->consume_associated_string());
    }

    make_bin.collect_into(commands);

    // add cutflow and eventlist outputs here, we will remove them later if we do not need them.
    AnalysisCommandBuilder do_cutflow(ALIL::DO_CUTFLOW_ON_REGION);
    do_cutflow.add_source_argument(dest);
    do_cutflow.add_empty_dest();

    do_cutflow.collect_into(commands);

    AnalysisCommandBuilder do_eventlist(ALIL::DO_EVENTLIST_ON_REGION);
    do_eventlist.add_source_argument(dest);
    do_eventlist.add_empty_dest();

    do_eventlist.collect_into(commands);

    node->set_associated_string(last_region);
}


void ALILConverter::visit_region_bins(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    std::string discriminant_expression = node->get_child(0)->consume_associated_string();

    std::optional<std::string> last_bound;

    NameScope bins_name_scope("BINS", this);

    for (PNode bound : node->get_child(1)->get_children()) {

        std::string lower_bound;

        if (!last_bound) {

            AnalysisCommandBuilder ge(ALIL::EXPR_GE);
            ge.add_source_argument(discriminant_expression);
            ge.add_source_argument(*last_bound);
            lower_bound = ge.reserve_dest_arg_value(this);
            ge.collect_into(commands);
        } else {
            lower_bound = "true";
        }

        last_bound.emplace(bound->consume_associated_string());

        AnalysisCommandBuilder lt(ALIL::EXPR_LT);
        lt.add_source_argument(discriminant_expression);
        lt.add_source_argument(*last_bound);
        std::string upper_bound = lt.reserve_dest_arg_value(this);
        lt.collect_into(commands);

        AnalysisCommandBuilder both_bounds(ALIL::EXPR_AND);
        both_bounds.add_source_argument(lower_bound);
        both_bounds.add_source_argument(upper_bound);
        std::string final_bound = both_bounds.reserve_dest_arg_value(this);
        both_bounds.collect_into(commands);

        AnalysisCommandBuilder bin(ALIL::CREATE_BIN_OF_REGION);
        bin.add_source_argument(last_region);
        bin.add_source_argument(final_bound);
        std::string bin_name = bin.reserve_dest_arg_value(this); 
        bin.collect_into(commands);

        // add cutflow and eventlist outputs here, we will remove them later if we do not need them.
        AnalysisCommandBuilder do_cutflow(ALIL::DO_CUTFLOW_ON_REGION);
        do_cutflow.add_source_argument(bin_name);
        do_cutflow.add_empty_dest();

        do_cutflow.collect_into(commands);

        AnalysisCommandBuilder do_eventlist(ALIL::DO_EVENTLIST_ON_REGION);
        do_eventlist.add_source_argument(bin_name);
        do_eventlist.add_empty_dest();

        do_eventlist.collect_into(commands);
    }

    node->set_associated_string(last_region);

}

void ALILConverter::visit_region_histo_use(PNode node) {
    std::string this_region = node->consume_associated_string();
    AnalysisCommandBuilder histo_use(ALIL::USE_HIST);
    
    visit_children(node);

    histo_use.add_source_argument(node->get_child(0)->consume_associated_string());
    histo_use.add_source_argument(this_region);
    histo_use.add_empty_dest();

    histo_use.collect_into(commands);

    node->set_associated_string(this_region);
}


void ALILConverter::visit_region_histogram(PNode node) {

    std::string this_region = node->consume_associated_string();

    visit_children(node);

    std::string hist_name = node->consume_associated_string();
    AnalysisCommandBuilder use_hist(ALIL::USE_HIST);

    use_hist.add_source_argument(hist_name);
    use_hist.add_source_argument(this_region);
    use_hist.add_empty_dest();

    use_hist.collect_into(commands);

    node->set_associated_string(this_region);
}


void ALILConverter::visit_histogram(PNode node) {
    
    bool is_2d = node->get_children().size() > 6;

    AnalysisCommandBuilder hist(is_2d ? ALIL::HIST_2D : ALIL::HIST_1D);

    visit_children(node);
    
    std::string name = node->get_child(0)->consume_associated_string();

    hist.add_dest_argument(name);
    hist.add_source_argument(node->get_child(1)->consume_associated_string()); //TODO:check this

    hist.add_source_argument(node->get_child(2)->consume_associated_string());
    hist.add_source_argument(node->get_child(3)->consume_associated_string());
    hist.add_source_argument(node->get_child(4)->consume_associated_string());
    hist.add_source_argument(node->get_child(5)->consume_associated_string());

    if (is_2d) {
        hist.add_source_argument(node->get_child(6)->consume_associated_string());
        hist.add_source_argument(node->get_child(7)->consume_associated_string());
        hist.add_source_argument(node->get_child(8)->consume_associated_string());
        hist.add_source_argument(node->get_child(9)->consume_associated_string());
    }

    node->set_associated_string(name);

    hist.collect_into(commands);

}


void ALILConverter::visit_particle_sum(PNode node) {
    visit_children(node);

    AnalysisCommandBuilder create_empty(ALIL::CREATE_EMPTY_PARTICLE);
    create_empty.add_empty_source();
    std::string last_added_particle = create_empty.reserve_dest_arg_value(this);

    create_empty.collect_into(commands);

    for (PNode part : node->get_children()) {
        bool is_negative = part->get_ast_type() == AST::PARTICLE_NEGATE;
        AnalysisCommandBuilder add_part(is_negative ? ALIL::SUB_PARTICLE : ALIL::ADD_PARTICLE);
        PNode relevant_part_node = is_negative ? part->get_child(0) : part;

        add_part.add_source_argument(last_added_particle);
        add_part.add_source_argument(relevant_part_node->consume_associated_string());
        last_added_particle = add_part.reserve_dest_arg_value(this);

        add_part.collect_into(commands);
    }

    node->set_associated_string(last_added_particle);
}

void ALILConverter::visit_variable_list(PNode node) {
    AnalysisCommandBuilder list_create(ALIL::CREATE_EMPTY_VALUE_LIST);
    list_create.add_empty_source();
    std::string last_list = list_create.reserve_dest_arg_value(this);
    list_create.collect_into(commands);

    visit_children(node);

    for (PNode value : node->get_children()) {
        AnalysisCommandBuilder add_to_list(ALIL::ADD_VALUE_TO_LIST);
        add_to_list.add_source_argument(last_list);
        add_to_list.add_source_argument(value->consume_associated_string());
        last_list = add_to_list.reserve_dest_arg_value(this);

        add_to_list.collect_into(commands);
    }

    node->set_associated_string(last_list);
}

void ALILConverter::visit_expression(PNode node) {
    NameScope expr_scope("EXPR", this);

    visit_children(node);

    node->set_associated_string(node->get_child(0)->consume_associated_string());
}

AnalysisLevelInstruction inst_for_binary(PToken tok) {
    switch (tok->get_token_type()) {
        case TOK::RAISED_TO_POWER:
            return ALIL::EXPR_RAISE;
        case TOK::MULTIPLY:
            return ALIL::EXPR_MULTIPLY;
        case TOK::DIVIDE:
            return ALIL::EXPR_DIVIDE;
        case TOK::PLUS:
            return ALIL::EXPR_ADD;
        case TOK::MINUS:
            return ALIL::EXPR_SUBTRACT;
        case TOK::AMPERSAND:
            return ALIL::EXPR_BITWISE_AND;
        case TOK::PIPE:
            return ALIL::EXPR_BITWISE_OR;
        case TOK::EQ: case TOK::ASSIGN:
            return ALIL::EXPR_EQ;
        case TOK::LT:
            return ALIL::EXPR_LT;
        case TOK::GT:
            return ALIL::EXPR_GT;
        case TOK::LE:
            return ALIL::EXPR_LE;
        case TOK::GE:
            return ALIL::EXPR_GE;
        case TOK::AND:
            return ALIL::EXPR_ADD;
        case TOK::OR:
            return ALIL::EXPR_OR;
        default:
            assert(false);
            return ALIL::CONVERSION_ERROR;
    }
}

bool is_a_comparison(PToken tok) {
    switch (tok->get_token_type()) {
        case TOK::LT: case TOK::GT: case TOK::LE: case TOK::GE:
            return true;
        default:
            return false;
    }
}

AnalysisLevelInstruction inclusive_exclusive_determination(PToken tok1, PToken tok2) {

    bool lhs_inclusive = tok1->get_token_type() == TOK::GE || tok1->get_token_type() == TOK::LE;
    bool rhs_inclusive = tok2->get_token_type() == TOK::GE || tok2->get_token_type() == TOK::LE;

    if (lhs_inclusive && rhs_inclusive) {
        return ALIL::EXPR_WITHIN;
    } else if (lhs_inclusive) {
        return ALIL::EXPR_WITHIN_RIGHT_EXCLUSIVE;
    } else if (rhs_inclusive) {
        return ALIL::EXPR_WITHIN_LEFT_EXCLUSIVE;
    } else {
        return ALIL::EXPR_WITHIN_EXCLUSIVE;
    }
}

void ALILConverter::visit_operator_terminal(PNode node) {
    
    switch (node->get_token()->get_token_type()) {
        case TOK::ARROW_INDEX:
        {   
            visit_children(node);
            // directly combine the names, since this is actually just sugar.

            std::stringstream arrow;
            arrow << node->get_child(0)->consume_associated_string() << "->" << node->get_child(1)->consume_associated_string();
            node->set_associated_string(arrow.str());
        } break;
        case TOK::DOT_INDEX:
        {
            if (node->get_child(1)->get_ast_type() == AST::VARYING_TERMINAL) {
                // this implies that we have a user function on the rhs.
                AnalysisCommandBuilder user_func(ALIL::FUNC_NAMED);
                visit_children(node);
                user_func.add_source_argument(node->get_child(0)->consume_associated_string());
                user_func.add_source_argument(node->get_child(1)->consume_associated_string());

                std::string dest = user_func.reserve_dest_arg_value(this);

                user_func.collect_into(commands);

                node->set_associated_string(dest);

            } else if (node->get_child(1)->get_ast_type() == AST::BUILTIN_FUNC_TERMINAL) {
                // we handle this in logic for builtin funcs.
                visit_children_after_index(node, 0);
                node->set_associated_string(node->get_child(1)->consume_associated_string());
            } else {
                raise_analysis_conversion_exception("Invalid token given after a dot index", node->get_token());
            }
        } break;

        case TOK::LT: case TOK::GT: case TOK::LE: case TOK::GE:
        {
            bool lhs_is_comparison = is_a_comparison(node->get_child(0)->get_token());
            bool rhs_is_comparison = is_a_comparison(node->get_child(1)->get_token());

            if (lhs_is_comparison && rhs_is_comparison) {
                raise_analysis_conversion_exception("Invalid chained comparison interval, too many comparisons in a row", node->get_child(1)->get_token());
                return;
            } else if (lhs_is_comparison || rhs_is_comparison) {
                PNode left_comparator = lhs_is_comparison ? node->get_child(0) : node;
                PNode right_comparator = lhs_is_comparison ? node : node->get_child(1);
                
                PNode left_bound = left_comparator->get_child(0);
                PNode right_bound = right_comparator->get_child(1);

                PNode discriminant = lhs_is_comparison ? left_comparator->get_child(1) : right_comparator->get_child(0);

                visit(left_bound);
                visit(right_bound);
                visit(discriminant);

                AnalysisCommandBuilder within(inclusive_exclusive_determination(left_comparator->get_token(), right_comparator->get_token()));
                within.add_source_argument(discriminant->consume_associated_string());
                within.add_source_argument(left_bound->consume_associated_string());
                within.add_source_argument(right_bound->consume_associated_string());
                node->set_associated_string(within.reserve_dest_arg_value(this));
                within.collect_into(commands);
                return;
            }
            // intentionally falls through
        } [[fallthrough]];

        default:
        {
            visit_children(node);
            AnalysisCommandBuilder binary_op(inst_for_binary(node->get_token()));
            binary_op.add_source_argument(node->get_child(0)->consume_associated_string());
            binary_op.add_source_argument(node->get_child(1)->consume_associated_string());
            node->set_associated_string(binary_op.reserve_dest_arg_value(this));
            binary_op.collect_into(commands);
        }
    
    }
}

void ALILConverter::visit_index_operator(PNode node) {

    visit_children(node);
    std::string value_to_index = node->get_child(0)->consume_associated_string();

    PNode bound_1;
    PNode bound_2;

    bool two_bound = false;
    AnalysisLevelInstruction inst;


    PNode index_node = node->get_child(1);

    if (index_node->get_children().size() > 1) {

        PNode lower_bound = index_node->get_child(0);
        PNode upper_bound = index_node->get_child(1);

        if (lower_bound->get_ast_type() == AST::UNBOUNDED && upper_bound->get_ast_type() == AST::UNBOUNDED) {
            assert(false);
        } else if (lower_bound->get_ast_type() == AST::UNBOUNDED) {
            inst = ALIL::EXPR_INDEX_UNTIL;
            bound_1 = upper_bound;
        } else if (upper_bound->get_ast_type() == AST::UNBOUNDED) {
            inst = ALIL::EXPR_INDEX_FROM;
            bound_1 = lower_bound;
        } else {
            inst = ALIL::EXPR_INDEX_RANGE;
            two_bound = true;
            bound_1 = lower_bound;
            bound_2 = upper_bound;
        }
    } else {
        inst = ALIL::EXPR_INDEX;
        bound_1 = index_node->get_child(0);
    }

    AnalysisCommandBuilder index_command(inst);
    index_command.add_source_argument(value_to_index);
    index_command.add_source_argument(bound_1->consume_associated_string());
    if (two_bound) index_command.add_source_argument(bound_2->consume_associated_string());

    node->set_associated_string(index_command.reserve_dest_arg_value(this));
    index_command.collect_into(commands);
}

void ALILConverter::visit_if_statement(PNode node) {
    visit_children(node);

    std::string discriminant = node->get_child(0)->consume_associated_string();
    std::string result_if_true = node->get_child(1)->consume_associated_string();
    std::string result_if_false = node->get_child(2)->consume_associated_string();

    AnalysisCommandBuilder if_statement(ALIL::EXPR_IF_TERNARY);
    if_statement.add_source_argument(discriminant);
    if_statement.add_source_argument(result_if_false);
    if_statement.add_source_argument(result_if_false);
    node->set_associated_string(if_statement.reserve_dest_arg_value(this));

    if_statement.collect_into(commands);
}

void ALILConverter::visit_within_statement(PNode node) {
    visit_children(node);
    AnalysisCommandBuilder within(ALIL::EXPR_WITHIN);
    within.add_source_argument(node->get_child(0)->consume_associated_string());
    within.add_source_argument(node->get_child(1)->consume_associated_string());
    within.add_source_argument(node->get_child(2)->consume_associated_string());
    node->set_associated_string(within.reserve_dest_arg_value(this));

    within.collect_into(commands);
}

void ALILConverter::visit_outside_statement(PNode node) {
    visit_children(node);
    AnalysisCommandBuilder within(ALIL::EXPR_WITHIN);
    within.add_source_argument(node->get_child(0)->consume_associated_string());
    within.add_source_argument(node->get_child(1)->consume_associated_string());
    within.add_source_argument(node->get_child(2)->consume_associated_string());
    std::string within_result = within.reserve_dest_arg_value(this);

    within.collect_into(commands);

    AnalysisCommandBuilder invert_interval(ALIL::EXPR_LOGICAL_NOT);
    invert_interval.add_source_argument(within_result);
    node->set_associated_string(invert_interval.reserve_dest_arg_value(this));

    invert_interval.collect_into(commands);
}

void ALILConverter::visit_sort_expression(PNode node) {
    bool is_ascending = true;
    if (node->get_children().size() > 1) {
        if (node->get_child(1)->get_ast_type() == AST::DESCEND) is_ascending = false;
    }
    AnalysisCommandBuilder sort(is_ascending ? ALIL::FUNC_SORT_ASCEND : ALIL::FUNC_SORT_DESCEND);

    visit_children(node);

    sort.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(sort.reserve_dest_arg_value(this));

    sort.collect_into(commands);
}



void ALILConverter::visit_min_expression(PNode node) {

    bool is_min = node->get_ast_type() == AST::MIN_EXPRESSION;

    PNode list_node = node->get_child(0);

    PNode first = list_node->get_child(0);
    visit(first);

    AnalysisCommandBuilder first_min(is_min ? ALIL::FUNC_MIN_OF_LIST : ALIL::FUNC_MAX_OF_LIST);
    first_min.add_source_argument(first->consume_associated_string());
    std::string last_val = first_min.reserve_dest_arg_value(this);

    first_min.collect_into(commands);

    bool is_first = true;

    for (PNode val : list_node->get_children()) {
        if (is_first) {
            is_first = false;
            continue;
        }

        visit(val);

        AnalysisCommandBuilder min_per_se(is_min ? ALIL::FUNC_MIN_OF_LIST : ALIL::FUNC_MAX_OF_LIST);
        min_per_se.add_source_argument(val->consume_associated_string());
        std::string min_of_this_val = min_per_se.reserve_dest_arg_value(this);

        min_per_se.collect_into(commands);

        AnalysisCommandBuilder min_with_prev(is_min ? ALIL::FUNC_MIN_OF_PAIR : ALIL::FUNC_MAX_OF_PAIR);
        min_with_prev.add_source_argument(last_val);
        min_with_prev.add_source_argument(min_of_this_val);

        last_val = min_with_prev.reserve_dest_arg_value(this);
        
        min_with_prev.collect_into(commands);
    }

    node->set_associated_string(last_val);

}


void ALILConverter::visit_max_expression(PNode node) {
    // we handle the max case in the other visitor.
    visit_min_expression(node);
}

void ALILConverter::visit_negate(PNode node) {
    AnalysisCommandBuilder negate(ALIL::EXPR_NEGATE);
    negate.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(negate.reserve_dest_arg_value(this));
    
    negate.collect_into(commands);
}

void ALILConverter::visit_l_not(PNode node) {
    AnalysisCommandBuilder l_not(ALIL::EXPR_LOGICAL_NOT);
    l_not.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(l_not.reserve_dest_arg_value(this));

    l_not.collect_into(commands);
}

AnalysisLevelInstruction inst_for_builtin(PToken tok) {
    switch (tok->get_token_type()) {
        case TOK::ANYOF: 
            return ALIL::FUNC_ANYOF;
        case TOK::ALLOF: 
            return ALIL::FUNC_ALLOF;
        case TOK::SQRT: 
            return ALIL::FUNC_SQRT;
        case TOK::ABS: 
            return ALIL::FUNC_ABS;
        case TOK::COS:  
            return ALIL::FUNC_COS;
        case TOK::SIN: 
            return ALIL::FUNC_SIN;
        case TOK::TAN: 
            return ALIL::FUNC_TAN;
        case TOK::SINH: 
            return ALIL::FUNC_SINH;
        case TOK::COSH: 
            return ALIL::FUNC_COSH;
        case TOK::TANH: 
            return ALIL::FUNC_TANH;
        case TOK::EXP: 
            return ALIL::FUNC_EXP;
        case TOK::LOG: 
            return ALIL::FUNC_LOG;
        case TOK::AVE: 
            return ALIL::FUNC_AVE;
        case TOK::SUM:
            return ALIL::FUNC_SUM;

        case TOK::LETTER_E: 
            return ALIL::FUNC_ENERGY;
        case TOK::LETTER_P: case TOK::PT:
            return ALIL::FUNC_PT;
        case TOK::LETTER_M: case TOK::MASS:
            return ALIL::FUNC_MASS;
        case TOK::LETTER_Q: case TOK::CHARGE:
            return ALIL::FUNC_CHARGE;
        case TOK::PHI: 
            return ALIL::FUNC_PHI;
        case TOK::ETA: 
            return ALIL::FUNC_ETA;
        case TOK::NUMOF: 
            return ALIL::FUNC_SIZE;
        case TOK::DR: 
            return ALIL::FUNC_DR;
        case TOK::DPHI: 
            return ALIL::FUNC_DPHI;
        case TOK::DETA:
            return ALIL::FUNC_DETA;
        case TOK::DR_HADAMARD: 
            return ALIL::FUNC_DR_HADAMARD;
        case TOK::DPHI_HADAMARD: 
            return ALIL::FUNC_DPHI_HADAMARD;
        case TOK::DETA_HADAMARD:
            return ALIL::FUNC_DETA_HADAMARD;
        case TOK::DISTINCT:
            return ALIL::FUNC_DISTINCT;

        default:
            assert(false);
            return ALIL::CONVERSION_ERROR;
    }
}

void ALILConverter::visit_builtin_func_terminal(PNode node) {
    AnalysisCommandBuilder func(inst_for_builtin(node->get_token()));

    PNode input_node;
    if (node->get_parent().lock()->get_ast_type() == AST::OPERATOR_TERMINAL && node->get_parent().lock()->get_token()->get_token_type() == TOK::DOT_INDEX) {
        input_node = node->get_parent().lock();
    } else {
        input_node = node->get_child(0);
    }

    switch (node->get_token()->get_token_type()) {
        case CASE_BUILT_IN_PARTICLE_FUN_ONE_ARG:
        {
            PNode particle_node = input_node->get_child(0);
            visit(particle_node);
            func.add_source_argument(particle_node->consume_associated_string());
        } break;
        case CASE_BUILT_IN_PARTICLE_FUN_TWO_ARG:
        {
            PNode particle_node_1 = input_node->get_child(0);
            PNode particle_node_2 = input_node->get_child(1);
            visit_children(input_node);
            func.add_source_argument(particle_node_1->consume_associated_string());
            func.add_source_argument(particle_node_2->consume_associated_string());
        }   break;
        case CASE_BUILT_IN_MATH_FUN:
        {
            func.add_source_argument(input_node->consume_associated_string());
        } break;
        default:
            assert(false);
    }

    node->set_associated_string(func.reserve_dest_arg_value(this));

    func.collect_into(commands);
}

void ALILConverter::visit_user_function(PNode node) {
    visit_children_before_index(node, 1);

    std::string func_name = node->get_child(0)->consume_associated_string();

    AnalysisCommandBuilder user_func(ALIL::FUNC_NAMED);

    if (node->get_child(1)->get_children().size() > 1) {
        visit_children_after_index(node, 1);
        user_func.add_source_argument(node->get_child(1)->consume_associated_string());
    } else {
        PNode single_argument = node->get_child(1)->get_child(0);
        visit(single_argument);
        user_func.add_source_argument(single_argument->consume_associated_string());
    }

    user_func.add_source_argument(func_name);

    node->set_associated_string(user_func.reserve_dest_arg_value(this));
    
    user_func.collect_into(commands);
}

void ALILConverter::visit_varying_terminal(PNode node) {
    std::string target_lexeme = node->get_token()->get_lexeme();

    // if this literal has an associated composite global name, change it here.
    if (what_global_name_for_this_comp_name.contains(target_lexeme)) {
        target_lexeme = what_global_name_for_this_comp_name[target_lexeme];
    }
    node->set_associated_string(target_lexeme);
}

void ALILConverter::visit_true_literal(PNode node) {
    node->set_associated_string("true");
}

void ALILConverter::visit_false_literal(PNode node) {
    node->set_associated_string("false");
}

void ALILConverter::visit_this_node(PNode node) {
    if (what_object_is_this == "") raise_analysis_conversion_exception("Used this in a context where it is not meaningful - \"this\" only means anything in an object block", node->get_token());
    node->set_associated_string(what_object_is_this);
}

void ALILConverter::clean_command_list() {

    bool do_last_cutflow = false;
    bool do_every_cutflow = false;

    bool do_last_eventlist = false;
    bool do_every_eventlist = false;

    if (config.get_argument("cutflow") == "all") do_every_cutflow = true;
    if (config.get_argument("cutflow") == "last") do_last_cutflow = true;

    if (config.get_argument("eventlist") == "all") do_every_eventlist = true;
    if (config.get_argument("eventlist") == "last") do_last_eventlist = true;

    auto command_list = commands.get_commands();

    ALILCollection new_collection;

    // backwards iteration pass
    for (auto command : command_list | std::views::reverse) {
        if (command.get_instruction() == ALIL::DO_CUTFLOW_ON_REGION && !do_every_cutflow) {
            if (do_last_cutflow) do_last_cutflow = false;
            else continue;
        } else if (command.get_instruction() == ALIL::DO_EVENTLIST_ON_REGION && !do_every_eventlist) {
            if (do_last_eventlist) do_last_eventlist = false;
            else continue;
        }

        AnalysisCommandBuilder new_command(command);
        new_command.collect_into_reverse(new_collection);
    }

    commands = new_collection;

    
}

void ALILConverter::visitation(PNode root) {
    visit(root);
    clean_command_list();
}

void ALILConverter::print_commands() {
    commands.print_collected_commands();
}

// bool ALILConverter::clear_to_next() {
//     if (iter_command >= command_list.size()) return false;
//     return true;
// }

// AnalysisCommandBuilder ALILConverter::next_command() {
//     return command_list[iter_command++];
// }

ALILConverter::ALILConverter(Config &conf): highest_var_val(0), iter_command(0),  config(conf){}

ALILCollection &ALILConverter::get_commands() {
    return commands;
}

ALILToFrameworkCompiler::ALILToFrameworkCompiler(ALILConverter *alil_in, Config &conf): alil(alil_in), config(conf) {}

#define VISIT_DISPATCH(ENUM, NAME) \
    case ALIL::ENUM: return convert_##NAME(command); \

std::string ALILToFrameworkCompiler::command_convert(const AnalysisCommand &command) {
    switch (command.get_instruction()) {
        ALIL_INSTRUCTION_LIST(VISIT_DISPATCH)
    }
    assert(false);
    return "";
}

#undef VISIT_DISPATCH