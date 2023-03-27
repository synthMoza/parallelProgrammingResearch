#ifndef DEQUE_HEADER
#define DEQUE_HEADER

#include <task.h>

// for simplicity, use one array
#define DEQUE_MAX_SIZE 512

typedef task_t deque_value_t; 
typedef struct deque deque_t;

deque_t* deque_get();

int deque_push_back(deque_t* deq, deque_value_t val);
int deque_push_front(deque_t* deq, deque_value_t val);
int deque_pop_front(deque_t* deq, deque_value_t* valPtr);
int deque_pop_back(deque_t* deq, deque_value_t* valPtr);
int deque_size(deque_t* deq);

const deque_value_t* deque_raw_buffer(deque_t* deq);
void deque_destroy(deque_t* deq);

#endif // #define DEQUE_HEADER
 