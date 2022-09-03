#include <mpi.h>
#include <stdio.h>

// Initial value of message
const int MSG_INITIAL_VALUE = 1;
// All message have the same tag
const int MSG_TAG = 0;

#define LOG(rank, str, value)					\
do								\
{								\
	printf("Thread[%d]: %s %d\n", rank, str, value);	\
}								\
while (0);							\

int main(int argc, char* argv[])
{
	MPI_Status status;

	int commsize = 0;
	int my_rank = 0;
	int message = 0;		

	MPI_Init(&argc, &argv);
	
	// Get comm size (to discover numer of executors)
	MPI_Comm_size(MPI_COMM_WORLD, &commsize);
	// Get rank to understand where to send next
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	if (my_rank == 0)
	{
		// First one has to set the initial value and send to the next one, then receive from the first one
		message = MSG_INITIAL_VALUE;
		LOG(my_rank, "Initial value of the message is", message);
		MPI_Send(&message, 1, MPI_INTEGER, my_rank + 1, MSG_TAG, MPI_COMM_WORLD);
		MPI_Recv(&message, 1, MPI_INTEGER, commsize - 1, MSG_TAG, MPI_COMM_WORLD, &status);
		LOG(my_rank, "Final value of message is", message);
	}
	else if (my_rank == commsize - 1)
	{
		// Last thread must send it to the first one
		MPI_Recv(&message, 1, MPI_INTEGER, my_rank - 1, MSG_TAG, MPI_COMM_WORLD, &status);
		message += 1;
		LOG(my_rank, "Modified value of message is", message);
		MPI_Send(&message, 1, MPI_INTEGER, 0, MSG_TAG, MPI_COMM_WORLD);
	}
	else
	{
		// Any other threads receives from the previous one and sends to the next one
		MPI_Recv(&message, 1, MPI_INTEGER, my_rank - 1, MSG_TAG, MPI_COMM_WORLD, &status);
		message += 1;
		LOG(my_rank, "Modified value of message is", message);
		MPI_Send(&message, 1, MPI_INTEGER, my_rank + 1, MSG_TAG, MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
