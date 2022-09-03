#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "parallel_sort.h"
#include "input.h"

void printArray(FILE* file, int* array, size_t arraySize)
{
    for (size_t i = 0; i < arraySize; ++i)
        fprintf(file, "%d ", array[i]);
    
    fprintf(file, "\n");
}

int integerComp(int* a, int* b)
{
    return *a < *b;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        printf("Usage: ./parallel_sort <number-of-threads> [<filename>]>\n");
        return EXIT_FAILURE;
    }

    long nthreads = input(argv[1]);

    FILE* outputFile = (argc == 3) ? fopen(argv[2], "w") : stdout;
    if (!outputFile)
    {
        printf("Error: file \"%s\" can not be opened\n", argv[5]);
        return EXIT_FAILURE;
    }

    if (nthreads <= 0)
    {
        printf("Error: number of threads must be a positive number\n");
        return EXIT_FAILURE;
    }

    size_t arraySize = 0;
    int* array = NULL;
    
    // Input array
    scanf("%lu", &arraySize);
    if (arraySize < nthreads)
    {
        printf("Error: array size is too smal for nthis number of threads\n");
        return EXIT_FAILURE;
    }

    array = (int*) malloc(sizeof(*array) * arraySize);

    for (size_t i = 0; i < arraySize; ++i)
        scanf("%d", &array[i]);

    // Start timer
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    parallelQuicksort(array, arraySize, integerComp, nthreads);
    // End timer
    gettimeofday(&end, 0);

    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    printf("Ellapsed time: %.3f\n", elapsed);

    printArray(outputFile, array, arraySize);
    free(array);

    return 0;
}