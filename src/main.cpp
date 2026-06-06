/**
 * @file main.cpp
 * @brief Entry point: initialises MPI, parses options, runs the solver,
 *        gathers the solution on rank 0 and writes output files.
 */

#include "CommandLine.hpp"
#include "JacobiSolver.hpp"
#include "Output.hpp"

#include <mpi.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

/**
 * @brief Program entry point.
 * @param argc Argument count received from the command line.
 * @param argv Argument values received from the command line.
 * @return Zero on success, non-zero if parsing, solving, or output fails.
 */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int mpi_rank = 0;
    int mpi_size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    int return_code = 0;

    try
    {
        const laplace::ProgramOptions options = laplace::parse_command_line(argc, argv);

        if (options.help)
        {
            if (mpi_rank == 0)
                laplace::print_usage(std::cout, argv[0]);
            MPI_Finalize();
            return 0;
        }

        if (mpi_size > options.n)
            throw std::invalid_argument("Use no more MPI ranks than grid rows.");

        laplace::JacobiSolver solver(options.n,
                                     options.tolerance,
                                     options.max_iterations,
                                     options.forcing,
                                     options.boundary,
                                     MPI_COMM_WORLD);

        const laplace::SolverResult result = solver.solve();

        std::vector<double> global_solution;
        solver.gather_solution(global_solution);

        if (mpi_rank == 0)
        {
            if (options.write_vtk)
                laplace::write_vtk(options.vtk_file, options.n, global_solution, options.forcing);
            if (options.write_csv)
                laplace::write_csv(options.csv_file, options.n, global_solution, options.forcing);

#ifdef _OPENMP
            const int openmp_threads = omp_get_max_threads();
#else
            const int openmp_threads = 1;
#endif

            std::cout << std::setprecision(10);
            if (options.quiet)
            {
                std::cout << options.n << ','
                          << mpi_size << ','
                          << openmp_threads << ','
                          << result.iterations << ','
                          << (result.converged ? 1 : 0) << ','
                          << result.increment_norm << ','
                          << result.exact_l2_error << ','
                          << result.elapsed_seconds << '\n';
            }
            else
            {
                std::cout << "Hybrid Jacobi solver for the Laplace equation\n"
                          << "n                  : " << options.n << '\n'
                          << "h                  : " << solver.h() << '\n'
                          << "MPI ranks          : " << mpi_size << '\n'
                          << "OpenMP max threads : " << openmp_threads << '\n'
                          << "forcing            : " << laplace::to_string(options.forcing) << '\n'
                          << "boundary           : " << laplace::to_string(options.boundary) << '\n'
                          << "iterations         : " << result.iterations << '\n'
                          << "converged          : " << (result.converged ? "yes" : "no") << '\n'
                          << "increment norm     : " << result.increment_norm << '\n'
                          << "exact L2 error     : " << result.exact_l2_error << '\n'
                          << "elapsed seconds    : " << result.elapsed_seconds << '\n';

                if (options.write_vtk)
                    std::cout << "VTK output         : " << options.vtk_file << '\n';
                if (options.write_csv)
                    std::cout << "CSV output         : " << options.csv_file << '\n';
            }
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Rank " << mpi_rank << " error: " << error.what() << '\n';
        return_code = 1;
    }

    MPI_Finalize();
    return return_code;
}
