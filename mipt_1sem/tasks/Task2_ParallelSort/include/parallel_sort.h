#include <stddef.h>

#ifndef __PARALLEL_SORT__
#define __PARALLEL_SORT__

typedef int (*compatatorFunc)(int*, int*);

// @brief
// Sort the given array from 'left' element to 'right' one (typically 0 and size - 1) using quick sort algorithm and 'nthreads' threads
// @param data Pointer to the array with data
// @param left Left index of the array
// @param right Right index of the array
// @param comp Comparator for the given type of elements
void parallelQuicksort(int* data, size_t size, compatatorFunc comp, size_t nthreads);

#endif // __PARALLEL_SORT__