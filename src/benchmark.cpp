#include <iostream>
#include <string>
#include <random>
#include <cstdlib>
#include <fstream>

#include <sdsl/int_vector.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/memory_management.hpp>

#include "libsais/libsais.h"
#include "libsais/libsais64.h"

#ifdef MALLOC_COUNT
#include "malloc_count/malloc_count.h"
#include "malloc_count/stack_count.h"
#endif

#include "Timer.hpp"
#include "gSAIS.hpp"
#include "FileManager.hpp"


/*
Compile:
g++ -std=c++11 -O3 -DNDEBUG -I ~/include -L ~/lib libsais/libsais.o libsais/libsais64.o benchmark.cpp  -o benchmark -ldl -lsdsl -ldivsufsort -ldivsufsort64

With malloc_count:
g++ -std=c++11 -O3 -DNDEBUG -I ~/include -L ~/lib libsais/libsais.o libsais/libsais64.o benchmark.cpp malloc_count/stack_count.o malloc_count/malloc_count.o -DMALLOC_COUNT -o benchmark -ldl -lsdsl -ldivsufsort -ldivsufsort64

With asserts:
g++ -std=c++11 -O3 -I ~/include -L ~/lib libsais/libsais.o libsais/libsais64.o benchmark.cpp  -o benchmark -ldl -lsdsl -ldivsufsort -ldivsufsort64

With asserts and malloc_count:
g++ -std=c++11 -O3 -I ~/include -L ~/lib libsais/libsais.o libsais/libsais64.o benchmark.cpp malloc_count/stack_count.o malloc_count/malloc_count.o -DMALLOC_COUNT -o benchmark -ldl -lsdsl -ldivsufsort -ldivsufsort64
*/

/*
Usage:
./benchmark file1 file2 ...
Calculates the generalized suffix array for each given file with sdls and libsais and compares them. Each Line is interpreted as 1 string.
The results are stored into file1.out, file2.out ...
*/

//Configuration
#define WORD_WIDTH 8


template<uint8_t INT_SIZE>
static std::size_t calculate_error(int64_t* first, sdsl::int_vector<INT_SIZE> second) {
    std::size_t err = 0;

    for (std::size_t i = 0; i < second.size()-1; ++i){
        err += std::abs((std::ptrdiff_t) first[i] - (std::ptrdiff_t)second[i+1]);
    }

    return err;
}
template<uint8_t INT_SIZE>
static std::size_t calculate_error(int32_t* first, sdsl::int_vector<INT_SIZE> second) {
    std::size_t err = 0;

    for (std::size_t i = 0; i < second.size()-1; ++i){
        err += std::abs((std::ptrdiff_t) first[i] - (std::ptrdiff_t)second[i+1]);
    }

    return err;
}


void concat(const std::vector<std::string> &words, uint8_t* word_ref){
    sais::size_type index = 0;
    for(sais::size_type i = 0; i < words.size(); ++i){
        for(sais::size_type j = 0; j < words[i].size(); ++j){
            if(words[i][j] == 1 || words[i][j] == 0){
                exit(2);
            }
            word_ref[index++] = words[i][j];
        }
        word_ref[index++] = 0;
    }
}

sais::size_type bits_needed(sais::size_type value)
{
    sais::size_type bits = 0;
    for (sais::size_type bit_test = 16; bit_test > 0; bit_test >>= 1)
    {
        if (value >> bit_test != 0)
        {
            bits += bit_test;
            value >>= bit_test;
        }
    }
    return bits + value;
}

int main(int argc, char* argv[]){
    //std::cout << "Configuration:\n"
    //          << "\n";

    Timer timer_ref;
    Timer timer_tst;

    std::cout << "Benchmark:\n";


    sais::size_type depth = 0;
    int64_t err_code = 0;


    for (int n = 1; n < argc; ++n){
        std::vector<std::string> lines;
        sais::util::readFileByLine(argv[n], lines);


        sdsl::int_vector<WORD_WIDTH> word_tst;
        sais::util::concat(lines, word_tst);

        const sais::size_type bits = bits_needed(word_tst.size());

        uint8_t* word_ref = new uint8_t[word_tst.size()-1];
        concat(lines, word_ref);
        
        void* SA_ref;
        if(bits < 32){
            SA_ref = new int32_t[word_tst.size()-1];
        } else {
            SA_ref = new int64_t[word_tst.size()-1];
        }


        sdsl::int_vector<8> SA_tst_8(0);
        sdsl::int_vector<16> SA_tst_16(0);
        sdsl::int_vector<24> SA_tst_24(0);
        sdsl::int_vector<32> SA_tst_32(0);
        sdsl::int_vector<48> SA_tst_48(0);
        sdsl::int_vector<64> SA_tst_64(0);

        if(bits+1 < 8){
            SA_tst_8.resize(word_tst.size());
        } else if(bits+1 < 16){
            SA_tst_16.resize(word_tst.size());
        } else if(bits+1 < 24){
            SA_tst_24.resize(word_tst.size());
        } else if(bits+1 < 32){
            SA_tst_32.resize(word_tst.size());
        } else if(bits+1 < 48){
            SA_tst_48.resize(word_tst.size());
        } else{
            SA_tst_64.resize(word_tst.size());
        }

        // call test implementation

        #ifdef MALLOC_COUNT
        
        std::size_t malloc_base_tst = malloc_count_current();
        void* stack_base = stack_count_clear();
        malloc_count_reset_peak();
        
        #endif

        timer_tst.tick();
        if(bits+1 < 8){
            depth = gsais::gsais(word_tst, SA_tst_8);
        } else if(bits+1 < 16){
            depth = gsais::gsais(word_tst, SA_tst_16);
        } else if(bits+1 < 24){
            depth = gsais::gsais(word_tst, SA_tst_24);
        } else if(bits+1 < 32){
            depth = gsais::gsais(word_tst, SA_tst_32);
        } else if(bits+1 < 48){
            depth = gsais::gsais(word_tst, SA_tst_48);
        } else{
            depth = gsais::gsais(word_tst, SA_tst_64);
        }
        timer_tst.tock();

        #ifdef MALLOC_COUNT

        std::size_t stack_tst = stack_count_usage(stack_base);
        std::size_t malloc_peak_tst = malloc_count_peak();

        // call reference implementation
        std::size_t malloc_base_ref = malloc_count_current();
        stack_base = stack_count_clear();
        malloc_count_reset_peak();

        #endif

        timer_ref.tick();
        if(bits < 32){
            err_code = libsais_gsa(word_ref, (int32_t*) SA_ref, word_tst.size()-1, 0, 0);
        } else {
            err_code = libsais64_gsa(word_ref, (int64_t*) SA_ref, word_tst.size()-1, 0, 0);
        }
        timer_ref.tock();

        #ifdef MALLOC_COUNT
        std::size_t stack_ref = stack_count_usage(stack_base);
        std::size_t malloc_peak_ref = malloc_count_peak();
        #endif

        //calculate error and print results
        std::size_t err;
        if(bits < 32){
            if(bits+1 < 8){
                err = calculate_error((int32_t*) SA_ref, SA_tst_8);
            } else if(bits+1 < 16){
                err = calculate_error((int32_t*) SA_ref, SA_tst_16);
            } else if(bits+1 < 24){
                err = calculate_error((int32_t*) SA_ref, SA_tst_24);
            } else if(bits+1 < 32){
                err = calculate_error((int32_t*) SA_ref, SA_tst_32);
            } else if(bits+1 < 48){
                err = calculate_error((int32_t*) SA_ref, SA_tst_48);
            } else{
                err = calculate_error((int32_t*) SA_ref, SA_tst_64);
            }
        } else {
            if(bits+1 < 8){
                err = calculate_error((int64_t*) SA_ref, SA_tst_8);
            } else if(bits+1 < 16){
                err = calculate_error((int64_t*) SA_ref, SA_tst_16);
            } else if(bits+1 < 24){
                err = calculate_error((int64_t*) SA_ref, SA_tst_24);
            } else if(bits+1 < 32){
                err = calculate_error((int64_t*) SA_ref, SA_tst_32);
            } else if(bits+1 < 48){
                err = calculate_error((int64_t*) SA_ref, SA_tst_48);
            } else{
                err = calculate_error((int64_t*) SA_ref, SA_tst_64);
            }
        }

        std::ofstream outFile;
        outFile.open (std::string(argv[n]) + std::string(".out"));

        std::cout << "Test Number " << n << " completed" << std::endl;
        outFile << "test Size: " << word_tst.size() << std::endl;
        outFile << "bits: " << bits << std::endl;
        outFile << "error: " << err << std::endl << std::endl;
        outFile << "Reference: " << std::endl;
        outFile << "time: " << timer_ref.duration() << std::endl;
        #ifdef MALLOC_COUNT
        outFile << "malloc base: " << malloc_base_ref << std::endl;
        outFile << "malloc peak: " << malloc_peak_ref << std::endl;
        outFile << "malloc diff: " << malloc_peak_ref - malloc_base_ref << std::endl;
        outFile << "stack usage: " << stack_ref << std::endl;
        outFile << "memory ratio: " << (malloc_peak_ref - malloc_base_ref + stack_ref)/ (double) (word_tst.size() * (WORD_WIDTH/8 + (bits < 32 ? 4 : 8))) << std::endl;
        #endif
        outFile << "return code: " << err_code << std::endl << std::endl;
        outFile << "Test: " << std::endl;
        outFile << "time: " << timer_tst.duration() << std::endl;
        #ifdef MALLOC_COUNT
        outFile << "malloc base: " << malloc_base_tst << std::endl;
        outFile << "malloc peak: " << malloc_peak_tst << std::endl;
        outFile << "malloc diff: " << malloc_peak_tst - malloc_base_tst << std::endl;
        outFile << "stack usage: " << stack_tst << std::endl;
        outFile << "memory ratio: " << (malloc_peak_tst - malloc_base_tst + stack_tst)/ (double) (word_tst.size() * (WORD_WIDTH/8 + (bits < 32 ? 4 : 8))) << std::endl;
        #endif
        outFile << "return code: " << depth << std::endl;
        std::cout << "====================================================================================" << std::endl;

        if(bits < 32){
            delete[] (int32_t*) SA_ref;
        } else {
            delete[] (int64_t*) SA_ref;
        }
        delete[] word_ref;
    }
}