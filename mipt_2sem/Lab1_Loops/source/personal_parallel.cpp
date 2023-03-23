#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define ISIZE 20000
#define JSIZE 20000

#define CHECK_FAILED(res)               \
if (res < 0)                            \
    return res;                         \

int main(int argc, char* argv[])
{
    CHECK_FAILED(MPI_Init(&argc, &argv));

    // старт таймера
    MPI_Barrier(MPI_COMM_WORLD);
	double startTime = MPI_Wtime();

    // получаем свой номер и кол-во потоков
    int rank, size;
    CHECK_FAILED(MPI_Comm_size(MPI_COMM_WORLD, &size));
	CHECK_FAILED(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    // каждый поток получает промежуток, в котором должен считать
    int totalSize = ISIZE - 5;

    int m = totalSize % size;
    int minSize = totalSize / size;
    
    int* recvCount = nullptr;
    int* displ = nullptr;
    int currentBegin = 0, currentEnd = 0;
    if (size > 1 && rank == 0)
    {
        // будем получать данные разных размеров => подсчитаем все размеры сразу же
        recvCount = (int*) malloc(size * sizeof(int));
        displ = (int*) malloc(size * sizeof(int));
     
        for (int i = 0; i < size; ++i)
        {
            recvCount[i] = JSIZE * (minSize + ((i < m) ? 1 : 0));
            displ[i] = JSIZE * (minSize * i + std::min(m, i));
        }
    }

    // каждый поток берет строчки [currentBegin; currentEnd)
    currentBegin = minSize * rank + std::min(m, rank);
    currentEnd = currentBegin + minSize + ((rank < m) ? 1 : 0);

    // (!) Не параллелим первый цикл - каждый поток считает ту часть a, которая ему нужна
    // выделяем память в куче, т.к. размер двумерного массива большой для стэка
    double* a = (double*) malloc(sizeof(double) * ISIZE * JSIZE);
    for (int i = currentBegin; i < currentEnd; i++)
        for (int j = 2; j < JSIZE; j++)
            a[i * JSIZE + j] = sin(0.00001 * (10 * i + j));

    // (!) Параллелим второй цикл - массив b
    // нужно распараллелить операции над массивом размером (ISIZE - 5, JSIZE - 2) за счет сдвига
    double* b = (double*) calloc(ISIZE * JSIZE, sizeof(double)); // calloc для инициализации нулями
    for (int i = currentBegin + 5; i < currentEnd + 5; i++)
        for (int j = 0; j < JSIZE - 2; j++)
            b[i * JSIZE + j] = a[(i - 5) * JSIZE + j + 2] * 1.5;

    // собрать данные со всех потоков
    if (size > 1)
        MPI_Gatherv(b + (currentBegin + 5) * JSIZE, (currentEnd - currentBegin) * JSIZE, MPI_DOUBLE, b + 5 * JSIZE, recvCount, displ, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // стоп таймера - нет смысла мерить, сколько один поток пишет в файл
    MPI_Barrier(MPI_COMM_WORLD);
	double endTime = MPI_Wtime();

    if (rank == 0)
    {
#ifdef OUTPUT_TO_FILE
        FILE *ff;
        ff = fopen("result_personal_parallel.txt","w");
        
        // можем итерироваться двойным циклом
        for (int i = 0; i < ISIZE; ++i)
        {
            for (int j = 0; j < JSIZE; ++j)
                fprintf(ff, "%f ", b[i * JSIZE + j]);
        
            fprintf(ff, "\n");
        }

        fclose(ff);
#endif
        free(recvCount);
        free(displ);
    }

	if (rank == 0)
        printf("Ellapsed time: %g s\n", endTime - startTime);

    free(a);
    free(b);
    MPI_Finalize();
}