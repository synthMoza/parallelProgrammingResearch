#include "input.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <omp.h>

int main(int argc, char* argv[])
{
    /*
        Считаем ряд: sin(kx)/k; сумма бесконечного ряда: (pi - x) / 2
    */

    constexpr long double sumArg = M_PI / 7;

    if (argc != 2)
    {
        std::cout << "Error! No input number, usage: ./infinite_series <number>" << std::endl;
        return EXIT_FAILURE;
    }

    auto inputNumber = util::strtol(argv[1]);
    long double sum = 0;
    
    // Используем замок для сложения результата со всех потоков
    omp_lock_t lock;
    omp_init_lock(&lock);

    #pragma omp parallel shared(sum)
    {
        long double tempSum = 0;

        #pragma omp for schedule(static, 1)
        for (size_t i = 1; i < static_cast<size_t>(inputNumber); ++i)
            tempSum += std::sin(i * sumArg) / i;
        
        omp_set_lock(&lock);
        sum += tempSum;
        omp_unset_lock(&lock);
    }

    constexpr auto precision = 100;
    std::cout << "Result: " << std::setprecision(precision) << sum << std::endl;
    std::cout << "Infinite sum equals to " << (M_PI - sumArg) / 2 << std::endl;

    omp_destroy_lock(&lock);
    return EXIT_FAILURE;
}
