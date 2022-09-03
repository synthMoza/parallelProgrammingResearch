#pragma once

#define _GNU_SOURCE      

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>       
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

#define min(a, b) ((a) > (b)) ? (b) : (a)

double integrate(double a, double b);
double thread_integrate(double a, double b, unsigned int nthreads);