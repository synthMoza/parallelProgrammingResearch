#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <omp.h>

#include "input.h"

/*
    Получить кол-во простых чисел от 1 до inputNumber включительно
*/
uint64_t GetPrimeNumbers(uint64_t inputNumber)
{
    /*
        Для проверки простоты числа используем решето Эратосфена. Почему именно его:
        - перебор всех чисел от 1 до N (или sqrt(N)) для проверки простоты N слишком долго, т.к. нужно перебрать 
            все числа, а нам нужно только их кол-во
        - можно использовать, например, какие-то быстрые проверки на простоту (малая теорема Ферма), но ее скорость
            компенсируется наличием неправильных ответов (числа Кармайкла)
        - существует так же другие решета (решето Сударама), но решето Эратосфена легко параллелится и легок в своей
            реализации (и, что важно, в распараллеливании)
    */
    uint64_t countPrimes = 0;
    std::vector<bool> sieve(inputNumber + 1);

    auto end = static_cast<uint64_t>(std::sqrt(inputNumber));
    #pragma omp parallel shared(sieve, countPrimes)
    {
        // Параллельно инициализируем решето начальными данными
        #pragma omp for schedule(static)
        for (uint64_t i = 2; i <= inputNumber; ++i)
            sieve[i] = true;

        // В нераспараллеленом цикле идем по числам, параллелим внутренний цикл "маркировки" чисел
        for (uint64_t i = 2; i <= end; ++i)
        {
            if (sieve[i])
            {
                #pragma omp for schedule(static)
                for (uint64_t j = i * i; j <= inputNumber; j += i)
                    sieve[j] = false;
            }
        }

        // Параллельным проходом считаем кол-во простых чисел в решете, складываем все результаты через reduction
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