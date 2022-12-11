#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define ISIZE 15000
#define JSIZE 15000

int main()
{
    int i, j;
    
    double* a = (double*) malloc(sizeof(double) * ISIZE * JSIZE);
    double* b = (double*) malloc(sizeof(double) * ISIZE * JSIZE);

    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i * JSIZE + j] = 10*i +j;
            b[i * JSIZE + j] = 0.;
        }
    }

    for (i=0; i<ISIZE; i++){
        for (j = 0; j < JSIZE; j++){
            a[i * JSIZE + j] = sin(0.00001*a[i * JSIZE + j]);
        }
    }

    for (i=5; i<ISIZE; i++){
        for (j = 0; j < JSIZE-2; j++){
            b[i * JSIZE + j] = a[(i - 5) * JSIZE + j + 2] * 1.5;
        }
    }

#ifdef OUTPUT_TO_FILE
    FILE *ff;
    ff = fopen("result_personal_sequential.txt","w");
    for(i=0; i < ISIZE; i++){
        for (j=0; j < JSIZE; j++){
            fprintf(ff, "%f ", b[i * JSIZE + j]);
        }
        
        fprintf(ff,"\n");
    }

    fclose(ff);
#endif

    free(a);
    free(b);

}