#include "integral.h"

#include <math.h>
#include <sys/time.h>

// Function to integrate
// For this particular task, it is given
// @param x the argument of the function
double function(double x) {
    double f = sin(1 / x);
    return f;
}

const size_t pointsPerPeriod = 100;

// Integrate the function on the given length
// @param a the start of the length
// @param b the end of the length
double integrate(double a, double b) {
    double eps = 1e-06; // the accuracy of calculations
    double result = 0;

    // Determine first dynamic step
    // Dynamic step tries to keep number of points in each period constant
    int n = 1 / (M_PI * a);
    if (n == 0) // if we start outside finite periods, calculate step as we were in the first period
        n = 1;
    
    double periodLength = 1 / (M_PI * n * (n + 1));
    double periodEnd = 1 / (M_PI * n);
    double step = periodLength / pointsPerPeriod;

    double x = a;
    while (x - eps < b) {
        // Check if we need to change step
        // If it was the last period, don't change it and continue with that till the end
        if (x > periodEnd && n != 1) {
            n--;
            periodLength = 1 / (M_PI * n * (n + 1));
            periodEnd = 1 / (M_PI * n);
            step = periodLength / pointsPerPeriod;
        }

        // Trapezoidal rule
        double tmp = function(x);
        x += step;
        if (x + eps >= b) // if the argument is out of the given range
            break;
        tmp += function(x);
        tmp *= 0.5 * step;

        result += tmp;
    }

    // Last iteration with b
    x -= step;
    result += 0.5 * (function(b) + function(x)) * step;

    if (result > -eps && result < eps)
        result = 0;

    return result; 
}

struct thread_data {
    double a; // the start of the length
    double b; // the end of the length
    double result;
    int isDummy;
};

void* threadFunction(void* thread_data) {
    struct timeval begin, end, interval;
    struct thread_data* thread_data_t = (struct thread_data*) thread_data;

    gettimeofday(&begin, NULL);
    thread_data_t->result = integrate(thread_data_t->a, thread_data_t->b);
    gettimeofday(&end, NULL);

    timersub(&end, &begin, &interval);
    if (thread_data_t->isDummy == 0)
        printf("Thread %p: took %ld.%06ld sec\n", (void*) pthread_self(), interval.tv_sec, interval.tv_usec);

    pthread_exit(EXIT_SUCCESS);
}

// Prepare data structures for each active thread
void wr_data_active_threads(struct thread_data** thread_data, int nthreads, double a, double b) {
    int page_size = getpagesize();  // the size of memory page
    double diff = (b - a) / nthreads;
    // Write data for each active thread
    for (int i = 0; i < nthreads; ++i) {
        thread_data[i] = (struct thread_data*) memalign(page_size, sizeof(struct thread_data));
        //thread_data[i] = (struct thread_data*) calloc(1, sizeof(struct thread_data));
        if (thread_data[i] == NULL) {
            printf("Error! Can't allocate memory for thread data number %d!\n", i);
            exit(EXIT_FAILURE);
        }
  
        thread_data[i]->a = a + i * diff;
        thread_data[i]->b = a + (i + 1) * diff;
        thread_data[i]->isDummy = 0;
    }
}

// Prepare data structures for each dummy thread
void wr_data_dummy_threads(struct thread_data** thread_data, int nthreads, int ndummythreads, double a, double b) {
    int page_size = getpagesize();  // the size of memory page
    double diff = (b - a) / nthreads;
    for (int j = nthreads; j < nthreads + ndummythreads; ++j) {
        thread_data[j] = (struct thread_data*) memalign(page_size, sizeof(struct thread_data));
        //thread_data[j] = (struct thread_data*) calloc(1, sizeof(struct thread_data));
        if (thread_data[j] == NULL) {
            printf("Error! Can't allocate memory for thread data number %d!\n", j);
            exit(EXIT_FAILURE);
        }
  
        thread_data[j]->a = a;
        thread_data[j]->b = a + diff;
        thread_data[j]->isDummy = 1;
    }
}

void threads_create_atrributes(pthread_attr_t* pthread_attr, int nthreads, int ndummythreads) {
    int nproc = get_nprocs(); // maximum number of threads
    int ret = 0;
    cpu_set_t cpu_set;

    // Create attributes for all threads
    int ncores = (nthreads - nproc / 2); // the number of cores to be linked with two or more threads
    if (ncores < 0)
        ncores = 0; // if ncores is less than zero, no threads must be put by two on one CPU
    int dnthreads = min(ncores * 2, nthreads); // threads to be put by two

    if (dnthreads != 0) {
        for (int i = 0; i < dnthreads; ++i) {
            ret = pthread_attr_init(&pthread_attr[i]);
            if (ret != 0) {
                printf("Error getting default attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
            
            // Link threads to cores
            CPU_ZERO(&cpu_set);
            CPU_SET(i % nproc, &cpu_set);

            ret = pthread_attr_setaffinity_np(&pthread_attr[i], sizeof(cpu_set), &cpu_set);
            if (ret != 0) {
                printf("Error setting affinity attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
        }
    }

    int dummy_index = nthreads; // the first dummy thread index

    // If there are any threads left, put them on other cores (and add dummy threads)
    if (dnthreads < nthreads) {
        for (int i = dnthreads; i < nthreads; ++i) {
            // Put an active thread on the core
            ret = pthread_attr_init(&pthread_attr[i]);
            if (ret != 0) {
                printf("Error getting default attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
            
            // Link threads to cores
            CPU_ZERO(&cpu_set);
            CPU_SET(dnthreads + 2 * (i - dnthreads), &cpu_set);

            ret = pthread_attr_setaffinity_np(&pthread_attr[i], sizeof(cpu_set), &cpu_set);
            if (ret != 0) {
                printf("Error setting affinity attributes for active threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }

            // Put an dummy thread on that PSYSICAL core too
            // and set its state to detachable
            ret = pthread_attr_init(&pthread_attr[dummy_index]);
            if (ret != 0) {
                printf("Error getting default attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
            
            CPU_ZERO(&cpu_set);
            CPU_SET(dnthreads + 2 * (i - dnthreads) + 1, &cpu_set);

            ret = pthread_attr_setaffinity_np(&pthread_attr[dummy_index], sizeof(cpu_set), &cpu_set);
            if (ret != 0) {
                printf("Error setting affinity attributes for dummy threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
            ret = pthread_attr_setdetachstate(&pthread_attr[dummy_index], PTHREAD_CREATE_DETACHED);
            if (ret != 0) {
                printf("Error setting detache state attributes for dummy threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }

            dummy_index++;
        }
    }

    // If there are any dummy threads left, put them on all other cores
    if (dummy_index < nthreads + ndummythreads && ndummythreads != 0) {
        for (int i = dummy_index; i < ndummythreads + nthreads; ++i) {
            ret = pthread_attr_init(&pthread_attr[i]);
            if (ret != 0) {
                printf("Error getting default attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
            
            // Link threads to cores
            CPU_ZERO(&cpu_set);
            CPU_SET(i % nproc, &cpu_set);

            ret = pthread_attr_setaffinity_np(&pthread_attr[i], sizeof(cpu_set), &cpu_set);
            if (ret != 0) {
                printf("Error setting affinity attributes for threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }

            ret = pthread_attr_setdetachstate(&pthread_attr[i], PTHREAD_CREATE_DETACHED);
            if (ret != 0) {
                printf("Error setting detache state attributes for dummy threads!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
        }
    }
}

double thread_integrate(double a, double b, unsigned int nthreads) {
    struct thread_data** thread_data = NULL;
    double result = 0;
    pthread_t* thread_ids = NULL;
    int ret = 0;
    pthread_attr_t* pthread_attr = NULL; // attributes of creating threads
    int ndummythreads = 0; // number of threads to do useless calculations

    // Get information about the system
    int nproc = get_nprocs(); // maximum number of threads

    // Other avalible cores will be filled
    if (nthreads < nproc)
        ndummythreads = nproc - nthreads;

    // Allocate space for threads data and attributes (and dummy threads too)
    pthread_attr = (pthread_attr_t*) calloc(nthreads + ndummythreads, sizeof(*pthread_attr));
    thread_data = (struct thread_data**) calloc(nthreads + ndummythreads, sizeof(*thread_data));
    thread_ids = (pthread_t*) calloc(nthreads + ndummythreads, sizeof(*thread_ids));

    // Write data for each active and dummy thread
    wr_data_active_threads(thread_data, nthreads, a, b);
    wr_data_dummy_threads(thread_data, nthreads, ndummythreads, a, b);

    // Create attributes for all threads
    threads_create_atrributes(pthread_attr, nthreads, ndummythreads);

    // Launch all threads
    // NOTE: disable dummy thread for now
    for (int i = 0; i < nthreads + ndummythreads; ++i) {    
            ret = pthread_create(&thread_ids[i], &pthread_attr[i], threadFunction, thread_data[i]);
            pthread_attr_destroy(&pthread_attr[i]);
            if (ret != 0) {
                printf("Error creating thread!\n");
                printf("return - %d\n", ret);
                exit(EXIT_FAILURE);
            }
    }

    // Collect results from all active threads
    for (int i = 0; i < nthreads; ++i) {
        pthread_join(thread_ids[i], NULL);
        
        result += thread_data[i]->result;
        free(thread_data[i]);
    }
    
    // Free dummy threads data
    for (int i = nthreads; i < nthreads + ndummythreads; ++i) {
        free(thread_data[i]);
    }

    // Free everything else
    free(pthread_attr);
    free(thread_data);
    free(thread_ids);

    return result;
}