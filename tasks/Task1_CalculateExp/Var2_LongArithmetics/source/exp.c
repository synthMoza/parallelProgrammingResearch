#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>
#include <mpi.h>

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

// Find the closest factorial to the given value
unsigned long findClosestFactorial(mpz_t value)
{
	mpz_t guess;
	mpz_init_set_ui(guess, 1);	

	for (unsigned long i = 1;;++i)
	{
		mpz_mul_ui(guess, guess, i);
		if (mpz_cmp(guess, value) >= 0)
		{
			mpz_clear(guess);
			return i;
		}
	}
}

// Calculate precision base on needed digits
// precision = 0.5 * 10^(-N)
// revPrecision = 2 * 10^(N)
void getRevPrecision(mpz_t revPrecision, unsigned long N)
{
	mpz_ui_pow_ui(revPrecision, 10, N);
	mpz_mul_ui(revPrecision, revPrecision, 2);	
}

// Calculate n! (using simple loop)
void calculateFactorial(mpz_t value, unsigned long n)
{
	mpz_set_ui(value, 1);
	for (unsigned int i = 0; i < n; ++i)
	{
		mpz_mul_ui(value, value, i + 1);
		//result *= (i + 1);
	}
}

// Calculate series: (1 / startRange! + 1 / (startRange + 1)! + ... + 1 / endRange!)
void calculateExp(mpf_t result, unsigned long startRange, unsigned long endRange)
{
	if (startRange > endRange)
	{
		return ;
	}
	
	mpf_t one;
	mpf_init_set_ui(one, 1);
	mpf_div_ui(result, one, endRange);
	//long double result = 1.0 / endRange;
	
	unsigned long endLoop = endRange - startRange;
	for (unsigned long i = 1; i < endLoop; ++i)
	{
		mpf_add_ui(result, result, 1);
		mpf_div_ui(result, result, endRange - i);
		//result = (1 + result) / (endRange - i);
	}	
	
	mpz_t value;
	mpz_init(value);
	
	calculateFactorial(value, startRange);
	mpf_add_ui(result, result, 1);
	mpf_t tmp;
	mpf_init(tmp);
	mpf_set_z(tmp, value);
	mpf_div(result, result, tmp);
	//result = (1 + result) / calculateFactorial(startRange);
	//return result;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./exp <N>\n");
		return EXIT_FAILURE;
	}

	// Measure time
	double start = 0, end = 0;

	// Initialize input argument
	unsigned long N = strtol(argv[1], NULL, 10);	
	
	// Set default precision for float
	mpf_set_default_prec(4 * N);

	int commSize = 0;
	int myRank = 0;

	MPI_Init(&argc, &argv);
	
	// Get rank and communicator size
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	if (N < commSize)
	{
		printf("Error: N must be greater or equal than number of processors\n");
                return EXIT_FAILURE;
	}
	
	unsigned long length = 0;
	if (myRank == 0)
	{
		// Zero thread will calculate the number of series members to calculate
		// We only need reverse precision (1 / epsilon)
		mpz_t revPrecision;
		mpz_init(revPrecision);
		getRevPrecision(revPrecision, N);
		// Need to find the closet factorial to the reverse precision
		length = findClosestFactorial(revPrecision);
		mpz_clear(revPrecision);
		// Now we are ready to deliever data to others	
	}	
	
	// Send length to all other threads
	MPI_Bcast(&length, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
	unsigned long m = length % commSize;
	unsigned long dataCount = length / commSize + ((myRank < m) ? 1 : 0);
	unsigned long startRange = (myRank < m) ? 1 + dataCount * myRank : 1 + m * (dataCount + 1) + (myRank - m) * dataCount;
	unsigned long endRange = startRange + dataCount - 1;

	mpf_t result;
	mpf_init_set_ui(result, 0);
	calculateExp(result, startRange, endRange);

	if (myRank == 0)
	{
		mpf_t tmp;
		mpf_init(tmp);
		for (unsigned int i = 1; i < commSize; ++i)
		{
			mpf_set_ui(tmp, 0);
			MPI_Recv(&tmp->_mp_prec, sizeof(tmp->_mp_prec), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&tmp->_mp_size, sizeof(tmp->_mp_size), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&tmp->_mp_exp, sizeof(tmp->_mp_exp), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(tmp->_mp_d, sizeof(mp_limb_t) * (tmp->_mp_prec + 1), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			mpf_add(result, result, tmp);
		}
	
		// Add missing 1
		mpf_add_ui(result, result, 1);
		gmp_printf("Result: e = %.*Ff\n", N + 1, result);
		
		mpf_clear(tmp);
	}
	else
	{
		// Send result to the zero thread
		MPI_Send(&result->_mp_prec, sizeof(result->_mp_prec), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&result->_mp_size, sizeof(result->_mp_size), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&result->_mp_exp, sizeof(result->_mp_exp), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(result->_mp_d, sizeof(mp_limb_t) * (result->_mp_prec + 1), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	end = MPI_Wtime();
	if (myRank == 0)
	{
		printf("Runtime = %g s\n", end - start);
	}

	mpf_clear(result);
	MPI_Finalize();
	return 0;
}
