#ifndef LAPLACE_JACOBI_SOLVER_HPP
#define LAPLACE_JACOBI_SOLVER_HPP

#include "Problem.hpp"
#include "RowDecomposition.hpp"

#include <mpi.h>

#include <vector>

namespace laplace
{

struct SolverResult
{
    int iterations = 0;
    bool converged = false;
    double increment_norm = 0.0;
    double exact_l2_error = 0.0;
    double elapsed_seconds = 0.0;
};

class JacobiSolver
{
  public:
    JacobiSolver(int n,
                 double tolerance,
                 int max_iterations,
                 ForcingKind forcing,
                 BoundaryKind boundary,
                 MPI_Comm communicator);

    SolverResult solve();
    void gather_solution(std::vector<double> &global_solution) const;

    const RowDecomposition &decomposition() const;
    int n() const;
    double h() const;

  private:
    int index(int local_row, int column) const;
    void initialize();
    void exchange_ghost_rows();
    double compute_exact_l2_error() const;

    int n_ = 0;
    double h_ = 0.0;
    double tolerance_ = 0.0;
    int max_iterations_ = 0;
    ForcingKind forcing_ = ForcingKind::Sine;
    BoundaryKind boundary_ = BoundaryKind::Homogeneous;
    MPI_Comm communicator_ = MPI_COMM_WORLD;
    RowDecomposition decomposition_;
    std::vector<double> current_;
    std::vector<double> next_;
};

} // namespace laplace

#endif
