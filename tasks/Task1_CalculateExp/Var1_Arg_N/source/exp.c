#include <mpi.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG_REAL(rank, msg, value)				\
do								\
{								\
	printf("Thread[%d]: %s %Lg\n", rank, msg, value);	\
}								\
while (0)							\

#define LOG_INT(rank, msg, value)				\
do								\
{								\
	printf("Thread[%d]: %s %d\n", rank, msg, value);	\
}								\
while (0)							\

// Calculate n! (using simple loop)
long double calculateFactorial(unsigned int n)
{
	long double result = 1;
	for (unsigned int i = 0; i < n; ++i)
	{
		result *= (i + 1);
	}

	return result;
}

// Calculate series: (1 / startRange! + 1 / (startRange + 1)! + ... + 1 / endRange!) 
long double calculateExp(unsigned int startRange, unsigned int endRange)
{
	if (startRange > endRange)
	{
		return -1;
	}

	long double result = 1.0 / endRange;
	
	unsigned int endLoop = endRange - startRange;
	for (unsigned int i = 1; i < endLoop; ++i)
	{
		result = (1 + result) / (endRange - i);
	}	

	result = (1 + result) / calculateFactorial(startRange);
	return result;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./exp <N>\n");
		return EXIT_FAILURE;
	}

	int commSize = 0;
	int myRank = 0;

	// Get input argument
	unsigned long N = strtol(argv[1], NULL, 10);

	// Initialize MPI
	MPI_Init(&argc, &argv);
	
	// Get rank and communicator size
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	// Abort if N < commsize
	if (N < commSize)
	{
		printf("Error: N must be greater or equal than number of processors.\n");
		return EXIT_FAILURE;
	}

	// Start measuring working time
	//MPI_Barrier(MPI_COMM_WORLD);
	//float t1 = MPI_Wtime();
	
	// Determine which path of series should calculate
	unsigned int m = N % commSize;
	unsigned int dataCount = N / commSize + ((myRank < m) ? 1 : 0);
#ifdef DEBUG
	LOG_INT(myRank, "m equals", m);
	LOG_INT(myRank, "dataCount equals", dataCount);
#endif

	unsigned int startRange = (myRank < m) ? 1 + dataCount * myRank : 1 + m * (dataCount + 1) + (myRank - m) * dataCount;
	unsigned int endRange = startRange + dataCount - 1;
#ifdef DEBUG
	LOG_INT(myRank, "startRange equals", startRange);
	LOG_INT(myRank, "endRange equals", endRange);
#endif
	long double result = calculateExp(startRange, endRange);
#ifdef DEBUG
	LOG_REAL(myRank, "tmp result is", result);
#endif	

	// Stop measuring working time
	//MPI_Barrier(MPI_COMM_WORLD);
	//float t2 = MPI_Wtime();
	
	if (myRank == 0)
	{
		// Collect results
		for (unsigned int i = 1; i < commSize; ++i)
		{
			long double tmp = 0;
			MPI_Recv(&tmp, 1, MPI_LONG_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			result += tmp;
		}
		
		result += 1;
		printf("Given argument: N = %u\n", N);
		printf("Result: e = %.20Lg\n", result);
		//printf("Elapsed time: %.0f s\n", t2 - t1);
	}
	else
	{
		MPI_Send(&result, 1, MPI_LONG_DOUBLE, 0, 0, MPI_COMM_WORLD);	
	}
	
	MPI_Finalize();

	return EXIT_SUCCESS;
}
