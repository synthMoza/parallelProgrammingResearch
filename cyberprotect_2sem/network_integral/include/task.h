#ifndef TASK_HEADER
#define TASK_HEADER

// Structure that describes data to be sent to each computer
// @param a the start of the length to integrate on
// @param b the end of the length to integrate on
// @param nthreads the amount of threads to launch
typedef struct task 
{
    double a;
    double b;
} task_t;

#endif // #define TASK_HEADER
 