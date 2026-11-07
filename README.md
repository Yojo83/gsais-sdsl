gSAIS for sdsl
=========

Motivation
-----------

We wanted to implement the gSAIS Algorithm in the context of sdsl. gSAIS (generalized Suffix array induced sorting) calculates the generalized suffix array with induced sorting, as described by [Louza et al.](https://doi.org/10.1016/j.tcs.2017.03.039). [sdsl](https://github.com/simongog/sdsl-lite) (Succinct Data Structure Library) is a C++11 library containing implementations of succinct data structures. We want to implement gSAIS based on the int_vector\<w\>.

This project is part of a bachelor thesis to understand gSAIS.

Content
--------
- source code for our implementation of gSAIS (and SAIS)
- source code for a benchmark to test our implementation
- test data for gSAIS
- results of the benchmark used on the test data

Requirements
------------

gSAIS and SAIS only require [sdsl](https://github.com/simongog/sdsl-lite) as library. Its instalation and requirements can be found in its README.

The benchmark additionally uses [libsais](https://github.com/IlyaGrebnov/libsais) to validate the correctness. It is a highly optimized C library to calculate the (generalized) suffix array. We link a compiled object of it to our program. 

To measure the memory usage of the algorithm the benchmark can use [malloc_count](https://github.com/bingmann/malloc_count). It is also compiled and linked to our program, as described in its README.

Getting Started
------------

To get you started, you can start by compiling the following sample program which constructs a suffix array over the text `immiissiissippi`:

```cpp
#include <iostream>
#include <string>

#include <sdsl/int_vector.hpp>

#include "SAIS.hpp"

static const std::string const_word "immiissiissippi";

int main() {
    sdsl::int_vector<8> word (const_word.length()+1);
    for(int i = 0; i < const_word.length(); ++i){
        word[i] = const_word[i];
    }
    word[const_word.length()] = 0;

    sdsl::int_vector<32> SA (word.size());
    
    sais::sais(word, SA);

    for(int i = 0; i < SA.length(); ++i){
        std::cout << SA[i] << " ";
    }

    std::cout << std::endl;
}
```

To compile the program using `g++` run:

```sh
g++ -std=c++11 -O3 -I ~/include -L ~/lib main.cpp -o program -lsdsl -ldivsufsort -ldivsufsort64
```

Here we use `0` as ending symbol of the string.

For gSAIS the procedure is pretty similar, as we fill one `int_vector<w>` with all strings. After every string there has to be a `1` (and no where else). The String still has to terminate with an extra `0`. An example input would be `banana1nana1banan10`:

```cpp
#include <iostream>
#include <string>

#include <sdsl/int_vector.hpp>

#include "gSAIS.hpp"

int nof_strings = 3;
static const std::string words[nof_strings] = {"banana", "nana", "banan"};
constexpr int N = words[0].length() + words[1].length() + words[2].length();

int main() {
    sdsl::int_vector<8> word (N + nof_strings + 1);
    for(int i = 0, index = 0; i < nof_strings; ++i){
        for(int j = 0; j < words[i].length(); ++j){
            word[index++] = words[i][j];
        }
        word[index++] = 1;
    }
    word[index] = 0;

    sdsl::int_vector<32> SA (word.size());
    
    gsais::gsais(word, SA);

    for(int i = 0; i < SA.length(); ++i){
        std::cout << SA[i] << " ";
    }

    std::cout << std::endl;
}
```

Benchmarks
----------

Our benchmark provides a simple test for our implementation to check for correctness, runtime and optionally space consumtion.
We uploaded our results here too.

