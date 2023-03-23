#ifndef _MPI_CPP_WRAPPER_TRAITS_
#define _MPI_CPP_WRAPPER_TRAITS_

#include <mpi.h>

namespace mpi
{
	// Interface for all MPI datatypes traits
	template <typename T>
	struct datatype_traits
	{};

	template<>
	struct datatype_traits<char>
	{
		static const MPI_Datatype datatype = MPI_CHAR;
		using type = char;
	};

	template<>
	struct datatype_traits<wchar_t>
	{
		static const MPI_Datatype datatype = MPI_WCHAR;
		using type = wchar_t;
	};

	template<>
	struct datatype_traits<short>
	{
		static const MPI_Datatype datatype = MPI_SHORT;
		using type = short;
	};

	template<>
	struct datatype_traits<int>
	{
		static const MPI_Datatype datatype = MPI_INT;
		using type = int;
	};

	template<>
	struct datatype_traits<long>
	{
		static const MPI_Datatype datatype = MPI_LONG;
		using type = long;
	};

	template<>
	struct datatype_traits<signed char>
	{
		static const MPI_Datatype datatype = MPI_SIGNED_CHAR;
		using type = signed char;
	};

	template<>
	struct datatype_traits<unsigned char>
	{
		static const MPI_Datatype datatype = MPI_UNSIGNED_CHAR;
		using type = unsigned char;
	};

	template<>
	struct datatype_traits<unsigned short>
	{
		static const MPI_Datatype datatype = MPI_UNSIGNED_SHORT;
		using type = unsigned short;
	};

	template<>
	struct datatype_traits<unsigned>
	{
		static const MPI_Datatype datatype = MPI_UNSIGNED;
		using type = unsigned;
	};

	template<>
	struct datatype_traits<unsigned long>
	{
		static const MPI_Datatype datatype = MPI_UNSIGNED_LONG;
		using type = unsigned long;
	};

	template<>
	struct datatype_traits<float>
	{
		static const MPI_Datatype datatype = MPI_FLOAT;
		using type = float;
	};

	template<>
	struct datatype_traits<double>
	{
		static const MPI_Datatype datatype = MPI_DOUBLE;
		using type = double;
	};

	template<>
	struct datatype_traits<long double>
	{
		static const MPI_Datatype datatype = MPI_LONG_DOUBLE;
		using type = long double;
	};
}

#endif // _MPI_CPP_WRAPPER_TRAITS_