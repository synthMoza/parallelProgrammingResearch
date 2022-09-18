/******************************************************************************
* ЗАДАНИЕ: bugged2.c
* ОПИСАНИЕ:
*   Еще одна программа на OpenMP с багом. 
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int nthreads, i, tid;
    float total;

    /*
        Не указано явно, какая область видимости у переменных (по умолчанию все
        приватные), а nthreads и total должны быть shared
    */
    #pragma omp parallel private(tid, i) shared(nthreads, total)
    {
        tid = omp_get_thread_num();
        if (tid == 0)
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
        printf("Thread %d is starting...\n", tid);

        #pragma omp barrier

        total = 0.0;
        /*
            Для корректного подсчета суммы в объявлении цикла должна стоять директива reduction,
            чтобы сложить все результаты потоков
        */
        #pragma omp for schedule(dynamic, 10) reduction(+:total)
        for (i = 0; i < 1000000; i++) 
            total = total + i*1.0;

        printf ("Thread %d is done! Total= %e\n", tid, total);
    }
}
