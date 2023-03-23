#pragma once

#include <ctime>
#include "matrix.h"

namespace util
{

Matrix<int> GetRandomMatrix(const size_t matrixRows, const size_t matrixColumns)
{
    constexpr int MAX_MATRIX_VALUE = 1000;
    util::Matrix<int64_t> matrix(matrixRows, matrixColumns);
    
    std::srand(std::time(nullptr) + std::rand());
    for (size_t i = 0; i < matrixRows; ++i)
        for (size_t j = 0; j < matrixColumns; ++j)
            matrix[i][j] = (std::rand() % (2 * MAX_MATRIX_VALUE)) - MAX_MATRIX_VALUE; 

    return matrix;
}

}