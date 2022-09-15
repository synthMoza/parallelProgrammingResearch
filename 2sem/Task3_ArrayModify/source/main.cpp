#include "omp.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

// Сгенерировать массив с элементами, равными индексам
template <size_t size>
inline std::array<uint64_t, size> GetInitializedArray()
{
    std::array<uint64_t, size> array;
    
    uint32_t counter = 0;
    std::for_each(array.begin(), array.end(), [&](uint64_t& elem){
        elem = (counter++);
    });

    return array;
}

// Вывести массив в stdout поток
template <typename It>
inline void PrintArray(It begin, It end)
{
    using value_type = typename std::iterator_traits<It>::value_type;
    std::for_each(begin, end, [](value_type& elem){
        std::cout << elem << " ";
    });

    std::cout << std::endl;
}

// Получить значение для элемента массива под данным номером (формула напрямую из задания)
template <size_t size>
inline uint64_t GetValue(std::array<uint64_t, size>& array, size_t index)
{
    if (index == 0)
        return array.at(index) * array.at(index + 1) / 3;
    else if (index == array.size() - 1)
        return array.at(index) * array.at(index - 1) / 3;
    else
        return array.at(index) * array.at(index + 1) * array.at(index - 1) / 3;
}

int main()
{
    constexpr size_t arraySize = 1e5;

    auto a = GetInitializedArray<arraySize>();
    std::array<uint64_t, arraySize> b;

    #pragma omp parallel shared(a, b)
    {
        #pragma omp for schedule(guided)
        for (size_t i = 0; i < arraySize; ++i)
            b[i] = GetValue(a, i);
    }

    std::cout << "Modified array:" << std::endl;
    PrintArray(b.begin(), b.end());

    return EXIT_SUCCESS;
}