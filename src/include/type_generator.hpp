#ifndef TYPE_GENERATOR_H
#define TYPE_GENERATOR_H

#include "alil.hpp"
#include "alil_converter.hpp"


#define CONVERTER_FUNCS_DECLARE(ENUM, NAME) \
    PType convert_##NAME(const AnalysisCommand &) override;

class Type;
typedef std::shared_ptr<Type> PType;

class TypeGenerator : public ALILToFrameworkCompiler<PType> {
    private:

    protected:
        ALIL_INSTRUCTION_LIST(CONVERTER_FUNCS_DECLARE);

    public:
        using ALILToFrameworkCompiler::ALILToFrameworkCompiler;

        PType get_type_of_command(const AnalysisCommand &command);
        void print() override;
};

#undef CONVERTER_FUNCS_DECLARE

#endif