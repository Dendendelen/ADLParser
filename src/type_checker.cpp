
#include "type_checker.hpp"
#include "alil.hpp"
#include "alil_converter.hpp"
#include "node.hpp"
#include "exceptions.hpp"
#include "type.hpp"
#include "type_generator.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>


// Equivalence class

PType EquivalenceRelation::find_representative(PType source) {
    if (parent.count(source) == 0) {
        parent[source] = source; // self-represent an empty class
    }

    if (parent[source] != source) {
        parent[source] = find_representative(parent[source]);
    }   
    return parent[source];
}

void EquivalenceRelation::union_of_classes(PType first_in, PType second_in) {
    PType first = find_representative(first_in);
    PType second = find_representative(second_in);

    
    // these two classes are already the same
    if (first == second) return;


    // we would really like to have the class be represented by a not completely generic type
    bool first_generic = (first->get_base_type() == TYPE::GENERIC);
    bool second_generic = (second->get_base_type() == TYPE::GENERIC);

    if (first_generic && !second_generic) {
        // first class is represented by non-generic type - use it for the second as well
        parent[first] = second;
    } else if (!first_generic && second_generic) {
        // same with second class
        parent[second] = first;
    } else {
        // either they are both non-generic or both not. Choose the first as our representative arbitrarily
        parent[second] = first;

        if (!first_generic && !second_generic) {
            // neither are generic, so we confirm that the base types match
            assert(first->get_base_type() == second->get_base_type());

            if (first->get_base_type() == TYPE::FUNCTION) {
                assert(first->get_num_of_sources() == second->get_num_of_sources());
                for (int i = 0; i < first->get_num_of_sources(); i++) {
                    union_of_classes(first->get_source_type(i), second->get_source_type(i));
                }
            }

            if (first->get_base_type() == TYPE::FUNCTION || first->get_base_type() == TYPE::LIST) {
                union_of_classes(first->get_dest_type(), second->get_dest_type());
            }
        }
        
    }
}



PType EquivalenceRelation::resolve_higher_order(PType source) {
    PType rep = find_representative(source);

    if (rep->get_base_type() == TYPE::GENERIC) {
        // the best we can do is a generic
        return rep;
    }

    if (rep->get_base_type() == TYPE::FUNCTION) {
        // this is represented by a function - ensure we have the most specific form of function possible as the representative type for this

        PType fun(std::make_shared<Type>(TYPE::FUNCTION));
        for (int i = 0; i < rep->get_num_of_sources(); i++) {
            fun->add_source_type(resolve_higher_order(rep->get_source_type(i)));
        }
        PType resolved_dest = resolve_higher_order(rep->get_dest_type());
        fun->add_dest_type(resolved_dest);
        parent[rep] = fun;
        return fun;

    }

    if (rep->get_base_type() == TYPE::LIST) {
        // similarly, this is represented by a list - ensure we have the most specific form of list
        PType resolved_elem = resolve_higher_order(rep->get_dest_type());
        PType list(std::make_shared<Type>(TYPE::LIST));
        list->add_dest_type(resolved_elem);
        parent[rep] = list;
        return list;

    }

    return rep;
}

std::unordered_map<PType, PType> EquivalenceRelation::resolve_all() {
    std::unordered_map<PType, PType> resolved_map;
    
    // first collect all our types
    std::vector<PType> all_types;
    for (const auto &entry : parent) {
        all_types.push_back(entry.first);
    }

    for (PType type : all_types) {
        resolved_map[type] = resolve_higher_order(type);
    }

    return resolved_map;
}

std::unordered_set<PType> EquivalenceRelation::get_all_representatives() {
    std::unordered_set<PType> all_reps;
    for (auto &pair : parent) {
       
        all_reps.emplace(find_representative(pair.first));
    }
    return all_reps;
}


// Pre-order class
// This is reflexive and transitive but not antisymmetric

void PreOrder::ensure_exists(PType type) {
    if (supertypes.find(type) == supertypes.end()) {
        supertypes[type] = {};
        subtypes[type] = {};
    }
}

bool PreOrder::has_path(PType from, PType to, std::unordered_set<PType> &visited) {

    // Reflexivity
    if (from == to) return true;

    // if we have looped back around to this, we have definitely not found a path
    if (visited.count(from) != 0) return false;
    
    ensure_exists(from);
    ensure_exists(to);

    visited.insert(from);
    
    // check the supertypes of the LHS - if we have found a path from there, there is clearly a chain since a <: b <: c ==> a <: c
    for (PType super : supertypes[from]) {
        if (has_path(super, to, visited)) {
            return true;
        }
    }
    
    return false;
}

bool PreOrder::add_subtype(PType sub, PType super){
    ensure_exists(sub);
    ensure_exists(super);

    // Reflexivity, a <: a always
    if (sub == super) return true;

    bool sub_generic = sub->get_base_type() == TYPE::GENERIC;
    bool super_generic = super->get_base_type() == TYPE::GENERIC;

    if (!sub_generic && !super_generic) {
        // both are not fully generic types - we can use more information
        if (sub->get_base_type() == TYPE::FUNCTION && super->get_base_type() == TYPE::FUNCTION) {
            assert(sub->get_num_of_sources() == super->get_num_of_sources());
            
            // functions are contravariant in arguments - add those constraints
            for (int i = 0; i < sub->get_num_of_sources(); i++) {
                if (!add_subtype(super->get_source_type(i), sub->get_source_type(i))) return false;
            }
            
            // functions are also covariant in return type
            if(!add_subtype(sub->get_dest_type(), super->get_dest_type())) return false;
            
        } else if (sub->get_base_type() == TYPE::LIST && super->get_base_type() == TYPE::LIST) {
            // lists are covariant bidirectionally (by axiom of our type system)
            if (!add_subtype(sub->get_dest_type(), super->get_dest_type())) return false;
        } else {
            // type system has the following axiom:
            // different non-generic constructor-based types cannot be subtypes
            if(sub->get_base_type() != super->get_base_type()) {
                return false;
            }
        }
    }

    // one is now a subtype of the other
    supertypes[sub].insert(super);
    subtypes[super].insert(sub);
    
    return true;


}


bool PreOrder::is_subtype(PType sub, PType super) {
    ensure_exists(sub);
    ensure_exists(super);
    
    std::unordered_set<PType> visited;
    return has_path(sub, super, visited);
}

std::unordered_set<PType> PreOrder::get_supertypes(PType type) {
    ensure_exists(type);
    
    std::unordered_set<PType> result;
    std::queue<PType> worklist;
    
    worklist.push(type);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        for (PType super : supertypes[current]) {
            if (result.insert(super).second) {
                worklist.push(super);
            }
        }
    }
    
    return result;
}

std::unordered_set<PType> PreOrder::get_subtypes(PType type) {
    ensure_exists(type);
    
    std::unordered_set<PType> result;
    std::queue<PType> worklist;
    
    worklist.push(type);
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        
        for (PType sub : subtypes[current]) {
            if (result.insert(sub).second) {
                worklist.push(sub);
            }
        }
    }
    
    return result;
}

std::unordered_map<PType, std::unordered_set<PType>> PreOrder::get_all_supertypes() {
    std::unordered_map<PType, std::unordered_set<PType>> result;
    
    for (const auto &entry : supertypes) {
        result[entry.first] = get_supertypes(entry.first);
    }
    
    return result;
}

std::unordered_map<PType, std::unordered_set<PType>> PreOrder::get_all_subtypes() {
    std::unordered_map<PType, std::unordered_set<PType>> result;
    
    for (const auto &entry : subtypes) {
        result[entry.first] = get_subtypes(entry.first);
    }
    
    return result;
}

std::vector<PType> PreOrder::topological_sort() {
    std::unordered_map<PType, int> in_degree;
    std::vector<PType> result;
    std::queue<PType> worklist;
    
    for (const auto &entry : subtypes) {
        in_degree[entry.first] = entry.second.size();
    }
    
    for (const auto &entry : in_degree) {
        if (entry.second == 0) {
            worklist.push(entry.first);
        }
    }
    
    while (!worklist.empty()) {
        PType current = worklist.front();
        worklist.pop();
        result.push_back(current);
        
        for (PType super : supertypes[current]) {
            in_degree[super]--;
            if (in_degree[super] == 0) {
                worklist.push(super);
            }
        }
    }
    
    return result;
}

std::unordered_set<PType> DeepSubtypeRelation::get_deep_supertypes(PType type) {
    ensure_exists(type);
    std::unordered_set<PType> result;
    std::queue<PType> q;
    q.push(type);
    while (!q.empty()) {
        PType cur = q.front(); q.pop();
        for (PType s : deep_supers[cur])
            if (result.insert(s).second) q.push(s);
    }
    return result;
}

bool DeepSubtypeRelation::is_deep_subtype(PType sub, PType super) {
    if (sub == super) return true;
    ensure_exists(sub);
    ensure_exists(super);
    std::unordered_set<PType> visited;
    std::queue<PType> q;
    q.push(sub);
    visited.insert(sub);
    while (!q.empty()) {
        PType cur = q.front(); q.pop();
        if (cur == super) return true;
        for (PType s : deep_supers[cur]) {
            if (visited.insert(s).second) q.push(s);
        }
    }
    return false;
}

void DeclaredConstructorRelations::declare_subtype_of_constructor(PType fundamental_type, BaseType constructor_type, PType content) {
    return;
}

bool DeclaredConstructorRelations::could_be_subtype(PType sub, PType super) {
    return false;
}



void ConstructorContentSet::record_subtype_of_constructor(PType type, PType content) {
    content_mapping[type].insert(content);
}
    
bool ConstructorContentSet::is_structured(PType type) {
    // by definition,
    // t is L-structured <==> content_L (t) =/= {} 

    auto it = content_mapping.find(type);
    return it != content_mapping.end() && !it->second.empty();
}
    
std::unordered_set<PType> ConstructorContentSet::get_contents(PType type) {
    auto it = content_mapping.find(type);
    if (it == content_mapping.end()) return {};
    return it->second;
}
    

std::optional<PType> ConstructorContentSet::get_core(PType type, PreOrder& subtyping) {
    // returns the least element of content_L(type) under subtyping, if exists
    // by axiom,  content_L(t) always has a least element

    auto it = content_mapping.find(type);
    if (it == content_mapping.end() || it->second.empty()) 
        return std::nullopt;
    
    const auto& cts = it->second;
    
    for (PType candidate : cts) {
        bool is_least = true;
        for (PType other : cts) {
            if (candidate == other) continue;
            if (!subtyping.is_subtype(candidate, other)) {
                is_least = false;
                break;
            }
        }
        if (is_least) return candidate;
    }
    
    // violation of the axiom, implies insufficient information
    return std::nullopt; 
}


Similarity::Similarity(ConstructorContentSet& c, PreOrder& s, EquivalenceRelation& e): contents(c), subtyping(s), equiv(e) {}
    
PType Similarity::sim_find(PType t) {
    if (sim_parent.count(t) == 0) {
        // self-parent an empty similarity class
        sim_parent[t] = t;
        return t;
    }
    
    if (sim_parent[t] != t) {
        sim_parent[t] = sim_find(sim_parent[t]);
    }
    return sim_parent[t];

}

PType Similarity::sim_union(PType a_in, PType b_in) {
    PType a = sim_find(a_in);
    PType b = sim_find(b_in);
    if (a == b) return a;
    // prefer non-generic representative, same logic as EquivalenceRelation
    if (a->get_base_type() == TYPE::GENERIC && b->get_base_type() != TYPE::GENERIC) {
        sim_parent[a] = b;
        return b;
    }
    sim_parent[b] = a;
    return a;
}

Similarity::GroupInfo &Similarity::get_info(PType rep) {
    if (group_info.count(rep) == 0) {
        group_info[rep] = {UNKNOWN, nullptr};
    }
    return group_info[rep];
}

Similarity::Structure Similarity::classify(PType t) {
    // a type is L-free, L-structured, or unknown
    t = equiv.find_representative(t);
    if (t->get_base_type() == TYPE::LIST) return L_STRUCTURED; // manifestly L-structured
    if (contents.is_structured(t)) return L_STRUCTURED; // L-structured by implication
    if (t->get_base_type() == TYPE::GENERIC) return UNKNOWN;
    return L_FREE;
}

std::optional<PType> Similarity::resolve_core(PType t_in) {
    PType t = equiv.find_representative(t_in);

    // it is a theorem that core(L<t>) = t 
    if (t->get_base_type() == TYPE::LIST) {
        return t->get_dest_type();
    }
    return contents.get_core(t, subtyping);
}

bool Similarity::add_similar(PType a_in, PType b_in) {
    if (contradiction) return false;
    
    PType a_equiv_rep = equiv.find_representative(a_in);
    PType b_equiv_rep = equiv.find_representative(b_in);
    
    PType ra = sim_find(a_equiv_rep);
    PType rb = sim_find(b_equiv_rep);
    if (ra == rb) return true;  // already in same obligation group
    
    // Determine current structure of each group
    GroupInfo ia = get_info(ra);
    GroupInfo ib = get_info(rb);
    
    // Refresh from actual type information
    Structure sa = (ia.structure != UNKNOWN) ? ia.structure : classify(a_equiv_rep);
    Structure sb = (ib.structure != UNKNOWN) ? ib.structure : classify(b_equiv_rep);
    
    // Contradiction: one free, one structured
    if (sa == L_FREE && sb == L_STRUCTURED) { contradiction = true; return false; }
    if (sa == L_STRUCTURED && sb == L_FREE) { contradiction = true; return false; }
    
    // Merge groups
    PType rc = sim_union(ra, rb);
    GroupInfo new_info = get_info(rc);
    
    if (sa == L_FREE && sb == L_FREE) {
        new_info.structure = L_FREE;
        new_info.core = nullptr;
    } else if (sa == L_STRUCTURED && sb == L_STRUCTURED) {
        new_info.structure = L_STRUCTURED;
        // Both structured: cores must also be similar
        auto core_a = (ia.core != nullptr) ? std::optional<PType>(ia.core) : resolve_core(a_equiv_rep);
        auto core_b = (ib.core != nullptr) ? std::optional<PType>(ib.core) : resolve_core(b_equiv_rep);
        
        if (core_a.has_value()) new_info.core = *core_a;
        else if (core_b.has_value()) new_info.core = *core_b;
        
        if (core_a.has_value() && core_b.has_value()) {
            // Recursive obligation: cores must be similar
            if (!add_similar(*core_a, *core_b)) return false;
        }
        // If only one has a core, the other's core is deferred
    } else if (sa == L_STRUCTURED) {
        new_info.structure = L_STRUCTURED;
        new_info.core = (ia.core != nullptr) ? ia.core : 
                  resolve_core(a_equiv_rep).value_or(nullptr);
    } else if (sb == L_STRUCTURED) {
        new_info.structure = L_STRUCTURED;
        new_info.core = (ib.core != nullptr) ? ib.core : 
                  resolve_core(b_equiv_rep).value_or(nullptr);
    } else {
        // Both unknown
        new_info.structure = UNKNOWN;
        new_info.core = nullptr;
    }
    
    return true;
}

void Similarity::notify_structured(PType t, PType content) {
    if (contradiction) return;
    
    t = equiv.find_representative(t);
    PType rt = sim_find(t);
    GroupInfo& info = get_info(rt);
    
    if (info.structure == L_FREE) {
        contradiction = true;
        return;
    }
    
    info.structure = L_STRUCTURED;
    
    if (info.core == nullptr) {
        info.core = content;
    }
}

// Assert that t is L-structured
bool Similarity::assert_structured(PType t_in) {
    if (contradiction) return false;
    
    PType t = equiv.find_representative(t_in);
    Structure s = classify(t);
    
    if (s == L_FREE) {
        // Contradiction: we know it's L-free but asserting structured
        contradiction = true;
        return false;
    }
    
    // Record the structural classification
    PType rt = sim_find(t);
    GroupInfo& info = get_info(rt);
    
    if (info.structure == L_FREE) {
        contradiction = true;
        return false;
    }
    
    info.structure = L_STRUCTURED;
    // Note: we do NOT set info.core
    // The type is structured but its core is unconstrained.
    
    return true;
}

// Assert that t is L-free
bool Similarity::assert_free(PType t_in) {
    if (contradiction) return false;
    
    PType t = equiv.find_representative(t_in);
    Structure s = classify(t);
    
    if (s == L_STRUCTURED) {
        // Contradiction: we know it's structured but asserting free
        contradiction = true;
        return false;
    }
    
    PType rt = sim_find(t);
    GroupInfo& info = get_info(rt);
    
    if (info.structure == L_STRUCTURED) {
        contradiction = true;
        return false;
    }
    
    info.structure = L_FREE;
    info.core = nullptr;
    
    return true;
}

// Ternary query: is t L-structured?
Ternary Similarity::is_structured_query(PType t_in) {
    PType t = equiv.find_representative(t_in);
    
    // Check direct classification
    Structure s = classify(t);
    if (s == L_STRUCTURED) return Ternary::TERN_TRUE;
    if (s == L_FREE) return Ternary::TERN_FALSE;
    
    // Check similarity group info
    PType rt = sim_find(t);
    GroupInfo& info = get_info(rt);
    if (info.structure == L_STRUCTURED) return Ternary::TERN_TRUE;
    if (info.structure == L_FREE) return Ternary::TERN_FALSE;
    
    return Ternary::TERN_UNKNOWN;
}

// Ternary query: is t L-free?
Ternary Similarity::is_free_query(PType t_in) {
    Ternary s = is_structured_query(t_in);
    if (s == Ternary::TERN_TRUE) return Ternary::TERN_FALSE;
    if (s == Ternary::TERN_FALSE) return Ternary::TERN_TRUE;
    return Ternary::TERN_UNKNOWN;
}

bool Similarity::are_similar(PType a, PType b) {
    // ~= is defined coinductively, we search for it
    if (contradiction) return false;

    std::unordered_set<std::pair<PType,PType>, PTypePairHash> visited;
    return are_similar_impl(a, b, visited);
}
    
bool Similarity::are_similar_impl(PType a_in, PType b_in, std::unordered_set<std::pair<PType,PType>, PTypePairHash>& visited) {
    PType a = equiv.find_representative(a_in);
    PType b = equiv.find_representative(b_in);
    
    // reflexive relation
    if (a == b) return true;
    
    // if in same similarity obligation group (and no contradiction), similar
    if (sim_find(a) == sim_find(b)) return true;
    
    // coinduction assumption
    auto key = std::make_pair(std::min(a, b), std::max(a, b));
    if (visited.count(key)) return true;
    visited.insert(key);
    
    Structure sa = classify(a);
    Structure sb = classify(b);
    
    if (sa == UNKNOWN || sb == UNKNOWN) return false;  // can't confirm
    if (sa == L_FREE && sb == L_FREE) return true; // definitely similar
    if (sa != sb) return false; // different classes, so definitionally not similar
    
    // both L-structured is the only option remaining
    auto core_a = resolve_core(a);
    auto core_b = resolve_core(b);
    
    if (!core_a.has_value() || !core_b.has_value()) return false;
    
    return are_similar_impl(*core_a, *core_b, visited);
}

// ternary query for premise eval
Ternary Similarity::have_similar(PType a, PType b) {
    std::unordered_set<std::pair<PType,PType>, PTypePairHash> visited;
    return have_similar_impl(a, b, visited);
}

Ternary Similarity::have_similar_impl(PType a_in, PType b_in, std::unordered_set<std::pair<PType,PType>, PTypePairHash>& visited) {
    PType a = equiv.find_representative(a_in);
    PType b = equiv.find_representative(b_in);
    
    if (a == b) return Ternary::TERN_TRUE;
    if (sim_find(a) == sim_find(b)) return Ternary::TERN_TRUE;
    
    auto key = std::make_pair(std::min(a, b), std::max(a, b));
    if (visited.count(key)) return Ternary::TERN_TRUE;  // coinductive
    visited.insert(key);
    
    Structure sa = classify(a);
    Structure sb = classify(b);
    
    if (sa == UNKNOWN || sb == UNKNOWN) return Ternary::TERN_UNKNOWN;
    if (sa == L_FREE && sb == L_FREE) return Ternary::TERN_TRUE;
    if (sa != sb) return Ternary::TERN_FALSE;
    
    auto core_a = resolve_core(a);
    auto core_b = resolve_core(b);
    if (!core_a.has_value() || !core_b.has_value()) return Ternary::TERN_UNKNOWN;
    
    return have_similar_impl(*core_a, *core_b, visited);
}


#define CONVERTER_FUNCS_DEFINE(ENUM, NAME) \
PType Typer::convert_##NAME(const AnalysisCommand &cmd) {return type_generator.get_type_of_command(cmd);}

ALIL_INSTRUCTION_LIST(CONVERTER_FUNCS_DEFINE);

#undef CONVERTER_FUNCS_DEFINE


// The actual type system handles the following rules
// equivalence =
// subtype <:
// hereditary subtype <<:
// similarity ~=
// list_content_set content_L(t)

std::optional<PType> Typer::get_list_core(PType type) {
    type = equiv.find_representative(type);
    
    // Syntactic case: type IS List⟨c⟩
    if (type->get_base_type() == TYPE::LIST) {
        return type->get_dest_type();
    }
    
    // Semantic case: type is List-structured via recorded contents
    return list_content_set.get_core(type, subtyping);
}

bool Typer::constrain_equality(PType first_in, PType second_in) {

    auto first = equiv.find_representative(first_in);
    auto second = equiv.find_representative(second_in);

    if (first == second) return true;

    bool first_generic_class = (first->get_base_type() == TYPE::GENERIC);
    bool second_generic_class = (second->get_base_type() == TYPE::GENERIC);

    if (!first_generic_class && !second_generic_class) {

        if (first->get_base_type() != second->get_base_type()) {
            return false;
        }

        if (first->get_base_type() == TYPE::FUNCTION && first->get_num_of_sources() != second->get_num_of_sources()) {
            return false;
        }

        // merge internals of these types by axiom
        if (first->get_base_type() == TYPE::LIST) {
            if (!constrain_equality(first->get_dest_type(), second->get_dest_type())) {
                return false;
            }
        }

        if (first->get_base_type() == TYPE::FUNCTION) {
            for (int i = 0; i < first->get_num_of_sources(); i++) {
                if (!constrain_equality(first->get_source_type(i), second->get_source_type(i))) {
                    return false;
                }
            }

            if (!constrain_equality(first->get_dest_type(),second->get_dest_type())) {
                return false;
            }
        }
    }

    // record equality into our equivalence classes
    equiv.union_of_classes(first, second);

    // a = b ==> a ~= b
    if (!similarity.add_similar(first, second)) return false;

    // a = b ==> a <: b and b <: a
    if (!subtyping.add_subtype(first, second)) return false;
    if (!subtyping.add_subtype(second, first)) return false;
    
    if (first->get_base_type() == TYPE::LIST) {
        list_content_set.record_subtype_of_constructor(second, first->get_dest_type());
        similarity.notify_structured(second, first->get_dest_type());
    }
    if (second->get_base_type() == TYPE::LIST) {
        list_content_set.record_subtype_of_constructor(first, second->get_dest_type());
        similarity.notify_structured(first, second->get_dest_type());
    }

    return true;
}


bool Typer::constrain_similar_depth(PType first, PType second) {
    return similarity.add_similar(first, second);
}



// void Typer::apply_depth_hereditary_simplification() {
//     // Collect all types that appear in hereditary relationships
//     std::unordered_set<PType> all_types;
//     for (const auto& [sub, supers] : hereditary_subtyping.get_all_supertypes()) {
//         all_types.insert(sub);
//         for (PType super : supers) {
//             all_types.insert(super);
//         }
//     }
    
//     // Check all pairs (using is_subtype for transitive closure)
//     for (PType a_orig : all_types) {
//         for (PType b_orig : all_types) {
//             PType a = equiv.find_representative(a_orig);
//             PType b = equiv.find_representative(b_orig);
//             if (a == b) continue;
//             if (subtyping.is_subtype(a, b)) continue;
            
//             if (hereditary_subtyping.is_subtype(a, b) && 
//                 similarity.are_similar(a, b)) {
//                 subtyping.add_subtype(a, b);
//             }
//         }
//     }
// }



// bool Typer::apply_subtyping_antisymmetry_simplification() {
//     bool changed = false;

//     auto all_supertypes = subtyping.get_all_supertypes();

//     std::vector<std::pair<PType, PType>> mutually_subtyped_pairs;

//     for (const auto& [sub, supers] : all_supertypes) {
//         PType sub_rep = equiv.find_representative(sub);

//         for (PType super : supers) {
//             PType super_rep = equiv.find_representative(super);

//             if (sub_rep == super_rep) {
//                 continue;
//             }

//             /*
//                 Antisymmetry:

//                     sub <: super
//                     super <: sub
//                     ----------------
//                     sub = super

//                 Since get_all_supertypes() is transitive, this also catches
//                 longer cycles, not just direct two-edge cycles.
//             */
//             if (subtyping.is_subtype(super_rep, sub_rep)) {
//                 mutually_subtyped_pairs.emplace_back(sub_rep, super_rep);
//             }
//         }
//     }

//     for (const auto& [a, b] : mutually_subtyped_pairs) {
//         PType a_rep = equiv.find_representative(a);
//         PType b_rep = equiv.find_representative(b);

//         if (a_rep == b_rep) {
//             continue;
//         }

//         if (!constrain_equality(a_rep, b_rep)) {
//             std::stringstream ss;
//             ss << "Subtype antisymmetry requires equality of incompatible types: ";

//             std::streambuf* old_stdout = std::cout.rdbuf();

//             std::stringstream a_print;
//             std::cout.rdbuf(a_print.rdbuf());
//             a_rep->print();

//             std::stringstream b_print;
//             std::cout.rdbuf(b_print.rdbuf());
//             b_rep->print();

//             std::cout.rdbuf(old_stdout);

//             raise_type_exception(
//                 "Subtype cycle forces incompatible equality: " +
//                 a_print.str() + " and " + b_print.str(),
//                 a_rep->get_origin_token()
//             );
//         }

//         changed = true;
//     }

//     return changed;
// }

// bool Typer::apply_structural_union_simplification() {

//     bool changed = false;

//     auto reps = equiv.get_all_representatives();
//     for (auto rep : reps) {
//         for (auto rep2 : reps) {
//             if (rep == rep2) continue;
//             if (types_structurally_equal(rep, rep2)) {
//                 // std::cout << "[";
//                 // rep->print(); std::cout << "|";
//                 // rep2->print(); std::cout << "]" << std::endl;
//                 if (!constrain_equality(rep, rep2)) return false;
//                 changed = true;
//             }
//         }
//     }

//     return changed;
// }

bool Typer::constrain_subtype(PType sub, PType super) {
    sub = equiv.find_representative(sub);
    super = equiv.find_representative(super);

    if (types_subtype_incompatible(sub, super)) return false;

    bool result = subtyping.add_subtype(sub, super);
    
    // when super = L<d>, record d into the content set 
    if (result && super->get_base_type() == TYPE::LIST) {
        list_content_set.record_subtype_of_constructor(sub, super->get_dest_type());
        similarity.notify_structured(sub, super->get_dest_type());
        for (PType sub_of_sub : subtyping.get_subtypes(sub)) {
            list_content_set.record_subtype_of_constructor(sub_of_sub, super->get_dest_type());
            similarity.notify_structured(sub_of_sub, super->get_dest_type());
        }
    }
    
    return result;
}

bool Typer::constrain_hereditary_subtype(PType sub_in, PType super_in) {
    PType sub = equiv.find_representative(sub_in);
    PType super = equiv.find_representative(super_in);

    if (types_hereditarily_incompatible(sub, super)) return false;

    PType current = sub;
    while (true) {
        current = equiv.find_representative(current);
        hereditary_subtyping.add_deep_subtype(current, super);
        
        // Try to peel one L-layer
        auto core = get_list_core(current);
        if (!core) break; // current is L-free: chain terminates

        current = *core;
    }
    return true;
}


bool Typer::types_structurally_incompatible(PType lhs, PType rhs) {
    // in a vacuum, generic types are never structurally incompatible, as they can unify with anything
    if (lhs->get_base_type() == TYPE::GENERIC || rhs->get_base_type() == TYPE::GENERIC) {
        return false;
    }
    
    // different base types are incompatible - nothing is going to make them equal
    if (lhs->get_base_type() != rhs->get_base_type()) {
        return true;
    }
    
    // same base type, need to check structural compatibility
    if (lhs->get_base_type() == TYPE::FUNCTION) {
        // functions with different arities are incompatible (since we do not allow Currying or anything)
        if (lhs->get_num_of_sources() != rhs->get_num_of_sources()) {
            return true;
        }
        
        // check argument and return type compatibility recursively
        for (int i = 0; i < lhs->get_num_of_sources(); i++) {
            PType lhs_src = equiv.find_representative(lhs->get_source_type(i));
            PType rhs_src = equiv.find_representative(rhs->get_source_type(i));
            if (types_structurally_incompatible(lhs_src, rhs_src)) {
                return true;
            }
        }
        
        PType lhs_dest = equiv.find_representative(lhs->get_dest_type());
        PType rhs_dest = equiv.find_representative(rhs->get_dest_type());
        if (types_structurally_incompatible(lhs_dest, rhs_dest)) {
            return true;
        }
    }
    
    if (lhs->get_base_type() == TYPE::LIST) {
        PType lhs_elem = equiv.find_representative(lhs->get_dest_type());
        PType rhs_elem = equiv.find_representative(rhs->get_dest_type());
        if (types_structurally_incompatible(lhs_elem, rhs_elem)) {
            return true;
        }
    }
    
    return false;
}

bool Typer::types_subtype_incompatible(PType sub, PType super) {
    sub = equiv.find_representative(sub);
    super = equiv.find_representative(super);

    // Generics can still be constrained later.
    if (sub->get_base_type() == TYPE::GENERIC ||
        super->get_base_type() == TYPE::GENERIC) {
        return false;
    }

    // Function subtyping:
    // (A -> B) <: (C -> D) iff C <: A and B <: D
    if (sub->get_base_type() == TYPE::FUNCTION && super->get_base_type() == TYPE::FUNCTION) {
        if (sub->get_num_of_sources() != super->get_num_of_sources()) return true;

        for (int i = 0; i < sub->get_num_of_sources(); i++) {
            // contravariant argument position
            if (types_subtype_incompatible(super->get_source_type(i), sub->get_source_type(i))) {
                return true;
            }
        }

        // covariant return position
        return types_subtype_incompatible(sub->get_dest_type(), super->get_dest_type());
    }

    // List subtyping is covariant
    if (sub->get_base_type() == TYPE::LIST && super->get_base_type() == TYPE::LIST) {
        return types_subtype_incompatible(sub->get_dest_type(), super->get_dest_type());
    }

    // Core is transitive - if sub is L-structured, super is L<d>, then sub <: L<d> iff core(sub) <: d
    if (super->get_base_type() == TYPE::LIST && sub->get_base_type() != TYPE::LIST) {
        // check if sub has known List-contents
        auto core = list_content_set.get_core(sub, subtyping);
        if (core.has_value()) {
            // sub is List-structured with known core
            // sub <: List⟨d⟩ iff core(sub) <: d (Core Transitivity)
            return types_subtype_incompatible(*core, super->get_dest_type());
        }
        // If sub could potentially be List-structured (generic constraints), 
        // not incompatible
        return !list_content_set.is_structured(sub) && 
               !list_declared_structure.could_be_subtype(sub, super);
    }

    // (Sep) — List <: Function or Function <: List is impossible
    if ((sub->get_base_type() == TYPE::LIST && super->get_base_type() == TYPE::FUNCTION) 
        ||(sub->get_base_type() == TYPE::FUNCTION && super->get_base_type() == TYPE::LIST)) {
        return true;
    }

    // Different non-constructor base types: check declared relationships
    if (sub->get_base_type() != super->get_base_type()) {
        // Check if there's a declared path
        return !list_declared_structure.could_be_subtype(sub, super);
    }

    return false;
}


bool Typer::types_hereditarily_incompatible(PType lhs_in, PType rhs_in) {
    PType lhs = equiv.find_representative(lhs_in);
    PType rhs = equiv.find_representative(rhs_in);

    if (lhs->get_base_type() == TYPE::GENERIC || rhs->get_base_type() == TYPE::GENERIC) {
        return false;
    }

    // Try every unwrapping level of lhs via core
    PType current = lhs;

    while (true) {
        current = equiv.find_representative(current);

        if (current->get_base_type() == TYPE::GENERIC) return false;

        if (!types_subtype_incompatible(current, rhs)) return false;

        auto core = get_list_core(current);
        if (!core.has_value()) break;  // not List-structured: stop
        
        current = *core;
    }

    return true;
}

bool Typer::types_structurally_equal(PType lhs_in, PType rhs_in) {
    PType lhs = equiv.find_representative(lhs_in);
    PType rhs = equiv.find_representative(rhs_in);

    if (lhs == rhs) {
        return true;
    }

    if (lhs->get_base_type() != rhs->get_base_type()) {
        return false;
    }

    switch (lhs->get_base_type()) {
        case TYPE::GENERIC:
            // two generics should not be assumed equal
            return false;

        case TYPE::LIST:
            return types_structurally_equal(lhs->get_dest_type(), rhs->get_dest_type());

        case TYPE::FUNCTION: 
        {
            if (lhs->get_num_of_sources() != rhs->get_num_of_sources()) {
                return false;
            }

            for (int i = 0; i < lhs->get_num_of_sources(); i++) {
                if (!types_structurally_equal(lhs->get_source_type(i), rhs->get_source_type(i))) {
                    return false;
                }
            }

            return types_structurally_equal(lhs->get_dest_type(), rhs->get_dest_type());
        }

        default:
            return true;
    }
}

Ternary Typer::truth_of_premise(PType lhs_in, PType rhs_in, StatementForm form) {
    // get representatives, this is the canonical form from our equivalence classes
    PType lhs_rep = equiv.find_representative(lhs_in);
    PType rhs_rep = equiv.find_representative(rhs_in);
    
    switch (form) {
        case STATEMENT::EQUALITY: {
            // same representative necessarily means equal
            if (lhs_rep == rhs_rep) {
                return Ternary::TERN_TRUE;
            }
            
            // structural incompatibility means there is no way for them to be equal
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT::INEQUALITY: {
            // same representative means equal, so inequality is false
            if (lhs_rep == rhs_rep) {
                return Ternary::TERN_FALSE;
            }
            
            // structural incompatibility means definitely unequal
            if (types_structurally_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT::SUBTYPE: {
            // manifestly a subtype
            if (subtyping.is_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            // incompatable and so impossible for them to be a subtype without having been explicitly declared as such
            if (types_subtype_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT::NONSUBTYPE: {
            if (subtyping.is_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            if (types_subtype_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT::SUPERTYPE: {
            return truth_of_premise(rhs_in, lhs_in, STATEMENT::SUBTYPE);
        }
        
        case STATEMENT::NONSUPERTYPE: {
            return truth_of_premise(rhs_in, lhs_in, STATEMENT::NONSUBTYPE);
        }
        
        case STATEMENT::HEREDITARY_SUBTYPE: {
            if (hereditary_subtyping.is_deep_subtype(lhs_rep, rhs_rep)) {
                return Ternary::TERN_TRUE;
            }
            // cannot ever be hereditarily matched
            if (types_hereditarily_incompatible(lhs_rep, rhs_rep)) {
                return Ternary::TERN_FALSE;
            }
            
            return Ternary::TERN_UNKNOWN;
        }
        
        case STATEMENT::HEREDITARY_SUPERTYPE: {
            return truth_of_premise(rhs_in, lhs_in, STATEMENT::HEREDITARY_SUBTYPE);
        }
        
        case STATEMENT::EQUAL_DEPTH: {
            return similarity.have_similar(lhs_rep, rhs_rep);
        }
        case STATEMENT::NOT_EQUAL_DEPTH: {
            Ternary equal_dep = similarity.have_similar(lhs_rep, rhs_rep);
            if (equal_dep == Ternary::TERN_TRUE) return Ternary::TERN_FALSE;
            else if (equal_dep == Ternary::TERN_FALSE) return Ternary::TERN_TRUE;
            else return Ternary::TERN_UNKNOWN;
        }
        case StatementForm::IS_LIST_STRUCTURED: {
            return similarity.is_structured_query(lhs_rep);
        }
        case StatementForm::IS_LIST_FREE: {
            return similarity.is_free_query(lhs_rep);
        }
        case StatementForm::IS_NOT_LIST_STRUCTURED: {
            Ternary struct_check = similarity.is_structured_query(lhs_rep);
            if (struct_check == Ternary::TERN_TRUE) return Ternary::TERN_FALSE;
            else if (struct_check == Ternary::TERN_FALSE) return Ternary::TERN_TRUE;
            else return Ternary::TERN_UNKNOWN;
        }
        case StatementForm::IS_NOT_LIST_FREE: {
            Ternary free_check = similarity.is_free_query(lhs_rep);
            if (free_check == Ternary::TERN_TRUE) return Ternary::TERN_FALSE;
            else if (free_check == Ternary::TERN_FALSE) return Ternary::TERN_TRUE;
            else return Ternary::TERN_UNKNOWN;
        }
        }

    return Ternary::TERN_UNKNOWN;
}


bool Typer::propagate_once() {
    bool changed = false;

    // first, antisymmetry
    {
        auto all_supers = subtyping.get_all_supertypes();
        std::vector<std::pair<PType, PType>> to_equate;

        for (const auto &[sub, supers] : all_supers) {
            PType sr = equiv.find_representative(sub);
            for (PType super : supers) {
                PType spr = equiv.find_representative(super);
                if (sr == spr) continue;
                if (subtyping.is_subtype(spr, sr))
                    to_equate.emplace_back(sr, spr);
            }
        }

        for (const auto &[a, b] : to_equate) {
            PType ar = equiv.find_representative(a);
            PType br = equiv.find_representative(b);
            if (ar == br) continue;
            if (!constrain_equality(ar, br))
                raise_type_exception("Antisymmetry cycle forces incompatible equality",
                                     ar->get_origin_token());
            changed = true;
        }
    }

    // From axiom of hereditary subtyping
    // (a <<: b) and (a ~= b) and core(a) <: core(b) ==> a <: b
    // We check all pairs in the hereditary subtyping relation.
    // For pairs where similarity holds and we can verify
    // the core condition, we derive ordinary subtyping.
    {
        const auto &all_deep = hereditary_subtyping.get_all_edges();

        for (const auto &[sub_orig, deep_supers] : all_deep) {
            PType a = equiv.find_representative(sub_orig);

            for (PType super_orig : deep_supers) {
                PType b = equiv.find_representative(super_orig);
                if (a == b) continue;
                if (subtyping.is_subtype(a, b)) continue;

                // Precondition: a ≃ b
                if (!similarity.are_similar(a, b)) continue;

                auto core_a = get_list_core(a);
                auto core_b = get_list_core(b);

                if (core_a && core_b) {
                    // Both L-structured.
                    //
                    // By Lemma 2 (Project to Core):
                    //   a ≪: b, both structured ⟹ core(a) ≪: core(b)
                    // Record this derived fact:
                    hereditary_subtyping.add_deep_subtype(
                        equiv.find_representative(*core_a),
                        equiv.find_representative(*core_b));

                    // Now check if core(a) <: core(b) holds
                    // (either already known, or derivable by previous
                    //  propagation rounds at lower depth)
                    if (subtyping.is_subtype(
                            equiv.find_representative(*core_a),
                            equiv.find_representative(*core_b))) {
                        // (S): a ≪: b ∧ a ≃ b ∧ core(a) <: core(b) ⟹ a <: b
                        if (constrain_subtype(a, b))
                            changed = true;
                    }
                } else if (!core_a && !core_b) {
                    // Both L-free.
                    //
                    // Lemma: L-free(a) ∧ a ≪: b ⟹ a <: b
                    // Proof: a ≪: b by Def 4 is a <: b ∨ ∃c(a <: L⟨c⟩ ∧ c ≪: b).
                    //   If a is L-free, a <: L⟨c⟩ would make a L-structured — contradiction.
                    //   So a <: b must hold. □
                    //
                    // Record it:
                    if (constrain_subtype(a, b))
                        changed = true;
                }
                // Mixed (one free, one structured): similarity should be false,
                // so the `are_similar` check above already filtered this out.
            }
        }
    }

    {
        auto all_subs = subtyping.get_all_subtypes();
        for (const auto &[super, subs] : all_subs) {
            PType sr = equiv.find_representative(super);

            if (similarity.is_structured_query(sr) != Ternary::TERN_TRUE) continue;
    
            for (PType sub : subs) {
                PType sb = equiv.find_representative(sub);
                if (sr == sb) continue;
                if (similarity.is_structured_query(sb) == Ternary::TERN_TRUE) continue;

                if (similarity.assert_structured(sb)) changed = true;
                // a <: b ⟹ a ≪: b
                // hereditary_subtyping.add_deep_subtype(sr, spr);
            }
        }
    }

    // We propagate known <: facts into ≪: so that transitive
    // closure in DeepSubtypeRelation picks them up.
    {
        auto all_supers = subtyping.get_all_supertypes();
        for (const auto &[sub, supers] : all_supers) {
            PType sr = equiv.find_representative(sub);

            if (similarity.is_free_query(sr) != Ternary::TERN_TRUE) continue;
    
            for (PType super : supers) {
                PType spr = equiv.find_representative(super);
                if (sr == spr) continue;
                if (similarity.is_free_query(spr) == Ternary::TERN_TRUE) continue;

                if (similarity.assert_free(spr)) changed = true;
                // a <: b ⟹ a ≪: b
                hereditary_subtyping.add_deep_subtype(sr, spr);
            }
        }
    }


    // structural identity, types with identical constructor structure are equal
    {
        auto reps = equiv.get_all_representatives();
        std::vector<PType> rep_vec(reps.begin(), reps.end());
        for (size_t i = 0; i < rep_vec.size(); i++) {
            for (size_t j = i + 1; j < rep_vec.size(); j++) {
                if (types_structurally_equal(rep_vec[i], rep_vec[j])) {
                    if (!constrain_equality(rep_vec[i], rep_vec[j]))
                        return false; // will be caught as error
                    changed = true;
                }
            }
        }
    }

    return changed;
}

void Typer::propagate_to_fixed_point() {
    bool should_continue = true;
    do {
        should_continue = propagate_once();
    } while (should_continue);
}

void Typer::apply_conclusion(Statement &conclusion, Constraint &constraint) {
    auto first = conclusion.get_lhs();
    auto second = conclusion.get_rhs();

    auto type_string = [&](PType t) -> std::string {
        std::stringstream ss;
        auto *old = std::cout.rdbuf(ss.rdbuf());
        t->print();
        std::cout.rdbuf(old);
        return ss.str();
    };

    std::string s1 = type_string(first);
    std::string s2 = type_string(second);
    PToken tok = constraint.get_originator_type().lock()->get_origin_token();

    switch (conclusion.get_form()) {
        case STATEMENT::EQUALITY:
            // Assert: first = second
            if (!constrain_equality(first, second))
                raise_type_exception("Cannot equate " + s1 + " with " + s2, tok);
            break;

        case STATEMENT::SUBTYPE:
            // Assert: first <: second
            if (!constrain_subtype(first, second))
                raise_type_exception("Failed subtype " + s1 + " <: " + s2, tok);
            break;

        case STATEMENT::SUPERTYPE:
            // Assert: second <: first
            if (!constrain_subtype(second, first))
                raise_type_exception("Failed supertype " + s1 + " :> " + s2, tok);
            break;

        case STATEMENT::HEREDITARY_SUBTYPE:
            // Assert: first <<: second
            if (!constrain_hereditary_subtype(first, second))
                raise_type_exception("Failed deep subtype " + s1 + " <<: " + s2, tok);
            break;

        case STATEMENT::HEREDITARY_SUPERTYPE:
            // Assert: second <<: first
            if (!constrain_hereditary_subtype(second, first))
                raise_type_exception("Failed deep supertype " + s1 + " :>> " + s2, tok);
            break;

        case STATEMENT::EQUAL_DEPTH:
            // Assert: first ~= second
            if (!constrain_similar_depth(first, second))
                raise_type_exception("Failed similarity " + s1 + " ~= " + s2, tok);
            break;
        
        case STATEMENT::IS_LIST_STRUCTURED:
            if (!similarity.assert_structured(first))
                raise_type_exception("Cannot assert " + s1 + " is List-structured", tok);
            break; 
        
        case STATEMENT::IS_LIST_FREE:
            if (!similarity.assert_free(first))
                raise_type_exception("Cannot assert " + s1 + " is List-free", tok);
            break; 

        default:
            // Negative constraints (NONSUBTYPE, etc.) cannot be asserted, only checked. These remain as deferred obligations.
            break;
    }
}

void Typer::resolve_constraints() {


    std::cout << "\n Resolving constraints... \n";

    bool made_progress = true;

    while (made_progress) {
        made_progress = false;
        std::vector<Constraint> deferred;

        for (auto &constraint : running_valid_constraints) {
            // Evaluate conjunction of premises
            Ternary premises_hold = Ternary::TERN_TRUE;

            for (auto &premise : constraint.get_premises()) {
                PType lr = equiv.find_representative(premise.get_lhs());
                PType rr = equiv.find_representative(premise.get_rhs());
                premises_hold.eq_land(truth_of_premise(lr, rr, premise.get_form()));
            }

            if (premises_hold == Ternary::TERN_FALSE) {
                // Premises falsified — constraint is vacuously satisfied
                continue;
            }

            if (premises_hold == Ternary::TERN_UNKNOWN) {
                // Cannot yet determine — defer
                deferred.push_back(constraint);
                continue;
            }

            // if premises definitely: apply all conclusions
            for (auto &conclusion : constraint.get_conclusions()) {
                if (conclusion.get_form() == STATEMENT::NONSUBTYPE ||
                    conclusion.get_form() == STATEMENT::NONSUPERTYPE ||
                    conclusion.get_form() == STATEMENT::INEQUALITY) {
                    // Negative conclusions are verification obligations, not assertions.
                    // Check them; if violated, that's a type error.
                    // If unknown, defer the whole constraint.
                    Ternary check = truth_of_premise(
                        equiv.find_representative(conclusion.get_lhs()),
                        equiv.find_representative(conclusion.get_rhs()),
                        conclusion.get_form());

                    if (check == Ternary::TERN_FALSE) {
                        raise_type_exception(
                            "Negative constraint violated",
                            constraint.get_originator_type().lock()->get_origin_token());
                    }
                    if (check == Ternary::TERN_UNKNOWN) {
                        deferred.push_back(constraint);
                        continue; // defer entire constraint
                    }
                    // TERN_TRUE: constraint satisfied, nothing to do
                } else {
                    apply_conclusion(conclusion, constraint);
                    made_progress = true;
                }
            }
        }

        running_valid_constraints = deferred;

        // After applying new facts, propagate all derived consequences
        propagate_to_fixed_point();
    }

    auto equalities = equiv.resolve_all();


    std::cout << "\n Constraints resolved.\n";

    for (auto variable : order_of_variables) {
        auto type_of_var = types_of_variables[variable];

        if (used_variables.count(variable) == 0) {
            std::cout << "UNUSED ";
        }

        std::cout << variable << " : ";

        if (equalities.contains(type_of_var)) type_of_var = equalities[type_of_var];
        type_of_var->print(equalities);
        
        std::cout << "\n";
    }
    std::cout << std::endl;

    for (auto constraint : running_valid_constraints) {
        constraint.print(equalities);
        std::cout << "\n";
    }

}


void Typer::collect_existing_constraints() {

    std::regex reg_string;
    std::regex reg_number;

    reg_number = std::regex("-{0,1}[0-9]*\\.{0,1}[0-9]*([Ee][-+]{0,1}[0-9]+){0,1}");
    reg_string= std::regex("\"[^\"]*\"");

    for (auto &command : alil->get_commands()) {

        auto type_of_function = command_convert(command);
        all_live_types.push_back(type_of_function);
        type_of_function->set_origin_token(command.get_token());
        type_of_function->print();

        // all the constraints this type comes with, we add to ours
        for (auto constraint : type_of_function->get_constraints()) {
            running_valid_constraints.push_back(constraint);
        }

        // generate a series of constraints for the inputs to match with the 
        for (int i = 0; i < command.get_num_arguments() - (command.has_dest_argument() ? 1 : 0); i++) {
            std::string arg = command.get_source_argument(i);

            used_variables.emplace(arg);

            PType type_of_param = type_of_function->get_source_type(i);
            
            PType type_of_arg;
            if (types_of_variables.count(arg) == 0) {
                if (std::regex_match(arg, reg_string)) {
                    type_of_arg = Type::fundamental_type_instance(TYPE::STRING);
                } else if (std::regex_match(arg, reg_number)) {
                    type_of_arg = Type::fundamental_type_instance(TYPE::NUMBER);
                } else {
                    // error condition - variable is undefined
                    // std::cerr << arg << std::endl;
                    if (false) { //TODO: add the actual config
                        raise_type_exception("The variable " + arg + " used in this function is undefined - use the automatic setting if you want this to automatically infer a type for it", command.get_token());
                    } else {
                        type_of_arg = std::make_shared<Type>(TYPE::GENERIC);
                        types_of_variables[arg] = type_of_arg;
                    }
                }
            } else {
                type_of_arg = types_of_variables[arg];
            }
            Constraint compatability_of_input(type_of_function);
            if (type_of_function->get_num_of_sources() <= i) {
                std::cerr << type_of_function->get_num_of_sources();
                assert(false);
            } 
            if (type_of_param->get_base_type() == TYPE::GENERIC) {
                compatability_of_input.add_conclusion(Statement(STATEMENT::EQUALITY, type_of_arg, type_of_function->get_source_type(i)));

            } else {
                compatability_of_input.add_conclusion(Statement(STATEMENT::SUBTYPE, type_of_arg, type_of_function->get_source_type(i)));

            }
            // equality_of_input.print();
            running_valid_constraints.push_back(compatability_of_input);
        }


        if (command.has_dest_argument()) {
            // the destination necessarily has the type of the codomain of the function in question
            order_of_variables.push_back(command.get_dest_argument());
            types_of_variables.emplace(command.get_dest_argument(), type_of_function->get_dest_type());
        }

    

    }

    int i = 0;
    for (auto constraint : running_valid_constraints) {
        std::cout << i++ << "|";
        constraint.print();
        std::cout << "\n";
    }
    std::cout << std::endl;
}


void Typer::print() {

    Type::highest_mapped_generic = 0;
    Type::generic_map.clear();
    types_of_variables.clear();

    collect_existing_constraints();

    // for (int i = 0; i < 6; i++) {
    //     std::cout << i+1 << "th pass \n" << std::endl;
    //     resolve_constraints();
    // }

    resolve_constraints();

    // std::cout << "\n\n\n" << "Second pass \n" << std::endl;

    // resolve_constraints();

    // std::cout << "\n\n\n" << "Thirdm pass \n" << std::endl;

    // resolve_constraints();

    auto equals = equiv.resolve_all();

    for (auto command : alil->get_commands()) {
        if (command.has_dest_argument()) {
            auto var_type = equiv.find_representative(types_of_variables[command.get_dest_argument()]);
            if (var_type->get_base_type() == TYPE::GENERIC) {
 
                auto subtypes = subtyping.get_subtypes(var_type);
                auto supertypes = subtyping.get_supertypes(var_type);

                int num_non_generic_supertypes = 0;
                for (auto type : supertypes) {
                    if (type->get_base_type() == TYPE::GENERIC) continue;
                    num_non_generic_supertypes ++;
                }

                int num_non_generic_subtypes = 0;
                for (auto type: subtypes) {
                    if (type->get_base_type() == TYPE::GENERIC) continue;
                    num_non_generic_subtypes ++;
                }

                if (num_non_generic_supertypes == 1) {
                    for (auto type : supertypes) {
                        if (type->get_base_type() == TYPE::GENERIC) continue;
                        std::cout << " = ";
                        type->print();
                    }
                } else if (num_non_generic_subtypes == 1) {
                    for (auto type : subtypes) {
                        if (type->get_base_type() == TYPE::GENERIC) continue;
                        std::cout << " = ";
                        type->print();
                    }
                } else {
                    for (auto type : subtyping.get_subtypes(var_type)) {
                        if (type->get_base_type() == TYPE::GENERIC) continue;
                        type->print();
                        std::cout << " <: ";
                    }
                    var_type->print();
                    for (auto type : supertypes) {
                        if (type->get_base_type() == TYPE::GENERIC) continue;
                        std::cout << " <: ";
                        type->print();
                    }
                }   
            } else {
                var_type->print(equals);
            }

        } else {
            std::cout << "void";
        }
        std::cout << " : ";
        command.print_instruction();
        std::cout << std::endl;
    }

}
