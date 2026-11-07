#ifndef G_SAIS_HPP
#define G_SAIS_HPP

#include <string>

#include "SAIS.hpp"

namespace gsais{

typedef sdsl::std_size_type_for_int_vector size_type;


template<uint8_t SIGMA, uint8_t INT_SIZE>
void induce_lms(const sdsl::int_vector<SIGMA>& word, sais::suffix_array<INT_SIZE>& SA, 
        sdsl::int_vector<INT_SIZE> S_buckets)
{
    SA.set(0, word.size()-1);
    SA.set(S_buckets[1]--, word.size()-2);
    
    size_type last = word.size() -1;
    
    for(size_type i = word.size()-3; i > 0; --i){
        if(SA.isL(i) || !SA.isL(i-1)){
            continue;
        }
        if(word[i] == 1){
            SA.set(last, -1);
            SA.set(S_buckets[1]--, i);

            continue;
        }

        last = (S_buckets[word[i]])--;
        SA.set(last, i);
    }
}
template<uint8_t SIGMA, uint8_t INT_SIZE>
void induce_ls(const sdsl::int_vector<SIGMA>& word, sais::suffix_array<INT_SIZE>& SA, 
        sdsl::int_vector<INT_SIZE> L_buckets, sdsl::int_vector<INT_SIZE> S_buckets)
{
    for(size_type i = 0; i < word.size(); ++i){
        size_type index = SA[i] - 1;

        if(index < word.size() && SA.isL(index) && word[index] != 1){
            SA.set(L_buckets[word[index]]++, index);
        }
    }

    for(size_type i = word.size(); i > 0; --i){
        size_type index = SA[i-1] - 1;

        if(index < word.size() && !SA.isL(index) && word[index] != 1){
            SA.set(S_buckets[word[index]]--, index);
        }
    }
}


//requires 0 at end and 1 as seperators
//2^(INT_SIZE - 1) > word.size() must hold
template<uint8_t SIGMA, uint8_t INT_SIZE>
size_type gsais(const sdsl::int_vector<SIGMA>& word, sdsl::int_vector<INT_SIZE>& SA_in){
    assert(word.size() <= SA_in.size());
    assert(word[word.size()-1] == 0);
    assert(word[word.size()-2] == 1);
    assert(word.size() < ((unsigned long) 1 << INT_SIZE-1)-1);

    sais::suffix_array<INT_SIZE> SA(SA_in);

    size_type max_char = 0;
    for(size_type i = 0; i < word.size(); ++i){
        if(word[i] > max_char){
            max_char = word[i];
        }
    }


    sdsl::int_vector<INT_SIZE> L_buckets(max_char+1, 0);
    sdsl::int_vector<INT_SIZE> S_buckets(max_char+1, 0);

    //counts characters in parallel
    size_type nof_lms = sais::lexLetters(word, SA, L_buckets);

    sais::calculate_bucket_indexes(L_buckets, S_buckets);

    //recursive sort LMS
    gsais::induce_lms(word, SA, S_buckets);
    gsais::induce_ls(word, SA, L_buckets, S_buckets);

    sdsl::int_vector<INT_SIZE> names(word.size(), -1);

    size_type red_sigma = sais::reduce_text<SIGMA, INT_SIZE, true>(word, SA, names);

    size_type depth = 0;

    if(red_sigma != nof_lms){
        //do recursion

        //clear SA
        SA.ref.resize(nof_lms);
        SA.reset();

        sdsl::int_vector<INT_SIZE> lms_positions(nof_lms);

        size_type j = 0;
        for(size_type i = 0; i < names.size(); ++i){
            if(names[i] >= red_sigma){
                continue;
            }

            lms_positions[j] = i;
            SA.set(j++, names[i]);
        }

        names.resize(0);
        sdsl::int_vector<INT_SIZE> red_SA(nof_lms);

        depth = sais::SAIS(SA.ref, red_SA, 1);
        
        SA.ref.resize(word.size());

        //clear SA
        SA.reset();
        sdsl::int_vector<INT_SIZE> temp_buckets = S_buckets;
        sais::lexLetters(word, SA, temp_buckets);

        //re-add seperator symbols
        SA.set(0, word.size()-1);
        SA.set(S_buckets[1]--, word.size()-2);
        
        for(size_type i = word.size()-1; i > 0; --i){
            if(SA.isL(i) || !SA.isL(i-1) || word[i] != 1){
                continue;
            }
            SA.set(S_buckets[1]--, i);
        }

        //can be removed?
        temp_buckets = S_buckets;

        for(size_type i = red_SA.size(); i > 0; --i){
            if(word[lms_positions[red_SA[i-1]]] != 1){
                SA.set(temp_buckets[word[lms_positions[red_SA[i-1]]]]--, lms_positions[red_SA[i-1]]);
            }
        }
    }

    
    //finish sort
    gsais::induce_ls(word, SA, L_buckets, S_buckets);
    
    return depth;

}

} //namespace gsais

#endif