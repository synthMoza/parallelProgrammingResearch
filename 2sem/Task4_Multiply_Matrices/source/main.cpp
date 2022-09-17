#include "get_matrix.h"
#include "matrix.h"
#include "input.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "No input matrix size! Usage: ./matrix_multiply <matrix_size>" << std::endl;
        return EXIT_FAILURE;
    }

    size_t matrixSize = util::strtol(argv[1]);
    auto firstMatrix = util::GetRandomMatrix(matrixSize, matrixSize);
    auto secondMatrix = util::GetRandomMatrix(matrixSize, matrixSize);

    auto result = firstMatrix * secondMatrix;
    
    std::cout << "firstMatrix" << std::endl << firstMatrix;
    std::cout << "secondMatrix" << std::endl << secondMatrix;
    std::cout << "result" << std::endl << result;

    return EXIT_SUCCESS;
}