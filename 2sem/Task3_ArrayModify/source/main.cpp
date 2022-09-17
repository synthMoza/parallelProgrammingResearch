#include "omp.h"

#include <algorithm>
#include <array>
#include <iostream>

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

int main()
{
    constexpr size_t arraySize = 1e5;

    auto a = GetInitializedArray<arraySize>();
    auto maxThreads = omp_get_max_threads();

    uint64_t cache = 0;
    #pragma omp parallel shared(a, cache)
    {
        auto threadId = omp_get_thread_num();
        uint64_t temp = 0;

        // Каждый поток итерируется по массиву с шагом maxThreads
        for (size_t i = 0; i < arraySize; i += maxThreads)
        {
            // Распределяем текущий промежуток итераций между всеми потоками
            auto end = std::min(i + maxThreads, arraySize);
            
            #pragma omp for schedule(static, 1)
            for (size_t j = i; j < end; ++j)
            {
                // Считаем новое значение, пока храним во временной переменной
                if (j == 0)
                {
                    temp = a.at(j) * a.at(j + 1) / 3;
                }
                else if (j == arraySize - 1)
                {
                    temp = a.at(j) * a.at(j - 1) / 3;
                }
                else
                {
                    if (threadId == 0)
                        temp = a.at(j) * cache * a.at(j + 1) / 3;
                    else
                        temp = a.at(j) * a.at(j - 1) * a.at(j + 1) / 3;
                }
            } // sync
            
            // Сохраняем последнее значение для следующей итерации
            if (threadId == maxThreads - 1 && end != arraySize)
                cache = a.at(i + threadId);

            // Можем безопасно записывать новое значение
            if (i + threadId < arraySize)
                a.at(i + threadId) = temp;

            // Синхронизируемся, чтобы кэш успел быть записан
            #pragma omp barrier
        }
    }

    std::cout << "Modified array:" << std::endl;
    PrintArray(a.begin(), a.end());

    return EXIT_SUCCESS;
}
