#ifndef _HEADER_TRANSPORT_EQUATION_SOLVER_
#define _HEADER_TRANSPORT_EQUATION_SOLVER_

#include <functional>
#include <utility>
#include <vector>

namespace se
{
	// Math function template
	using MathFunctionScalar = const std::function<double(double)>;
	using MathFunctionVector = const std::function<double(double, double)>;
	using Mesh = std::vector<std::vector<double>>;

	// Solves transport equations with this structure:
	// u_t + a * u_x = f(x, t)
	// x_1 < x < x_2, 0 < t < T
	// u(x, 0) = phi(x)
	// u(0, t) = mu(t)
	// a = const
	class TransportEquationSolver
	{
		// Equation functions
		MathFunctionVector m_a; // a
		MathFunctionVector m_f; // f(x, t)
		// Initial conditions
		std::pair<double, double> m_x_0; // {x_1, x_2}
		double m_T; // T
		
		MathFunctionScalar m_phi; // phi(x)
		MathFunctionScalar m_mu; // mu(t)

		// Mesh
		const std::size_t m_N;
		const std::size_t m_J;
		const double m_h;
		const double m_tau;
		// Get empty mesh with proper size
		Mesh GetEmptyMesh(std::size_t J, std::size_t N);
		// Set boundary conditions onto the mesh
		void SetBoundaryConditions(Mesh& mesh, int myRank = 0, int commSize = 0);

		// Routines for each thread
		Mesh MainThreadRoutine(int commSize);
		void OtherThreadRoutine(int myRank, int commSize);

		// Tags for communicating
		const int CALCULATE_TAG = 0x1;
		const int FINAL_TAG = 0x2;

		// Calculate y_11
		//
		// y_01 -- y_11
		//	|		|
		// y_00 -- y_01
		double CalculateMainNode(double y_00, double y_01, double y_11, double x, double t);

		// Debug print of the mesh
		void PrintMesh(Mesh& mesh, std::ostream& stream = std::cout);
	public:
		TransportEquationSolver(MathFunctionVector& a, MathFunctionVector& f, MathFunctionScalar phi,
			MathFunctionScalar mu, const std::size_t N, const std::size_t J, double x_1 = 0, double x_2 = 1, double T = 1) :
			m_a(a), m_f(f), m_phi (phi), m_mu (mu), m_N (N), m_J (J), m_x_0({ x_1, x_2 }), m_T(T),
			m_h ((x_2 - x_1) / (N - 1)), m_tau (T / (J - 1)) {}
		
		void Solve(const char* fileName);
	};
}

#endif // _HEADER_TRANSPORT_EQUATION_SOLVER_