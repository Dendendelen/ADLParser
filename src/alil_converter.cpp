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

std::string ALILConverter::reserve_scoped_limit_name() {
    std::stringstream new_var_name;
    new_var_name << "_L" << highest_var_val++ << "" << current_scope_name;
    return new_var_name.str();
}

std::string ALILConverter::reserve_scoped_region_name() {
    std::stringstream new_var_name;
    new_var_name << "_R" << highest_var_val++ << "" << current_scope_name;
    return new_var_name.str();
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
    //TODO: do this
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

    AnalysisCommand make_empty_union(AnalysisLevelInstruction::MAKE_EMPTY_UNION);
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
    hist.add_source_argument(node->get_child(1)->consume_associated_string()); //TODO:check this4

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