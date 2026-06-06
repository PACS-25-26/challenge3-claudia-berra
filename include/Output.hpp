#ifndef LAPLACE_OUTPUT_HPP
#define LAPLACE_OUTPUT_HPP

#include "Problem.hpp"

#include <string>
#include <vector>

namespace laplace
{

/**
 * @brief Writes the solution to a legacy VTK structured-points file.
 *
 * Exports three scalar fields: numerical solution, exact solution, and
 * pointwise error. The output directory is created if it does not exist.
 *
 * @param filename Output file path.
 * @param n        Grid points per direction.
 * @param solution Row-major n×n solution vector (rank 0 only).
 * @param forcing  Forcing term, used to evaluate the exact solution.
 * @throws std::runtime_error if the file cannot be opened.
 */
void write_vtk(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing);

/**
 * @brief Writes the solution to a CSV file with columns x, y, solution, exact, error.
 *
 * The output directory is created if it does not exist.
 *
 * @param filename Output file path.
 * @param n        Grid points per direction.
 * @param solution Row-major n×n solution vector (rank 0 only).
 * @param forcing  Forcing term, used to evaluate the exact solution.
 * @throws std::runtime_error if the file cannot be opened.
 */
void write_csv(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing);

} // namespace laplace

#endif
