#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <fstream>
#include <string>

#include <sdsl/int_vector.hpp>

namespace sais::util{

void readFileByLine(const char* path, std::vector<std::string> &lines){
    std::ifstream file(path);
    std::string str;
    while (std::getline(file, str))
    {
        lines.push_back(str);
    }
}


template<uint8_t SIGMA>
void concat(const std::vector<std::string> &words, sdsl::int_vector<SIGMA>& word){
    sais::size_type N = words.size() + 1;
    for(sais::size_type i = 0; i < words.size(); ++i){
        N += words[i].length();
    }

    word.resize(N);

    sais::size_type index = 0;
    for(sais::size_type i = 0; i < words.size(); ++i){
        for(sais::size_type j = 0; j < words[i].size(); ++j){
            if(words[i][j] == 1 || words[i][j] == 0){
                exit(2);
            }
            word[index++] = words[i][j];
        }
        word[index++] = 1;
    }
    
    word[index] = 0;
}


template<uint8_t SIGMA>
void createConcatWordFromFile(const char* path, sdsl::int_vector<SIGMA>& word){
    std::vector<std::string> lines;
    readFileByLine(path, lines);
    concat(lines, word);
}


}


#endif