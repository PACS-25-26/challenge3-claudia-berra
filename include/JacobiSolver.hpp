#ifndef LAPLACE_JACOBI_SOLVER_HPP
#define LAPLACE_JACOBI_SOLVER_HPP

#include "Problem.hpp"
#include "RowDecomposition.hpp"

#include <mpi.h>

#include <vector>

namespace laplace
{

/**
 * @brief Output of a completed solver run.
 */
struct SolverResult
{
    int iterations = 0;           ///< Number of iterations performed.
    bool converged = false;       ///< True if the stopping criterion was met.
    double increment_norm = 0.0;  ///< Final global increment norm (h-weighted L2).
    double exact_l2_error = 0.0;  ///< L2 error vs. the analytical solution.
    double elapsed_seconds = 0.0; ///< Wall-clock time inside solve().
};

/**
 * @brief Hybrid MPI+OpenMP Jacobi solver for the 2D Laplace equation on [0,1]².
 *
 * Rows of the n×n grid are split evenly across MPI ranks. At each iteration,
 * ghost rows are exchanged with neighbours via MPI_Sendrecv, and the stencil
 * update is parallelised with an OpenMP reduction over the local block.
 */
class JacobiSolver
{
  public:
    /**
     * @brief Constructs the solver and sets up the local grid partition.
     * @param n             Grid points per direction (>= 3).
     * @param tolerance     Stopping threshold for the global increment norm.
     * @param max_iterations Maximum number of Jacobi iterations.
     * @param forcing       Right-hand side forcing term.
     * @param boundary      Dirichlet boundary condition type.
     * @param communicator  MPI communicator used for domain decomposition.
     */
    JacobiSolver(int n,
                 double tolerance,
                 int max_iterations,
                 ForcingKind forcing,
                 BoundaryKind boundary,
                 MPI_Comm communicator);

    /**
     * @brief Runs the Jacobi iterations.
     * @return SolverResult with convergence info and timing.
     */
    SolverResult solve();

    /**
     * @brief Collects the distributed solution on rank 0 via MPI_Gatherv.
     *
     * On rank 0, @p global_solution is filled with the full n×n solution
     * in row-major order. Other ranks leave it unchanged.
     *
     * @param global_solution Output vector (populated only on rank 0).
     */
    void gather_solution(std::vector<double> &global_solution) const;

    /**
     * @brief Row decomposition used by this solver instance.
     * @return Const reference to the internal RowDecomposition.
     */
    const RowDecomposition &decomposition() const;

    /**
     * @brief Grid size n.
     * @return Number of grid points per direction.
     */
    int n() const;

    /**
     * @brief Grid spacing h = 1/(n-1).
     * @return Distance between adjacent grid points.
     */
    double h() const;

  private:
    /**
     * @brief Flat index into current_ / next_.
     * @param local_row 1-based local row (0 and local_rows+1 are ghost rows).
     * @param column    0-based column index.
     * @return Flat row-major vector index.
     */
    int index(int local_row, int column) const;

    /** @brief Initialises boundary nodes; sets interior to zero. */
    void initialize();

    /** @brief Exchanges the top and bottom ghost rows with neighbouring ranks. */
    void exchange_ghost_rows();

    /**
     * @brief Computes the global h-weighted L2 error against the exact solution.
     * @return Global L2 error norm.
     */
    double compute_exact_l2_error() const;

    int n_ = 0;                                        ///< Grid size.
    double h_ = 0.0;                                   ///< Grid spacing.
    double tolerance_ = 0.0;                           ///< Convergence tolerance.
    int max_iterations_ = 0;                           ///< Max iteration count.
    ForcingKind forcing_ = ForcingKind::Sine;          ///< Forcing term.
    BoundaryKind boundary_ = BoundaryKind::Homogeneous;///< Boundary condition.
    MPI_Comm communicator_ = MPI_COMM_WORLD;           ///< MPI communicator.
    RowDecomposition decomposition_;                   ///< Row split for this rank.
    std::vector<double> current_;                      ///< Solution at iteration k (includes ghost rows).
    std::vector<double> next_;                         ///< Solution at iteration k+1.
};

} // namespace laplace

#endif
