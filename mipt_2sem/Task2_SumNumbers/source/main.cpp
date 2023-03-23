#include <iostream>

#include "omp.h"
#include "input.h"

namespace omp
{

long sumNumbers(long N)
{
    if (omp_in_parallel())
        throw std::runtime_error("Already in parallel block");

    long sum = 0;
    #pragma omp parallel
    {
        // Разные варианты параметра schedule (N = 1000000000):
        // static, 1 - 2,120s
        // dynamic, 1 - 25,668s
        // guided, 1 - 0,230s

        #pragma omp for reduction(+:sum) schedule(guided, 1)
            for (long i = 1; i <= N; ++i)
                sum += i;
    }

    return sum;
}

}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: ./sum_numbers <N>" << std::endl;
        return EXIT_FAILURE;
    }

    long N = util::strtol(argv[1]);
    std::cout << "sum = " << omp::sumNumbers(N) << std::endl;

    return EXIT_SUCCESS;
}