#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define ISIZE 1000
#define JSIZE 1000

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    double a[ISIZE][JSIZE];
    int i, j;

    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i][j] = 10*i +j;
        }
    }

    for (i=0; i<ISIZE; i++){
        for (j = 0; j < JSIZE; j++){
            a[i][j] = sin(0.00001*a[i][j]);
        }
    }

#ifdef OUTPUT_TO_FILE
    FILE *ff;
    ff = fopen("result_generic_sequential.txt", "w");
    for(i=0; i < ISIZE; i++){
        for (j=0; j < JSIZE; j++){
            fprintf(ff,"%f ",a[i][j]);
        }
    
        fprintf(ff,"\n");
    }

    fclose(ff);
#endif
}