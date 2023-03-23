#include <integral.h>

#include <math.h>
#include <errno.h>
#include <sched.h>

double integrate(func_t f, double a, double b)
{
    unsigned int mesh = (int)(b - a) * 25; // number of lengths for numerical integration
    double eps = 1e-06; // the accuracy of calculations
    double h = (b - a) / mesh;
    double result = 0;
    double x = a + h; // the sequency of arguments

    while (x - eps < b) {
        result += 4 * f(x);
        x += h;
        if (x + eps >= b) // if the argument is out of the given range
            break;
        result += 2 * f(x);
        x += h;
    }

    result = (h / 3) * (f(a) + result + f(b));
    if (result > -eps && result < eps)
        result = 0;

    return result; 
}

struct thread_data 
{
    func_t f; // function to integrate
    double a; // the start of the length
    double b; // the end of the length
    double result;
};

static void* threadFunction(void* thread_data) 
{
    struct thread_data* thread_data_t = (struct thread_data*) thread_data;
    thread_data_t->result = integrate(thread_data_t->f, thread_data_t->a, thread_data_t->b);
    pthread_exit(EXIT_SUCCESS);
}

// Prepare data structures for each active thread
static int wr_data_threads(struct thread_data** thread_data, int nthreads, func_t f, double a, double b) 
{
    int page_size = getpagesize();  // the size of memory page
    double diff = (b - a) / nthreads;
    
    // Write data for each active thread
    for (int i = 0; i < nthreads; ++i) 
    {
        thread_data[i] = (struct thread_data*) memalign(page_size, sizeof(struct thread_data));
        if (thread_data[i] == NULL) 
            return -1;

        thread_data[i]->f = f;
        thread_data[i]->a = a + i * diff;
        thread_data[i]->b = a + (i + 1) * diff;
    }

    return 0;
}

static int threads_create_atrributes(pthread_attr_t* pthread_attr, int nthreads) 
{
    int nproc = get_nprocs(); // maximum number of threads
    cpu_set_t cpu_set = {};

    // Create attributes for all threads
    int ncores = (nthreads - nproc / 2); // the number of cores to be linked with two or more threads
    if (ncores < 0)
        ncores = 0; // if ncores is less than zero, no threads must be put by two on one CPU
    
    int dnthreads = min(ncores * 2, nthreads); // threads to be put by two
    if (dnthreads != 0) 
    {
        for (int i = 0; i < dnthreads; ++i) 
        {
            if (pthread_attr_init(&pthread_attr[i]) != 0)
            {
                for (int j = 0; j < i; ++j)
                    pthread_attr_destroy(&pthread_attr[j]);
                
                return -1;
            }
            
            // Link threads to cores
            CPU_ZERO(&cpu_set);
            CPU_SET(i % nproc, &cpu_set);

            if (pthread_attr_setaffinity_np(&pthread_attr[i], sizeof(cpu_set), &cpu_set) != 0) 
            {
                for (int j = 0; j <= i; ++j)
                    pthread_attr_destroy(&pthread_attr[j]);
                
                return -1;
            }
        }
    }

    // If there are any threads left, put them on other cores
    if (dnthreads < nthreads) 
    {
        for (int i = dnthreads; i < nthreads; ++i) 
        {
            // Put an active thread on the core
            if (pthread_attr_init(&pthread_attr[i]) != 0) 
            {
                for (int j = 0; j < i; ++j)
                    pthread_attr_destroy(&pthread_attr[j]);
                
                return -1;
            }
            
            // Link threads to cores
            CPU_ZERO(&cpu_set);
            CPU_SET(dnthreads + 2 * (i - dnthreads), &cpu_set);

            if (pthread_attr_setaffinity_np(&pthread_attr[i], sizeof(cpu_set), &cpu_set) != 0) 
            {
                for (int j = 0; j <= i; ++j)
                    pthread_attr_destroy(&pthread_attr[j]);
                
                return -1;
            }
        }
    }

    return 0;
}

int thread_integrate(func_t f, double a, double b, int nthreads, double* result) 
{
    int ret_code = 0;
    struct thread_data** thread_data = NULL;
    pthread_t* thread_ids = NULL;
    pthread_attr_t* pthread_attr = NULL; // attributes of creating threads

    // Allocate space for threads data and attributes
    pthread_attr = (pthread_attr_t*) calloc(nthreads, sizeof(*pthread_attr));
    if (!pthread_attr)
        return -ENOMEM;
    
    thread_data = (struct thread_data**) calloc(nthreads, sizeof(*thread_data));
    if (!thread_data)
    {
        ret_code = -ENOMEM;
        goto exit_thread_integrate_1;
    }

    thread_ids = (pthread_t*) calloc(nthreads, sizeof(*thread_ids));
    if (!thread_ids)
    {
        ret_code = -ENOMEM;
        goto exit_thread_integrate_2;
    }

    // Write data for each active and dummy thread
    if (wr_data_threads(thread_data, nthreads, f, a, b) < 0)
    {
        ret_code = -ENOMEM;
        goto exit_thread_integrate_3;
    }

    // Create attributes for all threads
    if (threads_create_atrributes(pthread_attr, nthreads) < 0)
    {
        ret_code = -1;
        goto exit_thread_integrate_3;
    }

    // Launch all threads
    for (int i = 0; i < nthreads; ++i) 
    {    
        int ret = pthread_create(&thread_ids[i], &pthread_attr[i], threadFunction, thread_data[i]);
        pthread_attr_destroy(&pthread_attr[i]);
        if (ret != 0)
        {
            ret_code = -1;
            goto exit_thread_integrate_3;
        }
    }

    // Collect results from all active threads
    for (int i = 0; i < nthreads; ++i) 
    {
        pthread_join(thread_ids[i], NULL);
        
        *result += thread_data[i]->result;
        free(thread_data[i]);
    }

exit_thread_integrate_3:
    free(thread_ids);    
exit_thread_integrate_2:
    free(thread_data);
exit_thread_integrate_1:
    free(pthread_attr);

    return ret_code;
}