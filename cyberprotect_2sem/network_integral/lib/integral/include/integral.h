#ifndef INTEGRAL_H
#define INTEGRAL_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>       
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

#define min(a, b) ((a) > (b)) ? (b) : (a)

typedef double (*func_t)(double);

// Integrate the function on the given length
// @param a the start of the length
// @param b the end of the length
// @param f function
// @return integral value
double integrate(func_t f, double a, double b);

// Integrate the function on the given length using threads
// @param a the start of the length
// @param b the end of the length
// @param nthreads the amount of threads to be launched
// @param f function
// @param result pointer to the result variable
// @return 0 on succes or -1 on error
int thread_integrate(func_t f, double a, double b, int nthreads, double* result);

#endif // #define INTEGRAL_H
