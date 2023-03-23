#include "omp.h"

#include <iostream>

int main()
{
    int threadId = 0;

    #pragma omp parallel private(threadId) 
    {
        threadId = omp_get_thread_num();

        // std::cout - thread-safe, но только в рамках вывода одного объекта
        #pragma omp critical
            std::cout << "ThreadId: " << threadId << std::endl;
    }

    return 0;
}