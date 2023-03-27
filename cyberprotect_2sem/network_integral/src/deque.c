#include <deque.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

typedef struct deque
{
    deque_value_t data[DEQUE_MAX_SIZE];
    int front;
    int back;
    int size;
} deque_t;

deque_t* deque_get()
{
    deque_t* deq = (deque_t*) malloc(sizeof(*deq));
    if (!deq)
        return NULL;
    
    deq->front = -1;
    deq->back = -1;
    deq->size = 0;
    return deq;
}

int deque_push_back(deque_t* deq, deque_value_t val)
{
    if (!deq)
        return -EINVAL;

    if((deq->front == 0 && deq->back == DEQUE_MAX_SIZE - 1) || (deq->front == deq->back + 1))
        return -EOVERFLOW;

    if (deq->front == -1)  // if queue is initially empty
    {   
        deq->front = 0;
        deq->back = 0;
    }
    else if(deq->back == DEQUE_MAX_SIZE - 1)  // back is at last position of queue
    {
        deq->back = 0;
    }
    else
    {
        deq->back = deq->back + 1;
    }

    deq->data[deq->back] = val;
    deq->size++;

    return 0;
}
 
int deque_push_front(deque_t* deq, deque_value_t val)
{
    if (!deq)
        return -EINVAL;

    if((deq->front == 0 && deq->back == DEQUE_MAX_SIZE - 1) || (deq->front == deq->back + 1))
        return -EOVERFLOW;
    
    if (deq->front == -1) // if queue is initially empty
    {
        deq->front = 0;
        deq->back = 0;
    }
    else if(deq->front == 0)
    {
        deq->front = DEQUE_MAX_SIZE - 1;
    }
    else
    {
        deq->front = deq->front - 1;
    }

    deq->data[deq->front] = val;  
    deq->size++;

    return 0;
}
 
int deque_pop_front(deque_t* deq, deque_value_t* valPtr)
{
    if (!deq)
        return -EINVAL;
    
    if (deq->front == -1)
        return -EOVERFLOW;

    if (valPtr)
        *valPtr = deq->data[deq->front];
    
    if (deq->front == deq->back) // queue has only one element
    {
        deq->front = -1;
        deq->back = -1;
    }
    else if (deq->front == DEQUE_MAX_SIZE - 1)
    {
        deq->front = 0;
    }
    else
    {
        deq->front = deq->front + 1;
    }

    deq->size--;
    return 0;
}
 
int deque_pop_back(deque_t* deq, deque_value_t* valPtr)
{
    if (!deq)
        return -EINVAL;
    
    if (deq->front == -1)
        return -EOVERFLOW;

    if (valPtr)
        *valPtr = deq->data[deq->back];
    
    if (deq->front == deq->back) // queue has only one element
    {
        deq->front = -1;
        deq->back = -1;
    }
    else if (deq->back == 0)
    {
        deq->back = DEQUE_MAX_SIZE - 1;
    }
    else
    {
        deq->back = deq->back - 1;
    }

    deq->size--;
    return 0;
}

const deque_value_t* deque_raw_buffer(deque_t* deq)
{
    if (!deq)
        return NULL;
    
    return deq->data;
}

int deque_size(deque_t* deq)
{
    if (!deq)
        return -1;

    return deq->size;
}

void deque_destroy(deque_t* deq)
{
    if (deq)
        free(deq);
}