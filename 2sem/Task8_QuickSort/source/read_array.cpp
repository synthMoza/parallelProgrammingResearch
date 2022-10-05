#include <cstring>
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "read_array.h"
#include "input.h"

using namespace util;

ArrayType util::GetInputArray(char* argv[])
{
    constexpr size_t maxSize = 1e5;
    
    ArrayType result;
    result.reserve(maxSize);

    if (std::strcmp(argv[1], "-f") == 0)
    {
        // Читаем элементы из файла
        std::ifstream inputFile(argv[2]);

        ArrayType::value_type inputValue;
        while (inputFile >> inputValue)
            result.push_back(inputValue);
    }
    else if (std::strcmp(argv[1], "-s") == 0)
    {
        size_t inputSize = util::strtoull(argv[2]);
        
        result.resize(inputSize);
        for (size_t i = 0; i < inputSize; ++i)
            std::cin >> result[i]; 
    }
    else
    {
        throw std::runtime_error("Unknown input flags");
    }

    return result;
}