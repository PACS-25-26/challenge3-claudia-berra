/**
 * @file JacobiSolver.cpp
 * @brief Jacobi solver implementation (hybrid MPI+OpenMP).
 */

#include "JacobiSolver.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace laplace
{

JacobiSolver::JacobiSolver(int n,
                           double tolerance,
                           int max_iterations,
                           ForcingKind forcing,
                           BoundaryKind boundary,
                           MPI_Comm communicator)
    : n_(n),
      h_(1.0 / static_cast<double>(n - 1)),
      tolerance_(tolerance),
      max_iterations_(max_iterations),
      forcing_(forcing),
      boundary_(boundary),
      communicator_(communicator)
{
    int mpi_size = 1;
    int mpi_rank = 0;
    MPI_Comm_size(communicator_, &mpi_size);
    MPI_Comm_rank(communicator_, &mpi_rank);

    decomposition_ = make_row_decomposition(n_, mpi_size, mpi_rank);

    // One ghost row above and one below the local block.
    current_.assign(static_cast<std::size_t>(decomposition_.local_rows + 2) * n_, 0.0);
    next_.assign(current_.size(), 0.0);

    initialize();
}

SolverResult JacobiSolver::solve()
{
    SolverResult result;

    MPI_Barrier(communicator_);
    const auto start_time = std::chrono::steady_clock::now();

    for (int iteration = 1; iteration <= max_iterations_; ++iteration)
    {
        exchange_ghost_rows();

        double local_squared_update = 0.0;
        const double *current = current_.data();
        double *next = next_.data();
        const int n = n_;
        const double h = h_;
        const int first_row = decomposition_.first_row;
        const int local_rows = decomposition_.local_rows;
        const ForcingKind forcing = forcing_;

#pragma omp parallel for default(none) shared(current, next) firstprivate(n, h, first_row, local_rows, forcing) reduction(+ : local_squared_update)
        for (int local_row = 1; local_row <= local_rows; ++local_row)
        {
            const int global_row = first_row + local_row - 1;
            if (global_row == 0 || global_row == n - 1)
                continue;

            const double y = static_cast<double>(global_row) * h;
            for (int column = 1; column < n - 1; ++column)
            {
                const double x = static_cast<double>(column) * h;
                const int center = local_row * n + column;
                const double updated = 0.25 *
                                       (current[center - n] + current[center + n] +
                                        current[center - 1] + current[center + 1] +
                                        h * h * forcing_value(forcing, x, y));
                const double difference = updated - current[center];
                next[center] = updated;
                local_squared_update += difference * difference;
            }
        }

        double global_squared_update = 0.0;
        MPI_Allreduce(&local_squared_update, &global_squared_update,
                      1, MPI_DOUBLE, MPI_SUM, communicator_);

        const double global_increment_norm = std::sqrt(h_ * global_squared_update);

        current_.swap(next_);

        result.iterations = iteration;
        result.increment_norm = global_increment_norm;

        if (global_increment_norm < tolerance_)
        {
            result.converged = true;
            break;
        }
    }

    MPI_Barrier(communicator_);
    const auto end_time = std::chrono::steady_clock::now();
    result.elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
    result.exact_l2_error = compute_exact_l2_error();

    return result;
}

void JacobiSolver::gather_solution(std::vector<double> &global_solution) const
{
    std::vector<int> counts;
    std::vector<int> displacements;

    if (decomposition_.mpi_rank == 0)
    {
        global_solution.assign(static_cast<std::size_t>(n_) * n_, 0.0);
        counts.resize(decomposition_.mpi_size);
        displacements.resize(decomposition_.mpi_size);

        for (int rank = 0; rank < decomposition_.mpi_size; ++rank)
        {
            const RowDecomposition part = make_row_decomposition(n_, decomposition_.mpi_size, rank);
            counts[rank] = part.local_rows * n_;
            displacements[rank] = part.first_row * n_;
        }
    }

    // Skip the leading ghost row when sending.
    MPI_Gatherv(current_.data() + n_,
                decomposition_.local_rows * n_,
                MPI_DOUBLE,
                global_solution.data(),
                counts.data(),
                displacements.data(),
                MPI_DOUBLE,
                0,
                communicator_);
}

const RowDecomposition &JacobiSolver::decomposition() const { return decomposition_; }
int JacobiSolver::n() const { return n_; }
double JacobiSolver::h() const { return h_; }

int JacobiSolver::index(int local_row, int column) const
{
    return local_row * n_ + column;
}

void JacobiSolver::initialize()
{
    for (int local_row = 1; local_row <= decomposition_.local_rows; ++local_row)
    {
        const int global_row = decomposition_.first_row + local_row - 1;
        const double y = static_cast<double>(global_row) * h_;

        for (int column = 0; column < n_; ++column)
        {
            const double x = static_cast<double>(column) * h_;
            const bool is_boundary = global_row == 0 || global_row == n_ - 1 ||
                                     column == 0 || column == n_ - 1;
            const double value = is_boundary ? boundary_value(boundary_, forcing_, x, y) : 0.0;
            current_[index(local_row, column)] = value;
            next_[index(local_row, column)] = value;
        }
    }
}

void JacobiSolver::exchange_ghost_rows()
{
    // MPI_PROC_NULL turns sends/receives at the domain boundary into no-ops.
    const int upper_rank = decomposition_.mpi_rank == 0
                               ? MPI_PROC_NULL : decomposition_.mpi_rank - 1;
    const int lower_rank = decomposition_.mpi_rank == decomposition_.mpi_size - 1
                               ? MPI_PROC_NULL : decomposition_.mpi_rank + 1;

    MPI_Sendrecv(&current_[index(1, 0)], n_, MPI_DOUBLE, upper_rank, 10,
                 &current_[index(decomposition_.local_rows + 1, 0)], n_, MPI_DOUBLE, lower_rank, 10,
                 communicator_, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&current_[index(decomposition_.local_rows, 0)], n_, MPI_DOUBLE, lower_rank, 20,
                 &current_[index(0, 0)], n_, MPI_DOUBLE, upper_rank, 20,
                 communicator_, MPI_STATUS_IGNORE);
}

double JacobiSolver::compute_exact_l2_error() const
{
    double local_squared_error = 0.0;
    const double *current = current_.data();
    const int n = n_;
    const double h = h_;
    const int first_row = decomposition_.first_row;
    const int local_rows = decomposition_.local_rows;
    const ForcingKind forcing = forcing_;

#pragma omp parallel for default(none) shared(current) firstprivate(n, h, first_row, local_rows, forcing) reduction(+ : local_squared_error)
    for (int local_row = 1; local_row <= local_rows; ++local_row)
    {
        const int global_row = first_row + local_row - 1;
        const double y = static_cast<double>(global_row) * h;
        for (int column = 0; column < n; ++column)
        {
            const double x = static_cast<double>(column) * h;
            const double diff = current[local_row * n + column] - exact_solution(forcing, x, y);
            local_squared_error += diff * diff;
        }
    }

    double global_squared_error = 0.0;
    MPI_Allreduce(&local_squared_error, &global_squared_error,
                  1, MPI_DOUBLE, MPI_SUM, communicator_);

    return std::sqrt(h_ * global_squared_error);
}

} // namespace laplace
