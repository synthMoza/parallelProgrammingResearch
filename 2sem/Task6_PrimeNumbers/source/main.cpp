#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <omp.h>

#include "input.h"

uint64_t GetPrimeNumbers(uint64_t inputNumber)
{
    uint64_t countPrimes = 0;
    std::vector<bool> sieve(inputNumber + 1);

    auto end = static_cast<uint64_t>(std::sqrt(inputNumber));
    #pragma omp parallel shared(sieve, countPrimes)
    {
        #pragma omp for schedule(static)
        for (uint64_t i = 2; i <= inputNumber; ++i)
            sieve[i] = true;

        for (uint64_t i = 2; i <= end; ++i)
        {
            if (sieve[i])
            {
                #pragma omp for schedule(static)
                for (uint64_t j = i * i; j <= inputNumber; j += i)
                    sieve[j] = false;
            }
        }

        #pragma omp for schedule(static) reduction(+ : countPrimes)
        for (uint64_t i = 2; i <= inputNumber; ++i)
            if (sieve[i])
                countPrimes++;
    }

    return countPrimes;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Error! No input number, usage: ./prime_numbers <N>" << std::endl;
        return EXIT_FAILURE;
    }

    uint64_t inputNumber = util::strtoull(argv[1]);

    auto countPrimes = GetPrimeNumbers(inputNumber);
    std::cout << "There are " << countPrimes << " prime numbers from 1 to " << inputNumber << std::endl;

    return EXIT_SUCCESS;
}