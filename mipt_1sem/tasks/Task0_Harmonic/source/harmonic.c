#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

// All messages will have this tag
const int MSG_TAG = 727;

#define LOG_REAL(rank, msg, value)				\
do								\
{								\
	printf("Thread[%d]: %s %g\n", rank, msg, value);	\
}								\
while (0)							\

#define LOG_INT(rank, msg, value)				\
do								\
{								\
	printf("Thread[%d]: %s %d\n", rank, msg, value);	\
}								\
while (0)							\


// Calculates the prefix sum of harmonical series
// 1 / start + 1 / (start + step) + ... + 1 / (start + step * n)
float prefixSum(float start, float step, int n)
{
	if (start <= 0 || step <= 0 || n < 0)
	{
		return EXIT_FAILURE;
	}

	float result = 0;
	for (int i = 0; i < n; ++i)
	{
		result += 1.f / (start + i * step);
	}

	return result;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage ./harmonic <N>\n");
		return EXIT_FAILURE;
	}
	
	char* end = NULL;
	int N = strtol(argv[1], &end, 10);

	int my_rank = 0;	
	int commsize = 0;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &commsize);
	
	int n = 0;
	if (my_rank + 1 <= N)
		n = (N - my_rank - 1) / commsize + 1;
	
	#ifdef DEBUG
	LOG_INT(my_rank, "have n that equals", n);
	#endif
	float result = prefixSum(my_rank + 1, commsize, n);
	#ifdef DEBUG
	LOG_REAL(my_rank, "temp result is", result);
	#endif

	if (my_rank == 0)
	{
		// START TIME
		// float t1 = MPI_Wtime();
		
		// Receive all results
		float tmp = 0;
		for (int i = 1; i < commsize; ++i)
		{
			MPI_Recv(&tmp, 1, MPI_FLOAT, i, MSG_TAG, MPI_COMM_WORLD, &status);
			result += tmp;
		}
		
		// STOP TIME
		// float t2 = MPI_Wtime();
		
		printf("===========================\n");
		printf("\tWORKERS: %d\n", commsize);
		printf("\tINPUT NUM: %d\n", N);			
		printf("\tRESULT: %g\n", result);
		// printf("\tELAPSED TIME: %g s\n", t2 - t1);
		printf("===========================\n");	
	}	
	else
	{
		MPI_Send(&result, 1, MPI_FLOAT, 0, MSG_TAG, MPI_COMM_WORLD);		
	}	

	MPI_Finalize();
	return 0;
}
