#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

#include "omp.h"

#include "read_array.h"
#include "quick_sort.h"

int main(int argc, char* argv[])
{
    using value_type = util::ArrayType::value_type;

    if (argc != 3)
    {
        std::cerr << "Error! Wrong input arguments. Usage: ./quick_sort [-f <fileName>/-s <size>]" << std::endl;
        return EXIT_FAILURE;
    }

    auto inputArray = util::GetInputArray(argv);
    #pragma omp parallel shared(inputArray)
    {
        #pragma omp single
        {
            util::QuickSort(inputArray, 0, inputArray.size() - 1, [](value_type& a, value_type& b){
                return a < b;
            });
        }
    }

    std::for_each(inputArray.begin(), inputArray.end(), [](value_type& elem){
        std::cout << elem << " ";
    });
    std::cout << std::endl;

    return EXIT_FAILURE;
}
