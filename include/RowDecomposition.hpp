#ifndef LAPLACE_ROW_DECOMPOSITION_HPP
#define LAPLACE_ROW_DECOMPOSITION_HPP

#include <algorithm>
#include <stdexcept>

namespace laplace
{

/**
 * @brief Describes how the n grid rows are split across MPI ranks.
 *
 * Rows are distributed as evenly as possible: the first (n % mpi_size)
 * ranks get one extra row.
 */
struct RowDecomposition
{
    int n = 0;          ///< Total grid points per direction.
    int mpi_size = 1;   ///< Number of MPI ranks.
    int mpi_rank = 0;   ///< Rank of this process.
    int first_row = 0;  ///< Global index of the first row owned by this rank.
    int local_rows = 0; ///< Number of rows owned by this rank.

    /**
     * @brief Global index of the last row owned by this rank.
     * @return first_row + local_rows - 1.
     */
    int last_row() const
    {
        return first_row + local_rows - 1;
    }
};

/**
 * @brief Builds a RowDecomposition for the given grid and MPI layout.
 * @param n        Grid size (must be >= 3).
 * @param mpi_size Number of ranks (must satisfy 1 <= mpi_size <= n).
 * @param mpi_rank Rank of the calling process (0-based).
 * @return Populated RowDecomposition for this rank.
 * @throws std::invalid_argument if the parameters are out of range.
 */
inline RowDecomposition make_row_decomposition(int n, int mpi_size, int mpi_rank)
{
    if (n < 3)
        throw std::invalid_argument("n must be at least 3.");
    if (mpi_size < 1)
        throw std::invalid_argument("The MPI communicator must contain at least one rank.");
    if (mpi_size > n)
        throw std::invalid_argument("The number of MPI ranks cannot exceed the number of grid rows.");

    const int base_rows  = n / mpi_size;
    const int extra_rows = n % mpi_size;

    RowDecomposition decomposition;
    decomposition.n          = n;
    decomposition.mpi_size   = mpi_size;
    decomposition.mpi_rank   = mpi_rank;
    decomposition.local_rows = base_rows + (mpi_rank < extra_rows ? 1 : 0);
    decomposition.first_row  = mpi_rank * base_rows + std::min(mpi_rank, extra_rows);

    return decomposition;
}

} // namespace laplace

#endif
