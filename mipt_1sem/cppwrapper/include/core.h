#ifndef _MPI_CPP_WRAPPER_CORE_
#define _MPI_CPP_WRAPPER_CORE_

#include <mpi.h>
#include <iostream>

namespace mpi
{
	namespace detail
	{
		// Singleton initializer class for convinient single time initialization of MPI library
		class Initializer
		{
			Initializer(int* argc, char** argv[])
			{
				int err = MPI_Init(argc, argv);
				if (err != MPI_SUCCESS)
					throw std::exception("Initializer(): failed to initialize MPI library");
			}
			~Initializer()
			{
				MPI_Finalize();
			}
		public:
			static void Init(int* argc = NULL, char** argv[] = NULL)
			{
				static Initializer i(argc, argv);
			}
		};
	}

	// Initialize MPI library
	void Initialize(int* argc = NULL, char** argv[] = NULL) {
		detail::Initializer::Init();
	}
}

#endif // _MPI_CPP_WRAPPER_CORE_