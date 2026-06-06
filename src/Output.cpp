/**
 * @file Output.cpp
 * @brief VTK and CSV output for the solver solution.
 */

#include "Output.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace laplace
{
namespace
{

/**
 * @brief Creates the parent directory of @p filename if it does not exist.
 * @param filename Target file path.
 */
void create_parent_directory(const std::string &filename)
{
    const std::filesystem::path path(filename);
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());
}

/**
 * @brief Checks that the solution vector has size n*n.
 * @param n        Grid size.
 * @param solution Solution vector to validate.
 * @throws std::runtime_error if the size is wrong.
 */
void check_solution_size(int n, const std::vector<double> &solution)
{
    const auto expected = static_cast<std::size_t>(n) * static_cast<std::size_t>(n);
    if (solution.size() != expected)
        throw std::runtime_error("The gathered solution has an unexpected size.");
}

} // namespace

void write_vtk(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing)
{
    check_solution_size(n, solution);
    create_parent_directory(filename);

    std::ofstream out(filename);
    if (!out)
        throw std::runtime_error("Cannot open VTK file '" + filename + "'.");

    const double h = 1.0 / static_cast<double>(n - 1);
    out << std::setprecision(16);
    out << "# vtk DataFile Version 3.0\n";
    out << "Laplace equation solved by hybrid Jacobi\n";
    out << "ASCII\n";
    out << "DATASET STRUCTURED_POINTS\n";
    out << "DIMENSIONS " << n << ' ' << n << " 1\n";
    out << "ORIGIN 0 0 0\n";
    out << "SPACING " << h << ' ' << h << " 1\n";
    out << "POINT_DATA " << static_cast<long long>(n) * n << "\n";

    out << "SCALARS solution double 1\nLOOKUP_TABLE default\n";
    for (const double value : solution)
        out << value << '\n';

    out << "SCALARS exact double 1\nLOOKUP_TABLE default\n";
    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int col = 0; col < n; ++col)
            out << exact_solution(forcing, static_cast<double>(col) * h, y) << '\n';
    }

    out << "SCALARS point_error double 1\nLOOKUP_TABLE default\n";
    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int col = 0; col < n; ++col)
        {
            const double x = static_cast<double>(col) * h;
            out << solution[static_cast<std::size_t>(row) * n + col] - exact_solution(forcing, x, y) << '\n';
        }
    }
}

void write_csv(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing)
{
    check_solution_size(n, solution);
    create_parent_directory(filename);

    std::ofstream out(filename);
    if (!out)
        throw std::runtime_error("Cannot open CSV file '" + filename + "'.");

    const double h = 1.0 / static_cast<double>(n - 1);
    out << std::setprecision(16) << "x,y,solution,exact,error\n";

    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int col = 0; col < n; ++col)
        {
            const double x = static_cast<double>(col) * h;
            const double val   = solution[static_cast<std::size_t>(row) * n + col];
            const double exact = exact_solution(forcing, x, y);
            out << x << ',' << y << ',' << val << ',' << exact << ',' << val - exact << '\n';
        }
    }
}

} // namespace laplace
