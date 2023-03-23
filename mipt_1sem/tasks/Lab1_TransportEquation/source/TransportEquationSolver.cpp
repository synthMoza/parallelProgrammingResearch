#include <mpi.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>

#include "TransportEquationSolver.h"

using namespace se;

#define PROCESS_ERROR(expr)													\
	do {																	\
		if (expr < 0) {														\
			std::stringstream errorStream;									\
			errorStream << "Error while executing " << #expr				\
				<< ", function " << __func__ << ", line "					\
				<< __LINE__ << std::endl;									\
			throw std::runtime_error(errorStream.str());					\
		}																	\
	} while (0);															\

void TransportEquationSolver::PrintMesh(Mesh & mesh, std::ostream & stream)
{
	for (auto& row : mesh)
	{
		for (std::size_t i = 0; i < row.size() - 1; ++i)
			stream << row[i] << ", ";
		stream << row.back() << std::endl;
	}
}

Mesh TransportEquationSolver::GetEmptyMesh(std::size_t J, std::size_t N)
{
	Mesh result(J);
	for (auto& row : result)
		row.resize(N);
	return result;
}

void TransportEquationSolver::SetBoundaryConditions(Mesh& mesh, int myRank, int commSize)
{
	auto meshRows = mesh.size();
	
	if (meshRows == m_J) 
	{
		for (std::size_t i = 0; i < meshRows; ++i)
			mesh[i][0] = m_mu(i * m_tau);
		for (std::size_t n = 0; n < mesh[0].size(); ++n)
			mesh[0][n] = m_phi(m_x_0.first + n * m_h);
	}
	else
	{
		std::size_t rowCount = mesh.size() / 2;
		for (std::size_t i = 0; i < rowCount; ++i)
		{
			mesh[2 * i][0] = m_mu((myRank + i * commSize) * m_tau);
			mesh[2 * i + 1][0] = m_mu((myRank + 1 + i * commSize) * m_tau);
		}
	}
}

void TransportEquationSolver::Solve(const char* fileName)
{
	MPI_Barrier(MPI_COMM_WORLD);
	double startTime = MPI_Wtime();

	// Get communicator size and rank
	int commSize = 0, myRank = 0;
	PROCESS_ERROR(MPI_Comm_size(MPI_COMM_WORLD, &commSize));
	PROCESS_ERROR(MPI_Comm_rank(MPI_COMM_WORLD, &myRank));

	// Main thread (0) will collect all data and own a mesh, others won't have a mesh to optimize memory allocations
	// So we need a little different routines
	Mesh mesh;
	if (myRank == 0)
		mesh = MainThreadRoutine(commSize);
	else
		OtherThreadRoutine(myRank, commSize);
	
	MPI_Barrier(MPI_COMM_WORLD);
	double endTime = MPI_Wtime();

	if (myRank == 0)
	{
		std::cout << "Ellapsed time: " << endTime - startTime << " s" << std::endl;

		std::fstream fstream;
		fstream.open(fileName, std::fstream::out);
		PrintMesh(mesh, fstream);
	}
}

double TransportEquationSolver::CalculateMainNode(double y_00, double y_01, double y_10, double x, double t)
{
	double a = m_a(x, t);
	return 2 * m_h * m_tau / (m_h + m_tau * a) * m_f(x + m_h / 2, t + m_tau / 2) +
		(a * m_tau - m_h) / (a * m_tau + m_h) * (y_01 - y_10) + y_00;
}

Mesh TransportEquationSolver::MainThreadRoutine(int commSize)
{
	// Get mesh
	Mesh mesh = GetEmptyMesh(m_J, m_N);
	SetBoundaryConditions(mesh);

	int sendRank = 1;
	int recvRank = commSize - 1;

	// Start with first row (zero row is boundary -> initialized), then add commSize until J is reached
	for (std::size_t i = 1; i < m_J; i += commSize)
	{ 
		for (std::size_t j = 1; j < m_N; ++j)
		{
			// If we need extra data, receive it
			if (commSize > 1 && i != 1)
				PROCESS_ERROR(MPI_Recv(&mesh[i - 1][j], 1, MPI_DOUBLE, recvRank, CALCULATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE));

			// Calculate the main node
			mesh[i][j] = CalculateMainNode(mesh[i - 1][j - 1], mesh[i][j - 1], mesh[i - 1][j],
				m_x_0.first + m_h * (1 + 2 * (j - 1)) / 2, m_tau * (1 + 2 * (i - 1)) / 2);

			// Send it to the next one (first rank)
			// Here we don't need to wait for receive now or even afterwards, so put the same request
			if (commSize > 1)
				PROCESS_ERROR(MPI_Send(&mesh[i][j], 1, MPI_DOUBLE, sendRank, CALCULATE_TAG, MPI_COMM_WORLD));
		}
	}

	// Receive all other thread results
	if (commSize > 1)
		for (std::size_t i = 2; i < m_J; ++i)
			if ((i - 1) % commSize != 0 && (i % commSize != 0 || i == m_J - 1))
				PROCESS_ERROR(MPI_Recv(mesh[i].data(), m_N, MPI_DOUBLE, (i - 1) % commSize, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE));

	return mesh;
}

void TransportEquationSolver::OtherThreadRoutine(int myRank, int commSize)
{
	// Other thread need only several rows
	std::size_t m = (m_J - 1) % commSize;
	std::size_t rowCount = (m_J - 1) / commSize + ((myRank < m) ? 1 : 0);
	
	Mesh mesh = GetEmptyMesh(2 * rowCount, m_N);
	SetBoundaryConditions(mesh, myRank, commSize);

	int sendRank = (myRank + 1) % commSize;
	int recvRank = myRank - 1;

	// Start with (myRank + 1) row, then add commSize until J is reached
	for (std::size_t i = myRank + 1, meshRow = 0; i < m_J; i += commSize, meshRow += 2)
	{
		for (std::size_t j = 1; j < m_N; ++j)
		{
			// We need extra data to calculate, receive it
			PROCESS_ERROR(MPI_Recv(&mesh[meshRow][j], 1, MPI_DOUBLE, recvRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE));

			// Calculate the main node
			mesh[meshRow + 1][j] = CalculateMainNode(mesh[meshRow][j - 1], mesh[meshRow][j], mesh[meshRow + 1][j - 1],
				m_x_0.first + m_h * (1 + 2 * (j - 1)) / 2, m_tau * (1 + 2 * (i - 1)) / 2);

			// Send it to the next one
			if (i != m_J - 1)
				PROCESS_ERROR(MPI_Send(&mesh[meshRow + 1][j], 1, MPI_DOUBLE, sendRank, CALCULATE_TAG, MPI_COMM_WORLD));
		}
	}

	// Send results to the main thread
	if (myRank == commSize - 1)
	{
		if ((m_J - 1) % commSize == 0)
			PROCESS_ERROR(MPI_Send(mesh[2 * rowCount - 1].data(), m_N, MPI_DOUBLE, 0, FINAL_TAG, MPI_COMM_WORLD));
	}
	else
		for (std::size_t i = 1; i < 2 * rowCount; i += 2)
			PROCESS_ERROR(MPI_Send(mesh[i].data(), m_N, MPI_DOUBLE, 0, FINAL_TAG, MPI_COMM_WORLD));
}