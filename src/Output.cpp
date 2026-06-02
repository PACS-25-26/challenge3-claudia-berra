#include "Output.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace laplace
{
namespace
{

void create_parent_directory(const std::string &filename)
{
    const std::filesystem::path path(filename);
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
}

void check_solution_size(int n, const std::vector<double> &solution)
{
    const auto expected_size = static_cast<std::size_t>(n) * static_cast<std::size_t>(n);
    if (solution.size() != expected_size)
    {
        throw std::runtime_error("The gathered solution has an unexpected size.");
    }
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
    {
        throw std::runtime_error("Cannot open VTK file '" + filename + "'.");
    }

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

    out << "SCALARS solution double 1\n";
    out << "LOOKUP_TABLE default\n";
    for (const double value : solution)
    {
        out << value << '\n';
    }

    out << "SCALARS exact double 1\n";
    out << "LOOKUP_TABLE default\n";
    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int column = 0; column < n; ++column)
        {
            const double x = static_cast<double>(column) * h;
            out << exact_solution(forcing, x, y) << '\n';
        }
    }

    out << "SCALARS point_error double 1\n";
    out << "LOOKUP_TABLE default\n";
    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int column = 0; column < n; ++column)
        {
            const double x = static_cast<double>(column) * h;
            const double error = solution[static_cast<std::size_t>(row) * n + column] -
                                 exact_solution(forcing, x, y);
            out << error << '\n';
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
    {
        throw std::runtime_error("Cannot open CSV file '" + filename + "'.");
    }

    const double h = 1.0 / static_cast<double>(n - 1);
    out << std::setprecision(16);
    out << "x,y,solution,exact,error\n";

    for (int row = 0; row < n; ++row)
    {
        const double y = static_cast<double>(row) * h;
        for (int column = 0; column < n; ++column)
        {
            const double x = static_cast<double>(column) * h;
            const double value = solution[static_cast<std::size_t>(row) * n + column];
            const double exact = exact_solution(forcing, x, y);
            out << x << ',' << y << ',' << value << ',' << exact << ',' << value - exact << '\n';
        }
    }
}

} // namespace laplace
