#include "alil_converter.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <memory>
#include <optional>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok) MAKE_UNCONSUMED: instruction(inst), has_been_collected(false), dest_declared(false) , source_declared(false), source_token(tok){

}

AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst) MAKE_UNCONSUMED: instruction(inst), has_been_collected(false), dest_declared(false), source_declared(false) {

}

AnalysisCommand::AnalysisCommand(const AnalysisCommand &other) MAKE_UNCONSUMED: instruction(other.instruction), has_been_collected(other.has_been_collected), dest_declared(other.dest_declared), source_declared(other.source_declared), dest_argument(other.dest_argument), source_arguments(other.source_arguments), source_token(other.source_token){

}

AnalysisCommand::~AnalysisCommand() CALLABLE_CONSUMED{
    assert(has_been_collected);
}

void AnalysisCommand::add_dest_argument(std::string arg) CALLABLE_UNCONSUMED{
    assert(!dest_argument);
    dest_argument = arg;
    dest_declared = true;
}

void AnalysisCommand::add_source_argument(std::string arg) CALLABLE_UNCONSUMED{
    source_arguments.push_back(arg);
    source_declared = true;
}


void AnalysisCommand::add_empty_source() CALLABLE_UNCONSUMED {
    source_declared = true;
}
void AnalysisCommand::add_empty_dest() CALLABLE_UNCONSUMED {
    dest_declared = true;
}

AnalysisLevelInstruction AnalysisCommand::get_instruction() {
    return instruction;
}
std::string AnalysisCommand::get_argument(int pos) CALLABLE_CONSUMED{
    assert(pos >= 0);
    
    if (pos == 0) {
        if (dest_argument) return *dest_argument;
        assert(source_arguments.size() >= 1);
        return source_arguments[0];
    } else if (dest_argument) {
        int pos_in_vec = pos - 1;
        assert(pos_in_vec < source_arguments.size());
        return source_arguments[pos_in_vec];
    } else {
        assert(pos < source_arguments.size());
        return source_arguments[pos];
    }

}

bool AnalysisCommand::has_dest_argument() CALLABLE_CONSUMED{
    return dest_argument.has_value();
}

std::string AnalysisCommand::get_dest_argument() CALLABLE_CONSUMED{
    assert(dest_argument);
    return *dest_argument;
}

std::string AnalysisCommand::get_source_argument(int pos) CALLABLE_CONSUMED{
    assert(pos < source_arguments.size()); 
    return source_arguments[pos];
}

int AnalysisCommand::get_num_arguments() CALLABLE_CONSUMED{
    return static_cast<int>(dest_argument.has_value()) + source_arguments.size();
}

std::string AnalysisCommand::reserve_dest_arg_value(ALILConverter *alil_conv) CALLABLE_UNCONSUMED {
    std::string dest_val = alil_conv->reserve_scoped_value_name();
    add_dest_argument(dest_val);
    return dest_val;
}

void AnalysisCommand::collect_into(ALILCollection &target) CALLABLE_UNCONSUMED __attribute__((set_typestate(consumed))){
    target.collect_command(*this);
    mark_collected();
}

void AnalysisCommand::mark_collected() CALLABLE_UNCONSUMED __attribute__((set_typestate(consumed))){
    has_been_collected = true;
}


ALILCollection::ALILCollection() {

}

void ALILCollection::collect_command(AnalysisCommand in __attribute__((param_typestate(unconsumed)))) {
    assert(in.source_declared && in.dest_declared);
    command_list.push_back(in);
}


// std::string AnalysisCommand::instruction_to_text(AnalysisLevelInstruction inst) {

//     switch (inst) {
//         using enum ALIL;
//         case CREATE_REGION:
//             return "CREATE_REGION";
//         case MERGE_REGIONS:
//             return "MERGE_REGIONS";
//         case CUT_REGION:
//             return "CUT_REGION";
//         case ADD_ALIAS:
//             return "ADD_ALIAS";
//         case ADD_EXTERNAL:
//             return "ADD_EXTERNAL";
//         case ADD_EXTERN_ATTR:
//             return "ADD_EXTERN_ATTR";
//         case ADD_CORRECTIONLIB:
//             return "ADD_CORRECTIONLIB";
//         case CREATE_MASK:
//             return "CREATE_MASK";
//         case LIMIT_MASK:
//             return "LIMIT_MASK";
//         case APPLY_MASK:
//             return "APPLY_MASK";

//         case CREATE_HIST_LIST:
//             return "CREATE_HIST_LIST";
//         case ADD_HIST_TO_LIST:
//             return "ADD_HIST_TO_LIST";
//         case USE_HIST:
//             return "USE_HIST";
//         case USE_HIST_LIST:
//             return "USE_HIST_LIST";
//         case HIST_1D:
//             return "HIST_1D";
//         case HIST_2D:
//             return "HIST_2D";
//         case CREATE_BIN:
//             return "CREATE_BIN";

//         case DO_CUTFLOW_ON_REGION:
//             return "DO_CUTFLOW_ON_REGION";
//         case DO_EVENTLIST_ON_REGION:
//             return "DO_EVENTLIST_ON_REGION";

//         case CREATE_TABLE:
//             return "CREATE_TABLE";
//         case CREATE_TABLE_LOWER_BOUNDS:
//             return "CREATE_TABLE_LOWER_BOUNDS";
//         case  CREATE_TABLE_UPPER_BOUNDS:
//             return "CREATE_TABLE_UPPER_BOUNDS";
//         case CREATE_TABLE_VALUE:
//             return "CREATE_TABLE_VALUE";
//         case APPEND_TO_TABLE:
//             return "APPEND_TO_TABLE";
//         case FINISH_TABLE:
//             return "FINISH_TABLE";

//         case SORT_ASCEND:
//             return "SORT_ASCEND";
//         case SORT_DESCEND:
//             return "SORT_DESCEND";
//         case WEIGHT_APPLY:
//             return "WEIGHT_APPLY";

//         case BEGIN_EXPRESSION:
//             return "BEGIN_EXPRESSION";
//         case END_EXPRESSION:
//             return "END_EXPRESSION";
//         case BEGIN_IF:
//             return "BEGIN_IF";
//         case END_IF:
//             return "END_IF";
//         case EXPR_RAISE:
//             return "EXPR_RAISE";
//         case EXPR_MULTIPLY:
//             return "EXPR_MULTIPLY";
//         case EXPR_DIVIDE:
//             return "EXPR_DIVIDE";
//         case EXPR_ADD:
//             return "EXPR_ADD";
//         case EXPR_SUBTRACT:
//             return "EXPR_SUBTRACT";
//         case EXPR_LT:
//             return "EXPR_LT";
//         case EXPR_LE:
//             return "EXPR_LE";
//         case EXPR_GT:
//             return "EXPR_GT";
//         case EXPR_GE:
//             return "EXPR_GE";
//         case EXPR_EQ:
//             return "EXPR_EQ";
//         case EXPR_NE:
//             return "EXPR_NE";
//         case EXPR_AMPERSAND:
//             return "EXPR_AMPERSAND";
//         case EXPR_PIPE:
//             return "EXPR_PIPE";
//         case EXPR_AND:
//             return "EXPR_AND";
//         case EXPR_OR:
//             return "EXPR_OR";
//         case EXPR_WITHIN:
//             return "EXPR_WITHIN";
//         case EXPR_OUTSIDE:
//             return "EXPR_OUTSIDE";
//         case EXPR_NEGATE:
//             return "EXPR_NEGATE";
//         case EXPR_LOGICAL_NOT:
//             return "EXPR_LOGICAL_NOT";
//         case EXPR_INDEX:
//             return "EXPR_INDEX";

//         case FUNC_BTAG:
//             return "FUNC_BTAG"; 
//         case FUNC_PT:
//             return "FUNC_PT"; 
//         case FUNC_ETA:
//             return "FUNC_ETA"; 
//         case FUNC_PHI:
//             return "FUNC_PHI"; 
//         case FUNC_MASS:
//             return "FUNC_M"; 
//         case FUNC_MSOFTDROP:
//             return "FUNC_MSOFTDROP";
//         case FUNC_ENERGY:
//             return "FUNC_E"; 
//         case MAKE_EMPTY_PARTICLE:
//             return "MAKE_EMPTY_PARTICLE"; 
//         case ADD_PART_ELECTRON:
//             return "ADD_PART_ELECTRON"; 
//         case ADD_PART_MUON:
//             return "ADD_PART_MUON"; 
//         case ADD_PART_TAU:
//             return "ADD_PART_TAU"; 
//         case ADD_PART_TRACK:
//             return "ADD_PART_TRACK"; 
//         case ADD_PART_PHOTON:
//             return "ADD_PART_PHOTON"; 
//         case ADD_PART_QGJET:
//             return "ADD_PART_QGJET"; 
//         case ADD_PART_METLV:
//             return "ADD_PART_METLV"; 
//         case ADD_PART_GEN:
//             return "ADD_PART_GEN"; 
//         case ADD_PART_JET:
//             return "ADD_PART_JET"; 
//         case ADD_PART_FJET:
//             return "ADD_PART_FJET"; 
//         case ADD_PART_NAMED:
//             return "ADD_PART_NAMED"; 
//         case SUB_PART_ELECTRON:
//             return "SUB_PART_ELECTRON"; 
//         case SUB_PART_MUON:
//             return "SUB_PART_MUON"; 
//         case SUB_PART_TAU:
//             return "SUB_PART_TAU"; 
//         case SUB_PART_TRACK:
//             return "SUB_PART_TRACK"; 
//         case SUB_PART_PHOTON:
//             return "SUB_PART_PHOTON"; 
//         case SUB_PART_QGJET:
//             return "SUB_PART_QGJET"; 
//         case SUB_PART_METLV:
//             return "SUB_PART_METLV"; 
//         case SUB_PART_GEN:
//             return "SUB_PART_GEN"; 
//         case SUB_PART_JET:
//             return "SUB_PART_JET"; 
//         case SUB_PART_FJET:
//             return "SUB_PART_FJET"; 
//         case SUB_PART_NAMED:
//             return "SUB_PART_NAMED";
//         case FUNC_ANYOF:
//             return "FUNC_ANYOF";
//         case FUNC_ALLOF:
//             return "FUNC_ALLOF";
//         case FUNC_SQRT:
//             return "FUNC_SQRT";
//         case FUNC_ABS:
//             return "FUNC_ABS";
//         case FUNC_COS:
//             return "FUNC_COS";
//         case FUNC_SIN:
//             return "FUNC_SIN";
//         case FUNC_TAN:
//             return "FUNC_TAN";
//         case FUNC_SINH:
//             return "FUNC_SINH";
//         case FUNC_COSH:
//             return "FUNC_COSH";
//         case FUNC_TANH:
//             return "FUNC_TANH";
//         case FUNC_EXP:
//             return "FUNC_EXP";
//         case FUNC_LOG:
//             return "FUNC_LOG";
//         case FUNC_AVE:
//             return "FUNC_AVE";
//         case FUNC_SUM:
//             return "FUNC_SUM";
//         case FUNC_MIN:
//             return "FUNC_MIN";
//         case FUNC_ANYOCCURRENCES:
//             return "FUNC_ANYOCCURANCES";
//         case FUNC_FIRST:
//             return "FUNC_FIRST";
//         case FUNC_SECOND:
//             return "FUNC_SECOND";
//         case FUNC_MAX:
//             return "FUNC_MAX";
//         case FUNC_MAX_LIST:
//             return "FUNC_MAX";
//         case FUNC_MIN_LIST:
//             return "FUNC_MIN_LIST";
//         case FUNC_SORT_ASCEND:
//             return "FUNC_SORT_ASCEND";
//         case FUNC_SORT_DESCEND:
//             return "FUNC_SORT_DESCEND";
//         case FUNC_NAMED:
//             return "FUNC_NAMED";
//         case MAKE_EMPTY_UNION:
//             return "MAKE_EMPTY_UNION";
//         case ADD_NAMED_TO_UNION:
//             return "ADD_NAMED_TO_UNION";
//         case ADD_ELECTRON_TO_UNION:
//             return "ADD_ELECTRON_TO_UNION";
//         case ADD_MUON_TO_UNION:
//             return "ADD_MUON_TO_UNION";
//         case ADD_TAU_TO_UNION:
//             return "ADD_TAU_TO_UNION";
//         case ADD_TRACK_TO_UNION:
//             return "ADD_TRACK_TO_UNION"; 
//         case ADD_PHOTON_TO_UNION:
//             return "ADD_PHOTON_TO_UNION"; 
//         case ADD_QGJET_TO_UNION:
//             return "ADD_QGJET_TO_UNION"; 
//         case ADD_METLV_TO_UNION:
//             return "ADD_METLV_TO_UNION"; 
//         case ADD_GEN_TO_UNION:
//             return "ADD_GEN_TO_UNION"; 
//         case ADD_JET_TO_UNION:
//             return "ADD_JET_TO_UNION"; 
//         case ADD_FJET_TO_UNION:
//             return "ADD_FJET_TO_UNION"; 

//         case MAKE_EMPTY_COMB:
//             return "MAKE_EMPTY_COMB";
//         case ADD_NAMED_TO_COMB:
//             return "ADD_NAMED_TO_COMB";
//         case ADD_ELECTRON_TO_COMB:
//             return "ADD_ELECTRON_TO_COMB";
//         case ADD_MUON_TO_COMB:
//             return "ADD_MUON_TO_COMB";
//         case ADD_TAU_TO_COMB:
//             return "ADD_TAU_TO_COMB";
//         case ADD_TRACK_TO_COMB:
//             return "ADD_TRACK_TO_COMB"; 
//         case ADD_PHOTON_TO_COMB:
//             return "ADD_PHOTON_TO_COMB"; 
//         case ADD_QGJET_TO_COMB:
//             return "ADD_QGJET_TO_COMB"; 
//         case ADD_METLV_TO_COMB:
//             return "ADD_METLV_TO_COMB"; 
//         case ADD_GEN_TO_COMB:
//             return "ADD_GEN_TO_COMB"; 
//         case ADD_JET_TO_COMB:
//             return "ADD_JET_TO_COMB"; 
//         case ADD_FJET_TO_COMB:
//             return "ADD_FJET_TO_COMB"; 

//         case FUNC_FLAVOR:
//             return "FUNC_FLAVOR";
//         case FUNC_CONSTITUENTS:
//             return "FUNC_CONSTITUENTS";
//         case FUNC_PDG_ID:
//             return "FUNC_PDG_ID";
//         case FUNC_JET_ID:
//             return "FUNC_JET_ID";
//         case FUNC_TAUTAG:
//             return "FUNC_TAUTAG";
//         case FUNC_CTAG:
//             return "FUNC_CTAG";
//         case FUNC_DXY:
//             return "FUNC_DXY";
//         case FUNC_DZ:
//             return "FUNC_DZ";
//         case FUNC_IS_TIGHT:
//             return "FUNC_IS_TIGHT";
//         case FUNC_IS_MEDIUM:
//             return "FUNC_IS_MEDIUM";
//         case FUNC_IS_LOOSE:
//             return "FUNC_IS_LOOSE";
//         case FUNC_THETA:
//             return "FUNC_THETA";
//         case FUNC_ABS_ISO:
//             return "FUNC_ABS_ISO";
//         case FUNC_MINI_ISO:
//             return "FUNC_MINI_ISO";
//         case FUNC_DR:
//             return "FUNC_DR";
//         case FUNC_DPHI:
//             return "FUNC_DPHI";
//         case FUNC_DETA:
//             return "FUNC_DETA";
//         case FUNC_DR_HADAMARD:
//             return "FUNC_DR_HADAMARD";
//         case FUNC_DPHI_HADAMARD:
//             return "FUNC_DPHI_HADAMARD";
//         case FUNC_DETA_HADAMARD:
//             return "FUNC_DETA_HADAMARD";
//         case FUNC_SIZE:
//             return "FUNC_SIZE";
//         case FUNC_GEN_PART_IDX:
//             return "FUNC_GEN_PART_IDX";
//         case FUNC_CHARGE:
//             return "FUNC_CHARGE";
//         case FUNC_RAPIDITY:
//             return "FUNC_RAPIDITY";

//         case FUNC_DISTINCT:
//             return "FUNC_DISTINCT";

//         case EXPR_WITHIN_EXCLUSIVE:
//             return "EXPR_WITHIN_EXCLUSIVE";
//         case EXPR_WITHIN_LEFT_EXCLUSIVE:
//             return "EXPR_WITHIN_LEFT_EXCLUSIVE";
//         case EXPR_WITHIN_RIGHT_EXCLUSIVE:
//             return "EXPR_WITHIN_RIGHT_EXCLUSIVE";
//         case NAME_ELEMENT_OF_COMB:
//             return "NAME_ELEMENT_OF_COMB";
//         case MAKE_EMPTY_DISJOINT:
//             return "MAKE_EMPTY_DISJOINT";
//         case ADD_NAMED_TO_DISJOINT:
//             return "ADD_NAMED_TO_DISJOINT";
//         case ADD_ELECTRON_TO_DISJOINT:
//             return "ADD_ELECTRON_TO_DISJOINT";
//         case ADD_MUON_TO_DISJOINT:
//             return "ADD_MUON_TO_DISJOINT";
//         case ADD_TAU_TO_DISJOINT:
//             return "ADD_TAU_TO_DISJOINT";
//         case ADD_TRACK_TO_DISJOINT:
//             return "ADD_TRACK_TO_DISJOINT";
//         case ADD_PHOTON_TO_DISJOINT:
//             return "ADD_PHOTON_TO_DISJOINT";
//         case ADD_QGJET_TO_DISJOINT:
//             return "ADD_QGJET_TO_DISJOINT";
//         case ADD_METLV_TO_DISJOINT:
//             return "ADD_METLV_TO_DISJOINT";
//         case ADD_GEN_TO_DISJOINT:
//             return "ADD_GEN_TO_DISJOINT";
//         case ADD_JET_TO_DISJOINT:
//             return "ADD_JET_TO_DISJOINT";
//         case ADD_FJET_TO_DISJOINT:
//             return "ADD_FJET_TO_DISJOINT";
//         case NAME_ELEMENT_OF_DISJOINT:
//             return "NAME_ELEMENT_OF_DISJOINT";
//           break;
//         }
// }

// void AnalysisCommand::print_instruction(int width_of_dest, int width_of_inst) CALLABLE_CONSUMED {
    
//     if (instruction == MAKE_EMPTY_PARTICLE || instruction == MAKE_EMPTY_UNION || instruction == MAKE_EMPTY_COMB || instruction == CREATE_REGION || instruction == CREATE_MASK) std::cout << std::endl;


//     std::cout << std::left << std::setw(width_of_dest) << (std::stringstream() << "(" << (has_dest_argument_yet ? dest_argument : "") << ") ").str() << std::left << std::setw(2) << " <- ";

//     std::cout << std::left << std::setw(width_of_inst) << instruction_to_text(instruction);

//     std::stringstream args;

//     for (auto it = source_arguments.begin(); it != source_arguments.end(); ++it) {
//         args << " (";
//         args << *it;
//         args << ")";
//     }

//     std::cout << std::left << args.str();
//     std::cout << std::endl;

//     if (instruction == END_EXPRESSION || instruction == ADD_HIST_TO_LIST) std::cout << std::endl;

// }

void AnalysisCommand::print_instruction() {
    print_instruction(0,0);
}


std::string ALILConverter::reserve_scoped_value_name() {
    std::stringstream new_var_name;
    new_var_name << "_V" << highest_var_val++ << "" << current_scope_name;
    last_value_name = new_var_name.str();
    return last_value_name;
}


ALILConverter::NameScope::NameScope(std::string type_name, ALILConverter *converter) {
    this_converter = converter;
    old_name = converter->current_scope_name;

    std::stringstream scope_name;
    scope_name << "_" << type_name << "_" << old_name;

    converter->current_scope_name = scope_name.str();
}

ALILConverter::NameScope::NameScope(std::string type_name, PNode id_node, ALILConverter *converter) {
    this_converter = converter;
    old_name = converter->current_scope_name;

    std::string name_lexeme = id_node->get_token()->get_lexeme();
    std::stringstream scope_name;
    scope_name << "_" << type_name << "_" << name_lexeme << "_" << old_name;

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

    AnalysisCommand give_init_name(ALIL::ADD_ALIAS);
    std::string name_of_init = info_id_node->consume_associated_string();
    give_init_name.add_dest_argument(name_of_init);
    give_init_name.add_source_argument(initializations_node->consume_associated_string());

    give_init_name.collect_into(commands);

    AnalysisCommand display_info(ALIL::DISPLAY_INFO);
    display_info.add_source_argument(name_of_init);
    display_info.add_empty_dest();

    display_info.collect_into(commands);
}

void ALILConverter::visit_definition(PNode node) {
    // DEFINITION -> ID
    //            -> EXTERN_ATTR |_| EXTERN_FUN |_| CORRECTIONLIB |_| PARTICLE_SUM |_| EXPRESSION

    PNode def_id_node = node->get_child(0);
    NameScope def_scope("DEF", def_id_node, this);

    PNode def_type_node = node->get_child(1);

    visit_children(node);

    std::optional<AnalysisCommand> operation;

    switch (def_type_node->get_ast_type()) {
        case AST::EXTERN_ATTR:
            operation.emplace(ALIL::ADD_EXTERN_ATTR);
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
    std::optional<AnalysisCommand> make_empty_composite; 
    
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


    for (PNode particle : comp_naming_elements->get_children()) {
        if (is_particle_step) {
            AnalysisCommand add_part_to_comp(ALIL::ADD_PART_TO_COMPOSITE);
            add_part_to_comp.add_source_argument(last_name_of_comp); 
            add_part_to_comp.add_source_argument(particle->consume_associated_string());

            // new name of the composite that now includes this particle
            last_name_of_comp = add_part_to_comp.reserve_dest_arg_value(this);
            
            add_part_to_comp.collect_into(commands);
        }
        is_particle_step = !is_particle_step;
    }

    assert(!is_particle_step);

    bool is_name_step = false;
    for (PNode name_part : comp_naming_elements->get_children()) {
        visit(name_part);
        if (is_name_step) {
            AnalysisCommand naming_command(ALIL::NAME_ELEMENT_OF_COMPOSITE);
            naming_command.add_source_argument(last_name_of_comp);
            naming_command.add_source_argument(std::to_string(index));

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

    assert(is_name_step);

    visit_children_after_index(node, 2);

    std::string last_mask = node->get_child(3)->consume_associated_string();

    for (auto local_global_pair : what_global_name_for_this_comp_name) {

        std::string local_name = local_global_pair.first;
        std::string global_name = local_global_pair.second;

        std::stringstream cut_down_global_name;
        cut_down_global_name << overall_name_of_composite << "->" << local_name;

        AnalysisCommand cut_down_element(ALIL::APPLY_MASK);
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

    AnalysisCommand cut_down_object(ALIL::APPLY_MASK);
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


    AnalysisCommand create_table(ALIL::CREATE_TABLE, table_id_node->get_token());

    std::string current_table = create_table.reserve_dest_arg_value(this);
    create_table.add_source_argument(num_vars_string);

    auto table_node_iterator = table_list_node->get_children().begin();

    for (int row = 0; row < num_entries; row++) {
        AnalysisCommand append_to_table(ALIL::APPEND_TO_TABLE);

        append_to_table.add_source_argument(current_table);
        current_table = append_to_table.reserve_dest_arg_value(this);

        AnalysisCommand create_table_value(do_errors ? ALIL::CREATE_TABLE_ERRORED_VALUE : ALIL::CREATE_TABLE_VALUE);
        std::string values_name = create_table_value.reserve_dest_arg_value(this);

        AnalysisCommand create_table_lower_bounds(ALIL::CREATE_TABLE_LOWER_BOUNDS);
        std::string lower_bound_name = create_table_lower_bounds.reserve_dest_arg_value(this);

        AnalysisCommand create_table_upper_bounds(ALIL::CREATE_TABLE_UPPER_BOUNDS);
        std::string upper_bound_name = create_table_upper_bounds.reserve_dest_arg_value(this);

        append_to_table.add_source_argument(values_name);
        append_to_table.add_source_argument(lower_bound_name);
        append_to_table.add_source_argument(upper_bound_name);

        for (int col = 0; col < num_columns_per_row; col++, table_node_iterator++) {

            assert(table_node_iterator != table_list_node->get_children().end());

            std::string current_arg_text = (*table_node_iterator)->consume_associated_string();
            if (col <= (do_errors ? 2 : 0)) {
                create_table_value.add_source_argument(current_arg_text);
            } else if (col % 2 == 0) {
                create_table_upper_bounds.add_source_argument(current_arg_text);
            } else {
                create_table_lower_bounds.add_source_argument(current_arg_text);
            }   
        }

        create_table_value.collect_into(commands);
        create_table_lower_bounds.collect_into(commands);
        create_table_upper_bounds.collect_into(commands);
        append_to_table.collect_into(commands);
    }

    AnalysisCommand final_naming(ALIL::ADD_ALIAS);
    final_naming.add_dest_argument(table_id_node->consume_associated_string());
    final_naming.add_source_argument(current_table);
}


void ALILConverter::visit_region(PNode node) {
    // REGION -> ID
    //        -> REGION_COMMANDS

    PNode region_id_node = node->get_child(0);
    NameScope region_scope("REG", region_id_node, this);

    PNode region_commands_node = node->get_child(1);

    visit_children(node);

    AnalysisCommand final_name_of_region(ALIL::ADD_ALIAS);
    final_name_of_region.add_dest_argument(region_id_node->consume_associated_string());
    final_name_of_region.add_source_argument(region_commands_node->consume_associated_string());

    final_name_of_region.collect_into(commands);
}


void ALILConverter::visit_histo_list(PNode node) {
    PNode histolist_id_node = node->get_child(0);
    NameScope histolist_scope("HISTOLIST", histolist_id_node, this);

    AnalysisCommand create_histo_list(ALIL::CREATE_EMPTY_HIST_LIST);
    create_histo_list.add_empty_source();
    std::string last_list = create_histo_list.reserve_dest_arg_value(this);

    create_histo_list.collect_into(commands);

    PNode histo_entries_list = node->get_child(1);
    for (PNode histo : histo_entries_list->get_children()) {
        visit_children(histo);

        // this node is a HISTOLIST_HISTOGRAM node, and so its child is a histogram node
        std::string histo_produced = histo->get_child(0)->consume_associated_string();
        AnalysisCommand add_to_list(ALIL::ADD_HIST_TO_LIST);
        add_to_list.add_source_argument(last_list);
        add_to_list.add_source_argument(histo_produced);
        last_list = add_to_list.reserve_dest_arg_value(this);

        add_to_list.collect_into(commands);
    }

    AnalysisCommand finish_list(ALIL::ADD_ALIAS);
    finish_list.add_dest_argument(histolist_id_node->consume_associated_string());
    finish_list.add_source_argument(last_list);

    finish_list.collect_into(commands);
}

void ALILConverter::visit_initializations(PNode node) {
    // INITIALIZATIONS -> N x INITIALIZATION

    AnalysisCommand create_info_list(ALIL::CREATE_EMPTY_INFO_LIST);
    std::string source = create_info_list.reserve_dest_arg_value(this);

    create_info_list.collect_into(commands);

    visit_children(node);

    for (auto initialization : node->get_children()) {
        AnalysisCommand add_info_to_list(AnalysisLevelInstruction::ADD_TO_INFO_LIST);
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

    AnalysisCommand create_mask(ALIL::CREATE_MASK);
    std::string global_name_of_first_object = (*what_global_name_for_this_comp_name.begin()).second;
    create_mask.add_source_argument(global_name_of_first_object);
    std::string source = create_mask.reserve_dest_arg_value(this);

    create_mask.collect_into(commands);

    for (PNode criterion : node->get_children()) {
        if (criterion->get_ast_type() == AST_type::DEFINITION) {

            PNode id_node = criterion->get_child(0);
            PNode sum_node = criterion->get_child(1);
            NameScope cand_scope("CAND", id_node, this);

            visit_children(criterion);

            AnalysisCommand add_def_name(ALIL::ADD_ALIAS);
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
            AnalysisCommand limit_mask(ALIL::LIMIT_MASK);
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

    AnalysisCommand create_mask(ALIL::CREATE_MASK);
    std::string source = create_mask.reserve_dest_arg_value(this);
    create_mask.add_source_argument(what_object_is_this);

    create_mask.collect_into(commands);

    visit_children(node);

    for (auto criterion : node->get_children()) {

        AnalysisCommand limit_mask(ALIL::LIMIT_MASK);
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

    AnalysisCommand make_empty_union(AnalysisLevelInstruction::CREATE_EMPTY_UNION);
    std::string source = make_empty_union.reserve_dest_arg_value(this);

    make_empty_union.collect_into(commands);

    PNode particle_list_node = node->get_child(0);

    visit_children(node);

    particle_list_node->get_children();

    auto children = particle_list_node->get_children();
    for (auto particle : children) {
        AnalysisCommand add_part_to_union(ALIL::ADD_PART_TO_UNION);

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

    std::optional<AnalysisCommand> sorter;

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

    AnalysisCommand invert_mask_expr(ALIL::EXPR_LOGICAL_NOT);
    invert_mask_expr.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(invert_mask_expr.reserve_dest_arg_value(this));

    invert_mask_expr.collect_into(commands);
 
}


void ALILConverter::visit_region_commands(PNode node) {
    // N x REGION_SELECT |_| REGION_REJECT |_| REGION_USE |_| REGION_WEIGHT |_| REGION_BIN |_| REGION_BINS |_| REGION_HISTO_USE |_| REGION_HISTOGRAM
    
    AnalysisCommand create_region(ALIL::CREATE_REGION);
    std::string source = create_region.reserve_dest_arg_value(this);

    create_region.collect_into(commands);

    visit_children(node);

    for (auto command : node->get_children()) {

        command->set_associated_string(source);
        visit(command);

        std::optional<AnalysisCommand> reg_command;

        reg_command->collect_into(commands);
    }

    // set the associated string to the final value name that has accumulated all infos thus far
    node->set_associated_string(source); 
}

void ALILConverter::visit_region_select(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommand select(ALIL::CUT_REGION);
    select.add_source_argument(last_region);
    select.add_source_argument(node->get_child(0)->consume_associated_string());
    node->set_associated_string(select.reserve_dest_arg_value(this)); 

    select.collect_into(commands);
}

void ALILConverter::visit_region_reject(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommand invert(ALIL::EXPR_LOGICAL_NOT);
    invert.add_source_argument(node->get_child(0)->consume_associated_string());
    std::string dest = invert.reserve_dest_arg_value(this);

    invert.collect_into(commands);

    AnalysisCommand select(ALIL::CUT_REGION);
    select.add_source_argument(last_region);
    select.add_source_argument(dest);

    node->set_associated_string(select.reserve_dest_arg_value(this));

    select.collect_into(commands);
}

void ALILConverter::visit_region_use(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommand use(ALIL::MERGE_REGIONS);
    use.add_source_argument(node->get_child(0)->consume_associated_string());
    use.add_source_argument(last_region);

    node->set_associated_string(use.reserve_dest_arg_value(this));

    use.collect_into(commands);
}

void ALILConverter::visit_region_weight(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommand weight(ALIL::WEIGHT_APPLY);
    weight.add_source_argument(node->get_child(0)->consume_associated_string());
    weight.add_source_argument(node->get_child(1)->consume_associated_string());
    
    node->set_associated_string(weight.reserve_dest_arg_value(this));

    weight.collect_into(commands);
}


void ALILConverter::visit_region_bin(PNode node) {

    std::string last_region = node->consume_associated_string();

    visit_children(node);

    AnalysisCommand make_bin(ALIL::CREATE_BIN_OF_REGION);
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

    node->set_associated_string(last_region);
}


void ALILConverter::visit_region_bins(PNode node) {
    std::string last_region = node->consume_associated_string();

    visit_children(node);

    std::string discriminant_expression = node->get_child(0)->consume_associated_string();

    std::optional<std::string> last_bound;

    for (PNode bound : node->get_child(1)->get_children()) {

        std::string lower_bound;

        if (!last_bound) {

            AnalysisCommand ge(ALIL::EXPR_GE);
            ge.add_source_argument(discriminant_expression);
            ge.add_source_argument(*last_bound);
            lower_bound = ge.reserve_dest_arg_value(this);
            ge.collect_into(commands);
        } else {
            lower_bound = "true";
        }

        last_bound.emplace(bound->consume_associated_string());

        AnalysisCommand lt(ALIL::EXPR_LT);
        lt.add_source_argument(discriminant_expression);
        lt.add_source_argument(*last_bound);
        std::string upper_bound = lt.reserve_dest_arg_value(this);
        lt.collect_into(commands);

        AnalysisCommand both_bounds(ALIL::EXPR_AND);
        both_bounds.add_source_argument(lower_bound);
        both_bounds.add_source_argument(upper_bound);
        std::string final_bound = both_bounds.reserve_dest_arg_value(this);
        both_bounds.collect_into(commands);

        AnalysisCommand bin(ALIL::CREATE_BIN_OF_REGION);
        bin.add_source_argument(last_region);
        bin.add_source_argument(final_bound);
        bin.reserve_dest_arg_value(this); // we do not need to know the name of this bin
        bin.collect_into(commands);
    }

    node->set_associated_string(last_region);

}

void ALILConverter::visit_region_histo_use(PNode node) {
    std::string this_region = node->consume_associated_string();
    AnalysisCommand histo_use(ALIL::USE_HIST);
    
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
    AnalysisCommand use_hist(ALIL::USE_HIST);

    use_hist.add_source_argument(hist_name);
    use_hist.add_source_argument(this_region);
    use_hist.add_empty_dest();

    use_hist.collect_into(commands);

    node->set_associated_string(this_region);
}


void ALILConverter::visit_histogram(PNode node) {
    
    bool is_2d = node->get_children().size() > 6;

    AnalysisCommand hist(is_2d ? ALIL::HIST_2D : ALIL::HIST_1D);

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

    AnalysisCommand create_empty(ALIL::CREATE_EMPTY_PARTICLE);
    create_empty.add_empty_source();
    std::string last_added_particle = create_empty.reserve_dest_arg_value(this);

    create_empty.collect_into(commands);

    for (PNode part : node->get_children()) {
        bool is_negative = part->get_ast_type() == AST::PARTICLE_NEGATE;
        AnalysisCommand add_part(is_negative ? ALIL::SUB_PARTICLE : ALIL::ADD_PARTICLE);
        PNode relevant_part_node = is_negative ? part : part->get_child(0);

        add_part.add_source_argument(last_added_particle);
        add_part.add_source_argument(relevant_part_node->consume_associated_string());
        last_added_particle = add_part.reserve_dest_arg_value(this);

        add_part.collect_into(commands);
    }

    node->set_associated_string(last_added_particle);
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
        case TOK::WITHIN:
            return ALIL::EXPR_WITHIN;
        case TOK::OUTSIDE:
            return ALIL::EXPR_OUTSIDE;
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

        case TOK::DOT_INDEX:


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

                AnalysisCommand within(inclusive_exclusive_determination(left_comparator->get_token(), right_comparator->get_token()));
                within.add_source_argument(discriminant->consume_associated_string());
                within.add_source_argument(left_bound->consume_associated_string());
                within.add_source_argument(right_bound->consume_associated_string());
                node->set_associated_string(within.reserve_dest_arg_value(this));
                within.collect_into(commands);
                return;
            }
            // intentionally falls through
        }

        default:
        {
            visit_children(node);
            AnalysisCommand binary_op(inst_for_binary(node->get_token()));
            binary_op.add_source_argument(node->get_child(0)->consume_associated_string());
            binary_op.add_source_argument(node->get_child(1)->consume_associated_string());
            node->set_associated_string(binary_op.reserve_dest_arg_value(this));
            binary_op.collect_into(commands);
        }
    
    }
}

void ALILConverter::visit_varying_terminal(PNode node) {
    node->set_associated_string(node->get_token()->get_lexeme());
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

// void ALILConverter::clean_command_list() {
//     std::deque<AnalysisCommand> new_list;

//     bool do_last_cutflow = false;
//     bool do_every_cutflow = false;

//     bool do_last_eventlist = false;
//     bool do_every_eventlist = false;

//     if (config.get_argument("cutflow") == "all") do_every_cutflow = true;
//     if (config.get_argument("cutflow") == "last") do_last_cutflow = true;

//     if (config.get_argument("eventlist") == "all") do_every_eventlist = true;
//     if (config.get_argument("eventlist") == "last") do_last_eventlist = true;


//     // backwards iteration pass
//     for (auto it = command_list.rbegin(); it != command_list.rend(); ++it) {
//         if (it->get_instruction() == DO_CUTFLOW_ON_REGION && !do_every_cutflow) {
//             if (do_last_cutflow) do_last_cutflow = false;
//             else continue;
//         } else if (it->get_instruction() == DO_EVENTLIST_ON_REGION && !do_every_eventlist) {
//             if (do_last_eventlist) do_last_eventlist = false;
//             else continue;
//         }

//         new_list.push_front(*it);
//     }
//     command_list = std::vector<AnalysisCommand>(new_list.begin(), new_list.end());

// }

// void ALILConverter::visitation(PNode root) {
//     visit(root);
//     clean_command_list();
// }

// void ALILConverter::print_commands() {

//     int top_size_of_dest = 0;
//     int top_size_of_inst = 0;

//     for (auto it = command_list.begin(); it != command_list.end(); ++it) {
//         if (it->has_dest_argument()) {
//             top_size_of_dest = std::max(top_size_of_dest, static_cast<int>(it->get_dest_argument().size()));
//         }
//         top_size_of_inst = std::max(top_size_of_inst, static_cast<int>(AnalysisCommand::instruction_to_text(it->get_instruction()).size()));
//     }

//     for (auto it = command_list.begin(); it != command_list.end(); ++it) {
//         it->print_instruction(top_size_of_dest+4, top_size_of_inst+1);
//     }
// }

// bool ALILConverter::clear_to_next() {
//     if (iter_command >= command_list.size()) return false;
//     return true;
// }

// AnalysisCommand ALILConverter::next_command() {
//     return command_list[iter_command++];
// }

ALILConverter::ALILConverter(Config &conf): highest_var_val(0), iter_command(0),  config(conf){}

ALILToFrameworkCompiler::ALILToFrameworkCompiler(ALILConverter *alil_in, Config &conf): alil(alil_in), config(conf) {}