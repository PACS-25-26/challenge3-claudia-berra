#ifndef LAPLACE_OUTPUT_HPP
#define LAPLACE_OUTPUT_HPP

#include "Problem.hpp"

#include <string>
#include <vector>

namespace laplace
{

void write_vtk(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing);

void write_csv(const std::string &filename,
               int n,
               const std::vector<double> &solution,
               ForcingKind forcing);

} // namespace laplace

#endif
