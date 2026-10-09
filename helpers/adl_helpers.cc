#include "Math/Vector4Dfwd.h"
#include <ROOT/RVec.hxx>
#include <Math/LorentzVector.h>
#include <Math/PtEtaPhiM4D.h>
#include <cassert>
#include <utility>
#include <functional>

using namespace ROOT::VecOps; 

const int ALL = 1;
const int NONE = 0;

RVec<int> create_mask(RVec<float> in) {
    // begin all masks as including all elements
    auto start_true_lamb = [](float value_m){
            return 1;
         };

    auto starter = Map(in, start_true_lamb);
    return starter;
}

RVec<int> limit_mask(RVec<int> mask, RVec<int> conditions) {
    auto zero_lamb = [](int mask_m, int cond_m){
            if (cond_m) return mask_m;
            return 0;
         };

    auto value_zeroed = Map(mask, conditions, zero_lamb);
    return value_zeroed;
}

RVec<float> apply_mask(RVec<int> mask, RVec<float> value) {

    auto invert_mask_lamb = [](bool mask_m) {
        if (mask_m) return 0;
        return 1;
    };

    auto inverted_mask = Map(mask, invert_mask_lamb);
    auto zero_elem = Nonzero(inverted_mask);
    auto masked = Drop(value, zero_elem);
    return masked;
}

RVec<float> empty_union() {
    return RVec<float>();
}

RVec<float> union_merge(RVec<float> val1, RVec<float> val2) {
    return Concatenate(val1, val2);
}

RVec<float> index_get(RVec<float> value, int start, int end) {
    RVec<int> indices;

    if (end == 0) end = value.size();

    for (int i = 0; i < static_cast<int>(value.size()); i++) {
        indices.push_back((i < end && i >= start) ? 1 : 0);
    }

    return value[indices];
}

float index_get(RVec<float> value, int index) {
    return value[index];
}

RVec<float> index_from(RVec<float> value, int start) {
    return index_get(value, start, 0);
}

RVec<float> index_until(RVec<float> value, int end) {
    return index_get(value, 0, end);
}


int size(RVec<float> vec) {
    return vec.size();
}

bool AnyOccurrences(float value, RVec<float> to_compare) {
    return Any(value == to_compare); 
}

RVec<bool> AnyOccurrences(RVec<float> values, RVec<float> to_compare) {
    auto is_in_lamb = [to_compare](float value) {
        return AnyOccurrences(value, to_compare);
    };
    auto is_in_vec = Map(values, is_in_lamb);
    return is_in_vec;
} 

float LVDeltaR(ROOT::Math::PtEtaPhiMVector v1, ROOT::Math::PtEtaPhiMVector v2) {
    float deta = v1.Eta()-v2.Eta();
    float dphi = DeltaPhi(v1.Phi(),v2.Phi());
    return sqrt(deta*deta+dphi*dphi);  
}

RVec<float> LVDeltaRHadamard(RVec<ROOT::Math::PtEtaPhiMVector> v1, RVec<ROOT::Math::PtEtaPhiMVector> v2) {
    if (v1.size() != v2.size()) {
        throw std::runtime_error("LVDeltaRHadamard: input vectors have different lengths");
    }
    auto dR_lamb = [](ROOT::Math::PtEtaPhiMVector vec1, ROOT::Math::PtEtaPhiMVector vec2) {
        return LVDeltaR(vec1, vec2);
    };
    auto dR_vec = Map(v1, v2, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaR(RVec<ROOT::Math::PtEtaPhiMVector> list_vec, ROOT::Math::PtEtaPhiMVector single_vec) {
    auto dR_lamb = [single_vec](ROOT::Math::PtEtaPhiMVector v1) {
        return LVDeltaR(v1, single_vec);
    };
    auto dR_vec = Map(list_vec, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaR(ROOT::Math::PtEtaPhiMVector single_vec, RVec<ROOT::Math::PtEtaPhiMVector> list_vec) {
    return LVDeltaR(list_vec, single_vec);
}


RVec<RVec<float>> LVDeltaROuter(RVec<ROOT::Math::PtEtaPhiMVector> v1, RVec<ROOT::Math::PtEtaPhiMVector> v2) {
    auto dR_lamb = [v2](ROOT::Math::PtEtaPhiMVector single_vec) {
        return LVDeltaR(v2, single_vec);
    };
    auto dR_matrix = Map(v1, dR_lamb);
    return dR_matrix;
}

RVec<RVec<float>> LVDeltaR(RVec<ROOT::Math::PtEtaPhiMVector> v1, RVec<ROOT::Math::PtEtaPhiMVector> v2) {
    return LVDeltaROuter(v1, v2);
}

int AnyOf(RVec<int> vec) { 
    return static_cast<int>(Any(vec));
}

RVec<int> AnyOf(RVec<RVec<int>> matrix) {
    auto any_lamb = [](RVec<int> row) {
        return AnyOf(row);
    };
    auto truth_list = Map(matrix, any_lamb);
    return truth_list;
}

int AllOf(RVec<int> vec) {
    return static_cast<int>(All(vec));
}

RVec<int> AllOf(RVec<RVec<int>> matrix) {
    auto all_lamb = [](RVec<int> row) {
        return AllOf(row);
    };
    auto truth_list = Map(matrix, all_lamb);
    return truth_list;
}


float MaxOf(RVec<float> vec) {
    return Max(vec);
}

RVec<float> MaxOf(RVec<RVec<float>> matrix) {
    auto max_lamb = [](RVec<float> row) {
        return MaxOf(row);
    };
    auto truth_list = Map(matrix, max_lamb);
    return truth_list;
}

float MinOf(RVec<float> vec) {
    return Min(vec);
}

RVec<float> MinOf(RVec<RVec<float>> matrix) {
    auto min_lamb = [](RVec<float> row) {
        return MinOf(row);
    };
    return Map(matrix, min_lamb);
}

float MeanOf(RVec<float> vec) {
    return Mean(vec);
}

RVec<float> MeanOf(RVec<RVec<float>> matrix) {
    auto mean_lamb = [](RVec<float> row) {
        return MeanOf(row);
    };
    return Map(matrix, mean_lamb);
}

float SumOf(RVec<float> vec) {
    return Sum(vec);
}

RVec<float> SumOf(RVec<RVec<float>> matrix) {
    auto sum_lamb = [](RVec<float> row) {
        return SumOf(row);
    };
    return Map(matrix, sum_lamb);
}


// a truly horrifying solution to a problem, the base RVec code forces the return type to be an RVec<int>, while we really need RVec<RVec<int>> for our matrix-like situations.
// here we force this new code into the VecOps namespace to act as a more specific template to force the interpreter to go with this one, giving us the right return type.
namespace ROOT { namespace VecOps {
#define RVEC_NEW_LOGICAL_OPERATOR(OP)                           \
template <typename T1>                                          \
auto operator OP(const RVec<RVec<float>> &lhs, const T1 &rhs) { \
    auto lt_lamb = [rhs] (RVec<float> one_vec){                 \
        return one_vec OP rhs;                                  \
    };                                                          \
    RVec<RVec<int>> lt_vec = Map(lhs, lt_lamb);                 \
    return lt_vec;                                              \
}                                                               \

RVEC_NEW_LOGICAL_OPERATOR(<)
RVEC_NEW_LOGICAL_OPERATOR(>)
RVEC_NEW_LOGICAL_OPERATOR(==)
RVEC_NEW_LOGICAL_OPERATOR(!=)
RVEC_NEW_LOGICAL_OPERATOR(<=)
RVEC_NEW_LOGICAL_OPERATOR(>=)
RVEC_NEW_LOGICAL_OPERATOR(&&)
RVEC_NEW_LOGICAL_OPERATOR(||)

#undef RVEC_NEW_LOGICAL_OPERATOR
}}


float LVDeltaPhi(ROOT::Math::PtEtaPhiMVector v1, ROOT::Math::PtEtaPhiMVector v2) {
    float dphi = DeltaPhi(v1.Phi(),v2.Phi());
    return dphi;
}

RVec<float> LVDeltaPhiHadamard(RVec<ROOT::Math::PtEtaPhiMVector> v1, RVec<ROOT::Math::PtEtaPhiMVector> v2) {
    if (v1.size() != v2.size()) {
        throw std::runtime_error("LVDeltaPhiHadamard: input vectors have different lengths");
    }
    auto dR_lamb = [](ROOT::Math::PtEtaPhiMVector vec1, ROOT::Math::PtEtaPhiMVector vec2) {
        return LVDeltaPhi(vec1, vec2);
    };
    auto dR_vec = Map(v1, v2, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaPhi(RVec<ROOT::Math::PtEtaPhiMVector> list_vec, ROOT::Math::PtEtaPhiMVector single_vec) {
    auto dR_lamb = [single_vec](ROOT::Math::PtEtaPhiMVector v1) {
        return LVDeltaPhi(v1, single_vec);
    };
    auto dR_vec = Map(list_vec, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaPhi(ROOT::Math::PtEtaPhiMVector single_vec, RVec<ROOT::Math::PtEtaPhiMVector> list_vec) {
    return LVDeltaPhi(list_vec, single_vec);
}

float LVDeltaEta(ROOT::Math::PtEtaPhiMVector v1, ROOT::Math::PtEtaPhiMVector v2) {
    float deta = v1.Eta()-v2.Eta();
    return deta;  
}

RVec<float> LVDeltaEtaHadamard(RVec<ROOT::Math::PtEtaPhiMVector> v1, RVec<ROOT::Math::PtEtaPhiMVector> v2) {
    if (v1.size() != v2.size()) {
        throw std::runtime_error("LVDeltaEtaHadamard: input vectors have different lengths");
    }
    auto dR_lamb = [](ROOT::Math::PtEtaPhiMVector vec1, ROOT::Math::PtEtaPhiMVector vec2) {
        return LVDeltaEta(vec1, vec2);
    };
    auto dR_vec = Map(v1, v2, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaEta(RVec<ROOT::Math::PtEtaPhiMVector> list_vec, ROOT::Math::PtEtaPhiMVector single_vec) {
    auto dR_lamb = [single_vec](ROOT::Math::PtEtaPhiMVector v1) {
        return LVDeltaEta(v1, single_vec);
    };
    auto dR_vec = Map(list_vec, dR_lamb);
    return dR_vec;
}

RVec<float> LVDeltaEta(ROOT::Math::PtEtaPhiMVector single_vec, RVec<ROOT::Math::PtEtaPhiMVector> list_vec) {
    return LVDeltaEta(list_vec, single_vec);
}

ROOT::Math::PtEtaPhiMVector TLV(float pt, float eta, float phi, float m) {
    return ROOT::Math::PtEtaPhiMVector(pt, eta, phi, m);
}

RVec<ROOT::Math::PtEtaPhiMVector> TLV(RVec<float> pt,RVec<float> eta,RVec<float> phi,RVec<float> m) {
    if (pt.size() != eta.size() || pt.size() != phi.size() || pt.size() != m.size()) {
        throw std::runtime_error("TLV: input component vectors have different lengths");
    }
    RVec<ROOT::Math::PtEtaPhiMVector> vs;
    vs.reserve(pt.size());
    for (size_t i = 0; i < pt.size(); i++) {
        vs.emplace_back(pt[i],eta[i],phi[i],m[i]);
    }
    return vs;
}

float M(ROOT::Math::PtEtaPhiMVector vec) {
    return vec.M();
}

RVec<float> M(RVec<ROOT::Math::PtEtaPhiMVector> vecs) {
    auto M_lamb = [](ROOT::Math::PtEtaPhiMVector vec) {
        return vec.M();
    };
    auto M_vec = Map(vecs, M_lamb);
    return M_vec;
}

float Pt(ROOT::Math::PtEtaPhiMVector vec) {
    return vec.Pt();
}

RVec<float> Pt(RVec<ROOT::Math::PtEtaPhiMVector> vecs) {
    auto Pt_lamb = [](ROOT::Math::PtEtaPhiMVector vec) {
        return vec.Pt();
    };
    auto Pt_vec = Map(vecs, Pt_lamb);
    return Pt_vec;
}

float Eta(ROOT::Math::PtEtaPhiMVector vec) {
    return vec.Eta();
}

RVec<float> Eta(RVec<ROOT::Math::PtEtaPhiMVector> vecs) {
    auto Eta_lamb = [](ROOT::Math::PtEtaPhiMVector vec) {
        return vec.Eta();
    };
    auto Eta_vec = Map(vecs, Eta_lamb);
    return Eta_vec;
}

float Phi(ROOT::Math::PtEtaPhiMVector vec) {
    return vec.Phi();
}

RVec<float> Phi(RVec<ROOT::Math::PtEtaPhiMVector> vecs) {
    auto Phi_lamb = [](ROOT::Math::PtEtaPhiMVector vec) {
        return vec.Phi();
    };
    auto Phi_vec = Map(vecs, Phi_lamb);
    return Phi_vec;
}

RVec<RVec<unsigned long>> MakeEmptyShape(unsigned long n) {
    RVec<RVec<unsigned long>> out;
    out.reserve(n);
    for (unsigned long i = 0; i < n; ++i) {
        out.push_back(RVec<unsigned long>{});
    }
    return out;
}

RVec<RVec<unsigned long>> ExpandComb(RVec<RVec<unsigned long>>  &input_tuple, RVec<float> &new_vector) {

    // If there is nothing to expand from, preserve shape but return empty content.
    if (input_tuple.empty() || new_vector.empty()) {
        return MakeEmptyShape(input_tuple.size() + 1);
    }

    auto indices = Combinations(input_tuple[0], new_vector);
    auto indices_for_new = indices[1];
    auto indices_for_all_old = indices[0];

    RVec<RVec<unsigned long>> new_indices;
    new_indices.reserve(input_tuple.size() + 1);

    for (auto single_old_index_list : input_tuple) {
        auto new_list = Take(single_old_index_list, indices_for_all_old);
        new_indices.push_back(new_list);
    }

    new_indices.push_back(indices_for_new);
    return new_indices;
}

RVec<RVec<unsigned long>> GeneralComb(RVec<RVec<float>> input_particles) {

    if (input_particles.empty()) {
        return {};
    }

    // If any particle collection is empty, there are no valid combinations.
    // Return the correct "shape" (one entry per particle), but empty.
    for (const auto &p : input_particles) {
        if (p.empty()) {
            return MakeEmptyShape(input_particles.size());
        }
    }

    if (input_particles.size() == 1) return Combinations(input_particles[0],1);

    RVec<RVec<unsigned long>> new_indices = Combinations(input_particles[0], input_particles[1]);
    for (unsigned long i = 2; i < input_particles.size(); i++) {
        new_indices = ExpandComb(new_indices, input_particles[i]);
    }
    return new_indices;
}


RVec<RVec<unsigned long>> GeneralDisjoint(RVec<RVec<float>> input_particles) {
    
    unsigned long particle_size = 0;
    unsigned long count = 0;

    for (auto it = input_particles.cbegin(); it != input_particles.cend(); ++it, ++count) {
        if (particle_size == 0) particle_size = it->size();
        assert(it->size() == particle_size);
    }

    // if the disjoint is on empty vectors, return the same shape as the input and similarly empty
    if (particle_size == 0) {
        return MakeEmptyShape(input_particles.size());
    }

    return ROOT::VecOps::Combinations(input_particles[0], count);
}

RVec<RVec<unsigned long>> GeneralDirect(RVec<RVec<float>> input_particles) {
    
    unsigned long particle_size = 0;
    unsigned long count = 0;

    for (auto it = input_particles.cbegin(); it != input_particles.cend(); ++it, ++count) {
        if (particle_size == 0) particle_size = it->size();
        assert(it->size() == particle_size);
    }

    // Empty input vectors, preserve shape to avoid crashes
    if (particle_size == 0) {
        return MakeEmptyShape(input_particles.size());
    }

    RVec<RVec<unsigned long>> out;
    out.reserve(count);

    for (unsigned long i = 0; i < count; i++) {
        RVec<unsigned long> increasing_index = ROOT::VecOps::Enumerate(input_particles[0]);
        out.push_back(increasing_index);
    }
    return out;
}

// turns a table into a useable correction function
/**
 * @brief Creates a correction list corresponding to a table
 * 
 * @param num_vars Number of variables being used to index
 * @param lower_bounds Lower bounds of each box, arranged entry x num_vars
 * @param upper_bounds Upper bounds of each box, arranged entry x num_vars
 * @param values Scaling corresponding to each box, arranged entry x dimension of value (1 for a scale factor, 3 for nominal/up/down, etc)
 * @param arguments Arguments to check against the table, with length num_vars
 * @return RVec<float> resulting correction terms, length [dimension of value]
 */
RVec<float> use_table(int num_vars, RVec<RVec<float>> lower_bounds, RVec<RVec<float>> upper_bounds, RVec<RVec<float>> values, RVec<float> arguments) {


    auto test_bound_lamb = [arguments, num_vars](RVec<float> lower_m, RVec<float> upper_m) {
        assert(arguments.size() == num_vars && lower_m.size() == num_vars && upper_m.size() == num_vars);
        int tot = Sum(arguments > lower_m) + Sum(arguments < upper_m);
        if (tot == (2*num_vars)) return true;
        return false;
    };

    auto one_hot_mapped = Map(lower_bounds, upper_bounds, test_bound_lamb);
    auto indices = Nonzero(one_hot_mapped);
    if (indices.size() != 1) assert(false);

    int first_index = indices[0];

    return values[first_index];
}

/**
 * @brief Creates an object representing a correction function, ready to apply on a list of arguments
 * 
 * @param num_vars Number of variables being used to index
 * @param lower_bounds Lower bounds of each box, arranged entry x num_vars
 * @param upper_bounds Upper bounds of each box, arranged entry x num_vars
 * @param values Scaling corresponding to each box, arranged entry x dimension of value (1 for a scale factor, 3 for nominal/up/down, etc)
 * @return Function object RVec<float> (with length num_vars) -> RVec<float> (with length [dimension of value]) taking in the values for each variable in the table, and returning the correction values 
 */
auto create_table_function(int num_vars, RVec<RVec<float>> lower_bounds, RVec<RVec<float>> upper_bounds, RVec<RVec<float>> values) {
    return std::bind(use_table, num_vars, lower_bounds, upper_bounds, values, std::placeholders::_1);
}

    

class ConstCorr {
public:
    ConstCorr(double nom, double up, double down)
        : nom_(nom), up_(up), down_(down) {}

    std::vector<double> eval() {
        return { nom_, up_, down_ };
    }

private:
    double nom_;
    double up_;
    double down_;
};

class ConstWeight {
public:
    ConstWeight(double nom)
        : nom_(nom) {}

    double eval() {
        return nom_;
    }

private:
    double nom_;

};