#ifndef SAIS_HPP
#define SAIS_HPP

#include <sdsl/int_vector.hpp>

namespace sais{

typedef sdsl::std_size_type_for_int_vector size_type;

template<uint8_t INT_SIZE>
struct suffix_array{
    sdsl::int_vector<INT_SIZE>& ref;

    suffix_array(sdsl::int_vector<INT_SIZE>& SA) : ref(SA){
        reset();
    }
    ~suffix_array(){
        for(size_type i = 0; i < ref.size(); ++i){
            setType(i, false);
        }
    }

    void reset(){
        for(size_type i = 0; i < ref.size(); ++i){  
            set(i, -1);
            setType(i, false);
        }
    }

    size_type size() const {
        return ref.size();
    }

    void set(const size_type& i, const size_type& value){
        bool temp = isL(i);
        ref[i] = value;// & ~(1 << INT_SIZE-1);
        setType(i, temp);
    }

    const size_type operator [](const size_type& i) const {
        return ref[i] & ~((unsigned long) 1 << INT_SIZE-1);
    }

    const bool isL(const size_type& i) const {
        return (ref[i] >> INT_SIZE-1) & 1;
    }

    void setType(const size_type& i, bool isL){
        if(isL){
            ref[i] = ref[i] | ((unsigned long) 1 << INT_SIZE-1);
        } else {
            ref[i] = ref[i] & ~((unsigned long) 1 << INT_SIZE-1);
        }
    }

};


template<uint8_t SIGMA, uint8_t INT_SIZE>
size_type lexLetters(const sdsl::int_vector<SIGMA>& word, suffix_array<INT_SIZE>& SA, sdsl::int_vector<INT_SIZE>& alphabet){
    size_type nof_lms = 0;

    size_type last = word.size() - 1;

    //mark letters as S or L
    SA.setType(last, false);
    ++alphabet[word[last]];
    //i is one greater than the current index
    for (size_type i = last; i > 0; --i) {
        
        if (word[i - 1] > word[i]) {
            SA.setType(i-1, 1);
            if(!SA.isL(i)){
                ++nof_lms;
            }
        }
        else if (word[i - 1] < word[i]) {
            SA.setType(i-1, 0);
        }
        else {
            SA.setType(i-1, SA.isL(i));
        }
        ++alphabet[word[i-1]];
    }

    return nof_lms;
}


template<uint8_t INT_SIZE>
void calculate_bucket_indexes(sdsl::int_vector<INT_SIZE>& L_buckets, sdsl::int_vector<INT_SIZE>& S_buckets){
    size_type current = 0;
    for(size_type i = 0; i < L_buckets.size(); ++i){
        if(L_buckets[i] > 0){
            S_buckets[i] = current + L_buckets[i] - 1;
            L_buckets[i] = current;
            current = S_buckets[i]+1;
        }
    }
}


template<uint8_t SIGMA, uint8_t INT_SIZE>
void induce_lms(const sdsl::int_vector<SIGMA>& word, suffix_array<INT_SIZE>& SA, 
        sdsl::int_vector<INT_SIZE> S_buckets)
{
    for(size_type i = 1; i < word.size(); ++i){
        if(!SA.isL(i) && SA.isL(i-1)){
            SA.set(S_buckets[word[i]]--, i);
        }
    }
}


template<uint8_t SIGMA, uint8_t INT_SIZE>
void induce_ls(const sdsl::int_vector<SIGMA>& word, suffix_array<INT_SIZE>& SA, 
        sdsl::int_vector<INT_SIZE> L_buckets, sdsl::int_vector<INT_SIZE> S_buckets)
{

    for(size_type i = 0; i < word.size(); ++i){
        size_type index = SA[i] - 1;
        if(index < word.size() && SA.isL(index)){
            SA.set(L_buckets[word[index]]++, index);
        }
    }

    for(size_type i = word.size(); i > 0; --i){
        size_type index = SA[i-1] - 1;
        if(index < word.size() && !SA.isL(index)){
            SA.set(S_buckets[word[index]]--, index);
        }
    }
}

template<uint8_t SIGMA, uint8_t INT_SIZE>
bool lms_positions_unequal(const sdsl::int_vector<SIGMA>& word, const suffix_array<INT_SIZE>& SA, size_type p1, size_type p2){
    bool is_lms_p1 = false, is_lms_p2 = false;
    for(;;){
        if(word[p1] != word[p2]) return true;
        if(SA.isL(p1) != SA.isL(p2)) return true;
        if(is_lms_p1 && is_lms_p2) return false;
        ++p1; ++p2;

        is_lms_p1 = !SA.isL(p1) && SA.isL(p1-1);
        is_lms_p2 = !SA.isL(p2) && SA.isL(p2-1);
        if(is_lms_p1 ^ is_lms_p2) return true;
    }
    return false;
}

template<uint8_t SIGMA, uint8_t INT_SIZE, bool special_sep = false>
size_type reduce_text(const sdsl::int_vector<SIGMA>& word, const suffix_array<INT_SIZE>& SA, sdsl::int_vector<INT_SIZE>& names){
    size_type red_sigma = 1;
    size_type last_lms = word.size();

    for(size_type i = 0; i < word.size(); ++i){

        size_type index = SA[i];

        if(index == 0 || index >= SA.size() || SA.isL(index) || !SA.isL(index-1)){
            continue;
        }

        if(last_lms < word.size() && ((special_sep && word[index] == 1) || lms_positions_unequal(word, SA, last_lms, index))){
            ++red_sigma;
        }

        names[index] = red_sigma-1;
        last_lms = index;
    }

    return red_sigma;
}

template<uint8_t SIGMA, uint8_t INT_SIZE>
size_type SAIS(const sdsl::int_vector<SIGMA>& word, sdsl::int_vector<INT_SIZE>& SA_in, int depth = 0){
    assert(word.size() <= SA_in.size());
    assert(word[word.size()-1] == 0);
    assert(word.size() < ((unsigned long) 1 << INT_SIZE-1)-1);

    suffix_array<INT_SIZE> SA(SA_in);

    size_type max_char = 0;
    for(size_type i = 0; i < word.size(); ++i){
        if(word[i] > max_char){
            max_char = word[i];
        }
    }

    //find labels in bit vector
    sdsl::int_vector<INT_SIZE> L_buckets(max_char+1, 0);
    sdsl::int_vector<INT_SIZE> S_buckets(max_char+1, 0);

    //counts letters in parallel
    size_type nof_lms = lexLetters(word, SA, L_buckets);

    calculate_bucket_indexes(L_buckets, S_buckets);

    //recursive sort LMS
    induce_lms(word, SA, S_buckets);
    induce_ls(word, SA, L_buckets, S_buckets);

    sdsl::int_vector<INT_SIZE> names(word.size(), -1);

    size_type red_sigma = reduce_text(word, SA, names);

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

        depth = SAIS(SA.ref, red_SA, depth+1);
            
        SA.ref.resize(word.size());

        //clear SA
        SA.reset();
        sdsl::int_vector<INT_SIZE> temp_buckets = S_buckets;

        lexLetters(word, SA, temp_buckets);

        temp_buckets = S_buckets;

        for(size_type i = red_SA.size(); i > 0; --i){
            SA.set(temp_buckets[word[lms_positions[red_SA[i-1]]]]--, lms_positions[red_SA[i-1]]);
        }
    }

    //finish sort
    induce_ls(word, SA, L_buckets, S_buckets);

    return depth;

}
}

#endif