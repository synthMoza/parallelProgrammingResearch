#include <iostream>
#include <cmath>
#include <mpi.h>

#include "TransportEquationSolver.h"

// For example, let's take the following equation:
// u_t + 2 * u_x = x + t
// 0 < x < 1, 0 < t < 1
// u(x, 0) = cos(pi * x)
// u(0, t) = exp(-t)
// =================================================
double a(double x, double t)
{
	return 2;
}

double f(double x, double t)
{
	return x + t;
}

const double pi = 3.14159265358979323846;

double phi(double x)
{
	return std::cos(pi * x);
}

double mu(double t)
{
	return std::exp(-t);
}
// =================================================

int main(int argc, char* argv[])
{
	// Input filename to output data
	if (argc < 2)
	{
		std::cout << "Usage: ./transportEquationLab <filename>" << std::endl;
		return EXIT_FAILURE;
	}

	// Mesh size
	const std::size_t N = 1600;
	const std::size_t J = 3200;

	// Initialize MPI library
	MPI_Init(&argc, &argv);

	se::TransportEquationSolver solver(a, f, phi, mu, N, J);
	solver.Solve(argv[1]);

	// Finalize MPI library
	MPI_Finalize();
	return EXIT_SUCCESS;
}