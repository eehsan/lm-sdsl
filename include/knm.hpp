#pragma once

#include <sdsl/int_vector.hpp>
#include <sdsl/int_vector_mapper.hpp>
#include "sdsl/suffix_arrays.hpp"
#include "sdsl/suffix_trees.hpp"
#include <sdsl/suffix_array_algorithm.hpp>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <string>
#include <iomanip>

#include "utils.hpp"
#include "collection.hpp"
#include "index_succinct.hpp"
#include "constants.hpp"
#include "kn.hpp"
#include "query.hpp"

template <class t_idx, class t_pattern>
double sentence_logprob_kneser_ney(const t_idx& idx, const t_pattern& word_vec, uint64_t& /*M*/, uint64_t ngramsize, bool fast_index, bool ismkn)
{
    // use new state-based container for evaluating sentence probability -- see query.hpp
    if (fast_index && ismkn) {
        double final_score = 0;
        LMQueryMKN<t_idx, typename t_pattern::value_type> query(&idx, ngramsize);
        for (const auto& word : word_vec)
            final_score += log10(query.append_symbol(word));
        return final_score;
    }

    //LOG(INFO) << "sentence_logprob_kneser_ney for: " << idx.m_vocab.id2token(word_vec.begin(), word_vec.end());
    //LOG(INFO) << "\tfast: " << fast_index << " mkn: " << ismkn;
    double final_score = 0;
    std::deque<uint64_t> pattern_deq;
    for (const auto& word : word_vec) {
        pattern_deq.push_back(word);
        if (word == PAT_START_SYM)
            continue;
        if (pattern_deq.size() > ngramsize) {
            pattern_deq.pop_front();
        }
        std::vector<uint64_t> pattern(pattern_deq.begin(), pattern_deq.end());
        /*
        if (pattern.back() == UNKNOWN_SYM) {
            M = M - 1; // excluding OOV from perplexity - identical to SRILM ppl
        }
*/
        double score;
        if (fast_index) {
            score = prob_kneser_ney_single(idx, pattern.begin(), pattern.end(), ngramsize);
        } else {
            score = prob_kneser_ney_dual(idx, pattern.begin(), pattern.end(), ngramsize);
        }
        final_score += log10(score);
    }
    //LOG(INFO) << "sentence_logprob_kneser_ney returning: " << final_score;
    return final_score;
}

template <class t_idx, class t_pattern>
double sentence_perplexity_kneser_ney(const t_idx& idx, t_pattern& pattern, uint32_t ngramsize, bool fast_index, bool ismkn)
{
    auto pattern_size = pattern.size();
    pattern.push_back(PAT_END_SYM);
    pattern.insert(pattern.begin(), PAT_START_SYM);
    // run the query
    uint64_t M = pattern_size + 1;
    double sentenceprob = sentence_logprob_kneser_ney(idx, pattern, M, ngramsize, fast_index, ismkn);
    double perplexity = pow(10, -(1 / (double)M) * sentenceprob);
    return perplexity;
}
