#ifndef LAPLACE_ROW_DECOMPOSITION_HPP
#define LAPLACE_ROW_DECOMPOSITION_HPP

#include <algorithm>
#include <stdexcept>

namespace laplace
{

struct RowDecomposition
{
    int n = 0;
    int mpi_size = 1;
    int mpi_rank = 0;
    int first_row = 0;
    int local_rows = 0;

    int last_row() const
    {
        return first_row + local_rows - 1;
    }
};

inline RowDecomposition make_row_decomposition(int n, int mpi_size, int mpi_rank)
{
    if (n < 3)
    {
        throw std::invalid_argument("n must be at least 3.");
    }
    if (mpi_size < 1)
    {
        throw std::invalid_argument("The MPI communicator must contain at least one rank.");
    }
    if (mpi_size > n)
    {
        throw std::invalid_argument("The number of MPI ranks cannot exceed the number of grid rows.");
    }

    const int base_rows = n / mpi_size;
    const int extra_rows = n % mpi_size;

    RowDecomposition decomposition;
    decomposition.n = n;
    decomposition.mpi_size = mpi_size;
    decomposition.mpi_rank = mpi_rank;
    decomposition.local_rows = base_rows + (mpi_rank < extra_rows ? 1 : 0);
    decomposition.first_row = mpi_rank * base_rows + std::min(mpi_rank, extra_rows);

    return decomposition;
}

} // namespace laplace

#endif
