#ifndef _MPI_CPP_WRAPPER_COMM_
#define _MPI_CPP_WRAPPER_COMM_

#include "traits.h"

namespace mpi
{
	template <MPI_Comm C>
	class Comm
	{
	public:
		static int GetRank()
		{
			int rank;
			int err = MPI_Comm_rank(C, &rank);
			if (err != MPI_SUCCESS)
				throw std::exception("MPI_Comm_rank exception"); // TODO: implement better exceptions
			return rank;
		}
		static int GetSize()
		{
			int size;
			int err = MPI_Comm_size(C, &size);
			if (err != MPI_SUCCESS)
				throw std::exception("MPI_Comm_size exception"); // TODO: implement better exceptions
			return size;
		}
		template<typename T>
		static void Send(const T& value, int dest, int tag)
		{
			int err = MPI_Send(&value, 1, datatype_traits<T>::datatype, dest, tag, C);
			if (err != MPI_SUCCESS)
				throw std::exception("Send(): failed to send data");
		}
		template<typename T, size_t N>
		static void Send(const T (&value)[N], int dest, int tag)
		{
			int err = MPI_Send(value, N, datatype_traits<T>::datatype, dest, tag, C);
			if (err != MPI_SUCCESS)
				throw std::exception("Send(): failed to send data");
		}
		template <typename T>
		static MPI_Status Recv(T& value, int src, int tag = MPI_ANY_TAG)
		{
			MPI_Status status;
			int err = MPI_Recv(&value, 1, datatype_traits<T>::datatype, src, tag, C, &status);
			if (err != MPI_SUCCESS)
				throw std::exception("Send(): failed to receive data");
			return status;
		}
		template <typename T, size_t N>
		static MPI_Status Recv(T (&value)[N], int src, int tag = MPI_ANY_TAG)
		{
			MPI_Status status;
			int err = MPI_Recv(value, N, datatype_traits<T>::datatype, src, tag, C, &status);
			if (err != MPI_SUCCESS)
				throw std::exception("Send(): failed to receive data");
			return status;
		}
	};

	// Usings for more comfortable usage
	using Comm_World = Comm<MPI_COMM_WORLD>;
	using Comm_Self = Comm<MPI_COMM_SELF>;
}

#endif // _MPI_CPP_WRAPPER_COMM_