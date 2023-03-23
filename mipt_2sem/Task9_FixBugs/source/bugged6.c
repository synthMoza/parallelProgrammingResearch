/******************************************************************************
* ЗАДАНИЕ: bugged6.c
* ОПИСАНИЕ:
*   Множественные ошибки компиляции
*   Программа должнна была показывать, как можно выполнять параллельный код,
*   используя функции, но что-то пошло не так.
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define VECLEN 100

float a[VECLEN], b[VECLEN];

float dotprod()
{
    int i, tid;
    /*
        Локально заведем другую переменную, на которую будем делать
        reduction, результат положим в sum, ее и вернем
    */
    float sum = 0.0;

    #pragma omp parallel private(i, tid)
    {
        tid = omp_get_thread_num();
     
        #pragma omp for reduction(+:sum)
        for (i = 0; i < VECLEN; i++)
        {
            sum = sum + (a[i] * b[i]);
            printf("  tid= %d i=%d\n", tid, i);
        }        
    }

    return sum;
}


int main (int argc, char *argv[])
{
    int i;
    //float sum;

    for (i = 0; i < VECLEN; i++)
        a[i] = b[i] = 1.0 * i;
    //sum = 0.0;

    /*
        Мы не можем сделать reduction внутри функции, а parallel блок вне функции, т.к. все
        локальные перменные будут приватными. Поэтому лучше перенести параллельный блок внутрь функции,
        а сумму просто возвращать из функции
    */

    printf("Sum = %f\n", dotprod());
}


