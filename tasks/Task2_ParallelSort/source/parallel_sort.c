#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define PROCESS_NULL(expr)                          \
do                                                  \
{                                                   \
    if (!expr)                                      \
    {                                               \
        perror("Unexpected NULL!");                 \
        exit(-1);                                   \
    }                                               \
} while (0);                                        \

#define PROCESS_ERROR(expr)                         \
do                                                  \
{                                                   \
    if (expr != 0)                                  \
    {                                               \
        perror("");                                 \
        exit(-1);                                   \
    }                                               \
} while (0);                                        \

#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <math.h>

#include "parallel_sort.h"

// Forward declaration
static void sequentialQuicksort(int* data, size_t left, size_t right, compatatorFunc comp);

// Data for each thread to calculate
typedef struct thread_data
{
    size_t left;
    size_t right;
    int* data;
    int (*comp)(int*, int*);
} thread_data_t;

// Thread routine with sequential quick sort
void* threadRoutine(void* arg)
{
    thread_data_t* tdata = (thread_data_t*) arg;
    sequentialQuicksort(tdata->data, tdata->left, tdata->right, tdata->comp);
}

static pthread_attr_t* createThreadAttributes(size_t nthreads)
{
    pthread_attr_t* attr = (pthread_attr_t*) malloc(nthreads * sizeof(*attr));
    PROCESS_NULL(attr);
 
    PROCESS_ERROR(pthread_attr_init(attr));
    size_t ncores = get_nprocs();

    cpu_set_t cpu_set;
    for (size_t i = 0; i < nthreads; ++i)
    {
        CPU_ZERO(&cpu_set);
        CPU_SET(i % ncores, &cpu_set);
        PROCESS_ERROR(pthread_attr_setaffinity_np(&attr[i], sizeof(cpu_set), &cpu_set));
    }

    return attr;
}

static int swap(int* data, size_t i, size_t j)
{
    int tmp = data[i];
    data[i] = data[j];
    data[j] = tmp;
}

static size_t doPivot(int* data, size_t left, size_t right, compatatorFunc comp)
{
    int pivot = data[right];
    size_t i = left - 1; // unsigned overflow won't be a problem

    for (size_t j = left; j <= right - 1; ++j)
        if (comp(&data[j], &pivot))
            swap(data, ++i, j);
    
    swap(data, i + 1, right);
    return i + 1;
}

static void sequentialQuicksort(int* data, size_t left, size_t right, compatatorFunc comp) {
    assert(data != NULL && comp != NULL);
    
    if (left < right)
    {
        size_t last = doPivot(data, left, right, comp);
        if (last > 0)
            sequentialQuicksort(data, left, last - 1, comp);

        sequentialQuicksort(data, last + 1, right, comp);
    }
    
}

typedef struct thread_merge
{
    thread_data_t* first;
    thread_data_t* second;
} thread_merge_t;

void mergeArrays(thread_merge_t* mergeData)
{
    size_t sizeFirst = mergeData->first->right - mergeData->first->left + 1;
    size_t sizeSecond = mergeData->second->right - mergeData->second->left + 1;
    size_t size = sizeFirst + sizeSecond;

    int* arrayFirst = mergeData->first->data;
    int* arraySecond = mergeData->second->data;
    int* array = (int*) malloc(size * sizeof(*array));
    
    // for (size_t i = 0; i < sizeFirst; ++i)
    //     printf("%d ", arrayFirst[i]);
    // printf("\n");
    // fflush(stdout);

    // for (size_t i = 0; i < sizeSecond; ++i)
    //     printf("%d ", arraySecond[i]);
    // printf("\n");
    // fflush(stdout);

    size_t i = 0, j = 0, k = 0;
    while (i < sizeFirst && j < sizeSecond)
    {
        if (mergeData->first->comp(&arrayFirst[i], &arraySecond[j]))
            array[k++] = arrayFirst[i++];
        else
            array[k++] = arraySecond[j++];
    }
    
    while (i < sizeFirst)
        array[k++] = arrayFirst[i++];
    while (j < sizeSecond)
        array[k++] = arraySecond[j++];

    // printf("after merge\n");
    // for (size_t i = 0; i < size; ++i)
    //     printf("%d ", array[i]);
    // printf("\n");

    free(mergeData->first->data);
    mergeData->first->data = array;
    mergeData->first->left = 0;
    mergeData->first->right = size - 1;
}

void* mergeRoutine(void* arg)
{
    mergeArrays((thread_merge_t*) arg);
}

// Algorithm will be as follows:
// 1) Main thread starts with choosing a pivot element and dividing the array in two parts,
// then he grabs the left part of it, and leaves the rest to the availible thread to perform a sequential quick sort
// 2) Main thread does that until he has no other threads, then he performs swquential algorithm on the rest
void parallelQuicksort(int* data, size_t size, compatatorFunc comp, size_t nthreads)
{
    assert(data != NULL && comp != NULL);

    if (nthreads == 1)
    {
        sequentialQuicksort(data, 0, size - 1, comp);
        return ;
    }

    // Main thread creates others, then proceeds to its part of job
    pthread_t* threadIds = (pthread_t*) malloc(nthreads * sizeof(*threadIds));
    pthread_attr_t* threadAttrs = createThreadAttributes(nthreads);
    thread_data_t* threadData = (thread_data_t*) malloc(nthreads * sizeof(*threadData));

    // Main thread data
    size_t m = (0 < size % nthreads) ? 1 : 0;
    size_t blockSize = size / nthreads + m;
    
    threadData[0].left = 0;
    threadData[0].right = blockSize - 1;
    threadData[0].data = memcpy(malloc(sizeof(*data) * blockSize), data, blockSize * sizeof(*data));
    threadData[0].comp = comp;

    data += blockSize;

    for (size_t i = 1; i < nthreads; ++i)
    {
        m = (i < size % nthreads) ? 1 : 0;
        blockSize = size / nthreads + m;

        threadData[i].left = 0;
        threadData[i].right = blockSize - 1;
        threadData[i].data = memcpy(malloc(sizeof(*data) * blockSize), data, blockSize * sizeof(*data));
        threadData[i].comp = comp;

        PROCESS_ERROR(pthread_create(&threadIds[i], &threadAttrs[i], threadRoutine, &threadData[i]));
        data += blockSize; // move pointer next, after data distribution ends we will return it to the starting position
    }

    data -= size;

    // Main thread does its job
    sequentialQuicksort(threadData[0].data, threadData[0].left, threadData[0].right, threadData[0].comp);
    
    // Join
    for (size_t i = 1; i < nthreads; ++i)
        pthread_join(threadIds[i], NULL);

    // for (size_t i = 0; i < nthreads; ++i)
    // {
    //     pthread_join(threadIds[i], NULL);
    //     printf("Thread %ld\n", i);
    //     for (size_t j = threadData[i].left; j <= threadData[i].right; ++j)
    //         printf("%d ", threadData[i].data[j]);
    //     printf("\n");
    // }

    // Merge all lists
    size_t counter = 1;
    size_t next = 1;
    thread_merge_t* mergeData = (thread_merge_t*) malloc(nthreads * sizeof(*mergeData));

    size_t end = ceil(log2(nthreads));
    while (counter <= end)
    {
        // Skip main for now
        for (size_t i = 2 * next; i < nthreads; i += 2 * next)
        {
            // Check if we need to merge with the next one
            if (i + next < nthreads)
            {
                mergeData[i].first = &threadData[i];
                mergeData[i].second = &threadData[i + next];
                // printf("Thread %ld, merge with %ld\n", i, i + counter);

                PROCESS_ERROR(pthread_create(&threadIds[i], &threadAttrs[i], mergeRoutine, &mergeData[i]));
            }
        }
        
        // Do main merge
        mergeData[0].first = &threadData[0];
        mergeData[0].second = &threadData[next];
        
        mergeArrays(&mergeData[0]);
        // Join
        for (size_t i = 2 * next; i < nthreads; i += 2 * next)
        {
            if (i + next < nthreads)
                pthread_join(threadIds[i], NULL);
        }

        counter++;
        next *= 2;
    }

    memcpy(data, mergeData[0].first->data, size * sizeof(*data));

    // Free resources
    free(mergeData);
    free(threadIds);

    for (size_t i = 0; i < nthreads; ++i)
        free(threadData[i].data);

    free(threadData);
    free(threadAttrs);
}