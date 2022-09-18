/******************************************************************************
* ЗАДАНИЕ: bugged1.c
* ОПИСАНИЕ:
*   Данная программа демонстрирует использование конструкции 'parallel for'.
*   Однако, данный код вызывает ошибки компиляции.
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N           50
#define CHUNKSIZE   5

int main(int argc, char **argv)
{
    int i, chunk, tid;
    float a[N], b[N], c[N];

    for (i = 0; i < N; i++)
        a[i] = b[i] = i * 1.0;
    chunk = CHUNKSIZE;

    /*
        Ошибка была в том, что после директивы #pragma omp parallel for не стоит цикл, а стоит получение номера текущего потока.
        Исправление в данном случае заключается в разделении на #pragma omp parallel, в котором получаем номер потока и запускаем
        цикл с #pragma omp for
    */
    #pragma omp parallel shared(a, b, c, chunk) private(i, tid)
    {
        tid = omp_get_thread_num();

        #pragma omp for schedule(static, chunk)
        for (i = 0; i < N; i++)
        {
            c[i] = a[i] + b[i];
            printf("tid = %d i = %d c[i] = %f\n", tid, i, c[i]);
        }        
    }
}
