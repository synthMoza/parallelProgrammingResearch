#include "omp.h"

#include <iostream>

int main()
{
    #pragma omp parallel
    {
        auto threadId = omp_get_thread_num();

        // Запускаем цикл по всем номерам потоков, внутри цикла барьер, после проверяем, 
        // является ли текущая итерация нужной
        for (int i = omp_get_max_threads() - 1; i >= 0; --i)
        {
            #pragma omp barrier
            if (i == threadId)
            {
                #pragma omp critical
                    std::cout << "ThreadId: " << i << std::endl;
            }
        }
        
    }

    return 0;
}