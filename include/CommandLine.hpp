#ifndef LAPLACE_COMMAND_LINE_HPP
#define LAPLACE_COMMAND_LINE_HPP

#include "Problem.hpp"

#include <iosfwd>
#include <string>

namespace laplace
{

struct ProgramOptions
{
    int n = 64;
    double tolerance = 1.0e-8;
    int max_iterations = 100000;
    ForcingKind forcing = ForcingKind::Sine;
    BoundaryKind boundary = BoundaryKind::Homogeneous;
    bool boundary_was_set = false;
    bool write_vtk = true;
    bool write_csv = false;
    bool quiet = false;
    bool help = false;
    std::string vtk_file = "output/solution.vtk";
    std::string csv_file = "output/solution.csv";
};

ProgramOptions parse_command_line(int argc, char **argv);
void print_usage(std::ostream &out, const char *program_name);

} // namespace laplace

#endif
