#ifndef LAPLACE_PROBLEM_HPP
#define LAPLACE_PROBLEM_HPP

#include <string>

namespace laplace
{

enum class ForcingKind
{
    Sine,
    Zero,
    Polynomial
};

enum class BoundaryKind
{
    Homogeneous,
    Exact
};

ForcingKind forcing_from_string(const std::string &name);
BoundaryKind boundary_from_string(const std::string &name);

std::string to_string(ForcingKind kind);
std::string to_string(BoundaryKind kind);

double forcing_value(ForcingKind kind, double x, double y);
double exact_solution(ForcingKind kind, double x, double y);
double boundary_value(BoundaryKind boundary, ForcingKind forcing, double x, double y);

} // namespace laplace

#endif
