#ifndef NETWORK_H
#define NETWORK_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #define _GNU_SOURCE 

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <integral.h>
#include <input.h>
#include <cvector.h>

#define check_error_return(ret)                 \
do {                                            \
    if (ret < 0)                                \
    {                                           \
        perror("");                             \
        return EXIT_FAILURE;                    \
    }                                           \
} while (0)

#define check_zero_return(ret)                  \
do {                                            \
    if (ret == 0)                               \
    {                                           \
        perror("");                             \
        return EXIT_FAILURE;                    \
    }                                           \
} while (0)


#define MAX_MSG_SIZE 128
#define PORT 8760

// The minimum amout of processors for debug
#define MIN_PROC 4
// The amount of threads per each core of the CPU
#define THREADS_PER_CORE 2
// The number of the network interface
#define INTERFACE_NUMBER 1
// Broadcast message
#define BR_MSG "BRMSG"

// Structure that describes data to be sent to each computer
// @param a the start of the length to integrate on
// @param b the end of the length to integrate on
// @param nthreads the amount of threads to launch
typedef struct task 
{
    double a;
    double b;
} task_t;

typedef vector_define(task_t) task_vec_t;

typedef enum
{
    STATE_NONE,
    STATE_OK,
    STATE_TIMEOUT,
    STATE_DEAD
} node_state_t;

// Each node has assosiated values:
// - vector of tasks to be perfomed, we don't need to store result as it always being added to the sum
// - nreceive_tasks - how many tasks we need to receive from this node
// - socket to taslk to the node
// - threads - how many threads this node has
// - node state - if we connected to node and can't get results, mark it dead and distribute this task between all alive threads
typedef struct node
{
    int socket;
    int threads;
    node_state_t state;
    task_vec_t tasks;
    int nreceive_tasks;
} node_t;

#endif // #define NETWORK_H
