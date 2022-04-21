#include "integral.h"
#include "input.h"

#include <math.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    float result = 0;
    long num_threads = input(argc, argv);

    if (num_threads < 1) {
        printf("Wrong threads number!\n");
        return EXIT_FAILURE;
    }

    const double a = 0.015;
    const double b = 1e6;
    result = thread_integrate(a, b, num_threads);

    printf("Number of threads: %ld\n", num_threads);
    printf("Result: %g\n", result);
    
    return EXIT_SUCCESS;
}