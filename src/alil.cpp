#include "alil.hpp"
#include "alil_converter.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <memory>

AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok): instruction(inst), source_token(tok) {}

AnalysisCommand::AnalysisCommand(AnalysisLevelInstruction inst): instruction(inst) {}

AnalysisCommand::AnalysisCommand(const AnalysisCommand &other): instruction(other.instruction), dest_argument(other.dest_argument), source_arguments(other.source_arguments), source_token(other.source_token) {}

AnalysisLevelInstruction AnalysisCommand::get_instruction() {
    return instruction;
}
std::string AnalysisCommand::get_argument(std::size_t pos) {
    assert(pos >= 0);
    
    if (pos == 0) {
        if (dest_argument) return *dest_argument;
        assert(source_arguments.size() >= 1);
        return source_arguments[0];
    } else if (dest_argument) {
        size_t pos_in_vec = pos - 1;
        assert(pos_in_vec < source_arguments.size());
        return source_arguments[pos_in_vec];
    } else {
        assert(pos < source_arguments.size());
        return source_arguments[pos];
    }

}

bool AnalysisCommand::has_dest_argument() {
    return dest_argument.has_value();
}

std::string AnalysisCommand::get_dest_argument() {
    assert(dest_argument);
    return *dest_argument;
}

int AnalysisCommand::get_num_source_arguments() {
    return source_arguments.size();
}

std::string AnalysisCommand::get_source_argument(size_t pos) {
    assert(pos < source_arguments.size()); 
    return source_arguments[pos];
}

int AnalysisCommand::get_num_arguments() {
    return static_cast<int>(dest_argument.has_value()) + source_arguments.size();
}



void AnalysisCommand::print_instruction(int width_of_dest, int width_of_inst) {

    switch (instruction) {
        using enum ALIL;
        case CREATE_EMPTY_CARTESIAN: case CREATE_EMPTY_DIRECT: case CREATE_EMPTY_DISJOINT: case CREATE_EMPTY_HIST_LIST: case CREATE_EMPTY_INFO_LIST: case CREATE_EMPTY_VALUE_LIST: case CREATE_EMPTY_PARTICLE: case CREATE_EMPTY_UNION: case CREATE_TABLE: case CREATE_MASK: case CREATE_REGION: case CREATE_BIN_OF_REGION:
        {    
            std::cout << std::endl;
        } break;
        default:
        {

        }
    }

    std::cout << std::left << std::setw(width_of_dest) << (std::stringstream() << "(" << (dest_argument ? *dest_argument : "") << ") ").str() << std::left << std::setw(2) << " <- ";

    std::cout << std::left << std::setw(width_of_inst) << instruction_to_text(instruction);

    std::stringstream args;

    for (auto it = source_arguments.begin(); it != source_arguments.end(); ++it) {
        args << " (";
        args << *it;
        args << ")";
    }

    std::cout << std::left << args.str();

}



void AnalysisCommand::print_instruction() {
    print_instruction(0,0);
}

#define TYPE_TO_STRING(ENUM, NAME) \
case ALIL::ENUM: return #ENUM;
std::string AnalysisCommand::instruction_to_text(AnalysisLevelInstruction inst) {

    switch(inst) {
        ALIL_INSTRUCTION_LIST(TYPE_TO_STRING)
    }
    assert(false);
    return "";

}
#undef TYPE_TO_STRING

/*
AnalysisCommandBuilder : the uncollected version of an analysis command, capable of being modified.

*/

AnalysisCommandBuilder::AnalysisCommandBuilder(AnalysisLevelInstruction inst, std::weak_ptr<Token> tok) MAKE_UNCONSUMED: AnalysisCommand(inst, tok), has_been_collected(false) , dest_declared(false), source_declared(false){}

AnalysisCommandBuilder::AnalysisCommandBuilder(AnalysisLevelInstruction inst) MAKE_UNCONSUMED: AnalysisCommand(inst), has_been_collected(false), dest_declared(false), source_declared(false) {}

AnalysisCommandBuilder::AnalysisCommandBuilder(const AnalysisCommandBuilder &other): AnalysisCommand(other), has_been_collected(other.has_been_collected), dest_declared(other.dest_declared), source_declared(other.source_declared) {}

AnalysisCommandBuilder::~AnalysisCommandBuilder() CALLABLE_CONSUMED{
    assert(has_been_collected);
}

void AnalysisCommandBuilder::add_dest_argument(std::string arg) CALLABLE_UNCONSUMED{
    assert(!dest_argument);
    dest_argument = arg;
    dest_declared = true;
}

void AnalysisCommandBuilder::add_source_argument(std::string arg) CALLABLE_UNCONSUMED{
    source_arguments.push_back(arg);
    source_declared = true;
}

void AnalysisCommandBuilder::add_empty_source() CALLABLE_UNCONSUMED {
    source_declared = true;
}
void AnalysisCommandBuilder::add_empty_dest() CALLABLE_UNCONSUMED {
    dest_declared = true;
}


std::string AnalysisCommandBuilder::reserve_dest_arg_value(ALILConverter *alil_conv) CALLABLE_UNCONSUMED {
    std::string dest_val = alil_conv->reserve_scoped_value_name();
    add_dest_argument(dest_val);
    return dest_val;
}


void AnalysisCommandBuilder::collect_into(ALILCollection &target) CALLABLE_UNCONSUMED SET_CONSUMED{
    target.collect_command(*this);
    mark_collected();
}

void AnalysisCommandBuilder::mark_collected() CALLABLE_UNCONSUMED SET_CONSUMED{
    has_been_collected = true;
}


ALILCollection::ALILCollection() {

}

void ALILCollection::collect_command(AnalysisCommandBuilder &in PARAM_UNCONSUMED) {
    assert(in.source_declared && in.dest_declared);
    command_list.push_back(in);
}

void ALILCollection::print_collected_commands() {


    int top_size_of_dest = 0;
    int top_size_of_inst = 0;

    // first pass: gauge the dimensions of the list
    for (auto command : command_list) {
        if (command.has_dest_argument()) {
            top_size_of_dest = std::max(top_size_of_dest, static_cast<int>(command.get_dest_argument().size()));
        }
        top_size_of_inst = std::max(top_size_of_inst, static_cast<int>(AnalysisCommand::instruction_to_text(command.get_instruction()).size()));
    }

    // second pass: actually print the list using the dimensions we established
    for (auto command : command_list) {
        switch (command.get_instruction()) {
            using enum ALIL;
            case CREATE_EMPTY_CARTESIAN: case CREATE_EMPTY_DIRECT: case CREATE_EMPTY_DISJOINT: case CREATE_EMPTY_HIST_LIST: case CREATE_EMPTY_INFO_LIST: case CREATE_EMPTY_VALUE_LIST: case CREATE_EMPTY_PARTICLE: case CREATE_EMPTY_UNION: case CREATE_TABLE: case CREATE_MASK: case CREATE_REGION: case CREATE_BIN_OF_REGION:
            {    
                std::cout << std::endl;
            } break;
            default:
            {

            }
        }

        command.print_instruction(top_size_of_dest+4, top_size_of_inst+1);
    }

    std::cout << std::endl;
}