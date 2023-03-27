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
#include <deque.h>

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

typedef enum
{
    STATE_NONE,
    STATE_OK,
    STATE_TIMEOUT,
    STATE_DEAD
} node_state_t;

// Each node has assosiated values:
// - deque of tasks to be perfomed, we don't need to store result as it always being added to the sum
// - nreceive_tasks - how many tasks we need to receive from this node
// - socket to taslk to the node
// - threads - how many threads this node has
// - node state - if we connected to node and can't get results, mark it dead and distribute this task between all alive threads
typedef struct node
{
    int socket;
    long threads;
    node_state_t state;
    deque_t* tasks;
} node_t;

#endif // #define NETWORK_H
