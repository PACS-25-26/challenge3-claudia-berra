#ifndef LAPLACE_COMMAND_LINE_HPP
#define LAPLACE_COMMAND_LINE_HPP

#include "Problem.hpp"

#include <iosfwd>
#include <string>

namespace laplace
{

/**
 * @brief All runtime parameters read from the command line.
 */
struct ProgramOptions
{
    int n = 64;                                        ///< Grid points per direction.
    double tolerance = 1.0e-8;                        ///< Global Jacobi stopping tolerance.
    int max_iterations = 100000;                      ///< Maximum iteration count.
    ForcingKind forcing = ForcingKind::Sine;          ///< Forcing term.
    BoundaryKind boundary = BoundaryKind::Homogeneous;///< Boundary condition type.
    bool boundary_was_set = false;                    ///< True if --boundary was given explicitly.
    bool write_vtk = true;                            ///< Write VTK output.
    bool write_csv = false;                           ///< Write CSV output.
    bool quiet = false;                               ///< Print a single CSV line instead of full report.
    bool help = false;                                ///< Print usage and exit.
    std::string vtk_file = "output/solution.vtk";    ///< VTK output path.
    std::string csv_file = "output/solution.csv";    ///< CSV output path.
};

/**
 * @brief Parses argv into a ProgramOptions struct.
 *
 * Accepts both "--n 64" and "--n=64" syntax.
 *
 * @param argc Argument count from main.
 * @param argv Argument array from main.
 * @return Populated ProgramOptions.
 * @throws std::invalid_argument on unknown options or invalid values.
 */
ProgramOptions parse_command_line(int argc, char **argv);

/**
 * @brief Prints usage information.
 * @param out          Output stream (e.g. std::cout).
 * @param program_name Name of the executable (argv[0]).
 */
void print_usage(std::ostream &out, const char *program_name);

} // namespace laplace

#endif
