#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "type.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "type_generator.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>




class EquivalenceRelation {
    // union-find data structure for equivalence classes

    private:
        std::unordered_map<PType, PType> parent;

    public:
        PType find_representative(PType source);
        void union_of_classes(PType first, PType second);
        PType resolve_higher_order(PType source);
        std::unordered_map<PType, PType> resolve_all();
        std::unordered_set<PType> get_all_representatives();

};

class PreOrder {
private:
    std::unordered_map<PType, std::unordered_set<PType>> supertypes;
    std::unordered_map<PType, std::unordered_set<PType>> subtypes;


    void ensure_exists(PType);
    bool has_path(PType, PType, std::unordered_set<PType> &);
    
public:
    bool add_subtype(PType sub, PType super);

    bool is_subtype(PType, PType);
    std::unordered_set<PType> get_supertypes(PType);
    std::unordered_set<PType> get_subtypes(PType);

    std::unordered_map<PType, std::unordered_set<PType>> get_all_supertypes();
    std::unordered_map<PType, std::unordered_set<PType>> get_all_subtypes();
    std::vector<PType> topological_sort();
};

class DeepSubtypeRelation {
    private:
        std::unordered_map<PType, std::unordered_set<PType>> deep_supers;
        std::unordered_map<PType, std::unordered_set<PType>> deep_subs;

        void ensure_exists(PType type) {
            if (deep_supers.find(type) == deep_supers.end()) {
                deep_supers[type] = {};
                deep_subs[type] = {};
            }
        }

    public:
        // Record a ≪: b. No structural checks — ≪: crosses constructor boundaries.
        void add_deep_subtype(PType sub, PType super) {
            ensure_exists(sub);
            ensure_exists(super);
            deep_supers[sub].insert(super);
            deep_subs[super].insert(sub);
        }


        bool is_deep_subtype(PType sub, PType super);

        // Get all direct deep-super edges (for iteration in propagation)
        const std::unordered_map<PType, std::unordered_set<PType>>& 
        get_all_edges() const {
            return deep_supers;
        }

        // Get all types b such that a ≪: b (transitive closure)
        std::unordered_set<PType> get_deep_supertypes(PType type) ;
};

class ConstructorContentSet {
    // implements the content_L (type) function
    // e.g. if OrderedList <: List<Number>, then content_List(OrderedList) = {Number}
    private:
        std::unordered_map<PType, std::unordered_set<PType>> content_mapping;
    public:
        void record_subtype_of_constructor(PType type, PType content);
        std::unordered_set<PType> get_contents(PType type);

        std::optional<PType> get_core(PType type, PreOrder& subtyping);
        bool is_structured(PType type);
};

class DeclaredConstructorRelations {
    private:
        std::unordered_map<PType, std::vector<std::pair<BaseType, PType>>> declared_constructor_subtypes;
    public:
        void declare_subtype_of_constructor(PType fundamental_type, BaseType constructor_type, PType content);
        bool could_be_subtype(PType sub, PType super);
};

// All constructor-specific operations parameterized by ConstructorId
class ConstructorSystem {
    BaseType id;
    ConstructorContentSet contents;
    
    // Per-constructor ≪: facts
    std::vector<std::pair<PType, PType>> hereditary_facts;
    
public:
    bool is_structured(PType t);
    std::optional<PType> get_core(PType t, PreOrder& ord);
    bool is_hereditary_subtype(PType a, PType b, PreOrder& ord, 
                                EquivalenceRelation& eq);
    bool are_similar(PType a, PType b, PreOrder& ord, 
                      EquivalenceRelation& eq);
    
    // Unwrap: given t of this constructor, return content type
    std::optional<PType> unwrap(PType t);
};

struct Ternary {
    enum TernaryType {
        TERN_TRUE,
        TERN_UNKNOWN,
        TERN_FALSE
    };

    TernaryType val;

    constexpr Ternary(TernaryType v) : val(v) {}

    // allow direct reference
    constexpr operator TernaryType() const { return val; }

    // disallow coaxing to bool
    explicit operator bool() = delete;

    constexpr Ternary l_and(const Ternary &t) const {
        if (val == TERN_FALSE || t.val == TERN_FALSE) return TERN_FALSE;
        if (val == TERN_TRUE && t.val == TERN_TRUE) return TERN_TRUE;
        return TERN_UNKNOWN;
    }

    void eq_land(const Ternary &t) {
        val = this->l_and(t);
    }
};


struct PTypePairHash {
    std::size_t operator()(const std::pair<PType, PType>& p) const {
        auto h1 = std::hash<PType>{}(p.first);
        auto h2 = std::hash<PType>{}(p.second);
        return h1 ^ (h2 << 32) ^ (h2 >> 32);
    }
};

// Replace DepthEquivalence with proper similarity computation
// a ≃_List b iff (both List-free) or 
//                 (both List-structured and core(a) ≃_List core(b))
class Similarity {
    private:
        ConstructorContentSet& contents;
        PreOrder& subtyping;
        EquivalenceRelation& equiv;

        bool contradiction = false;

        std::unordered_map<PType, PType> sim_parent;

        enum Structure { UNKNOWN, L_FREE, L_STRUCTURED };
        struct GroupInfo {
            Structure structure = UNKNOWN;
            PType core = nullptr;  // meaningful only when L_STRUCTURED
        };
        std::unordered_map<PType, GroupInfo> group_info;

        PType sim_find(PType t);
        PType sim_union(PType a, PType b);  // returns new representative
        GroupInfo& get_info(PType rep);

        bool are_similar_impl(PType a, PType b, std::unordered_set<std::pair<PType,PType>, PTypePairHash>& visited);
        Ternary have_similar_impl(PType a, PType b, std::unordered_set<std::pair<PType,PType>, PTypePairHash>& visited);

        Structure classify(PType t);
        std::optional<PType> resolve_core(PType t);
                        
    public:
        Similarity(ConstructorContentSet& c, PreOrder& s, EquivalenceRelation& e);
        // Coinductive check with visited set to handle cycles
        
        bool add_similar(PType a, PType b);
        void notify_structured(PType t, PType content);
        bool assert_structured(PType t_in);
        bool assert_free(PType t_in);

        Ternary is_structured_query(PType t_in);
        Ternary is_free_query(PType t_in) ;
        
        bool are_similar(PType a, PType b);
        Ternary have_similar(PType a, PType b);
        bool has_contradiction() const { return contradiction; }
};


#define CONVERTER_FUNCS_DECLARE(ENUM, NAME) \
    PType convert_##NAME(const AnalysisCommand &) override;


class Typer : public ALILToFrameworkCompiler<PType> {
    private:

        TypeGenerator type_generator;

        EquivalenceRelation equiv;
        PreOrder subtyping;
        DeepSubtypeRelation hereditary_subtyping;

        ConstructorContentSet list_content_set;
        DeclaredConstructorRelations list_declared_structure;
        Similarity similarity;

        std::optional<PType> get_list_core(PType type);

        bool constrain_equality(PType, PType);
        bool constrain_similar_depth(PType, PType);
        bool constrain_subtype(PType, PType);
        bool constrain_hereditary_subtype(PType sub, PType super);

        bool types_hereditarily_incompatible(PType lhs, PType rhs);
        bool types_subtype_incompatible(PType sub, PType super);
        bool types_structurally_incompatible(PType lhs, PType rhs);
        bool types_structurally_equal(PType lhs, PType rhs);

        bool propagate_once();
        void propagate_to_fixed_point();
        void apply_conclusion(Statement &conclusion, Constraint &constraint);

        Ternary truth_of_premise(PType lhs, PType rhs, StatementForm form);

        std::vector<PType> all_live_types;

        std::vector<Constraint> running_valid_constraints;
        std::unordered_map<std::string, PType> types_of_variables;
        std::vector<std::string> order_of_variables;

        std::unordered_set<std::string> used_variables;


    protected:
        ALIL_INSTRUCTION_LIST(CONVERTER_FUNCS_DECLARE);

    public:
        Typer(ALILCollection *alil_in, Config &conf): ALILToFrameworkCompiler<PType>(alil_in, conf), type_generator(nullptr, conf), similarity(list_content_set, subtyping, equiv) {}

    
        void collect_existing_constraints();
        void resolve_constraints();

        void print() override;
};

#undef CONVERTER_FUNCS_DECLARE
#endif