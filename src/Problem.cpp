/**
 * @file Problem.cpp
 * @brief Forcing terms, exact solutions, and boundary values.
 */

#include "Problem.hpp"

#include <cmath>
#include <stdexcept>

namespace laplace
{
namespace
{

constexpr double pi = 3.141592653589793238462643383279502884; ///< π.

} // namespace

ForcingKind forcing_from_string(const std::string &name)
{
    if (name == "sine")       return ForcingKind::Sine;
    if (name == "zero")       return ForcingKind::Zero;
    if (name == "poly" || name == "polynomial") return ForcingKind::Polynomial;
    throw std::invalid_argument("Unknown forcing term '" + name + "'.");
}

BoundaryKind boundary_from_string(const std::string &name)
{
    if (name == "homogeneous" || name == "zero") return BoundaryKind::Homogeneous;
    if (name == "exact")                         return BoundaryKind::Exact;
    throw std::invalid_argument("Unknown boundary kind '" + name + "'.");
}

std::string to_string(ForcingKind kind)
{
    switch (kind)
    {
    case ForcingKind::Sine:       return "sine";
    case ForcingKind::Zero:       return "zero";
    case ForcingKind::Polynomial: return "poly";
    }
    return "unknown";
}

std::string to_string(BoundaryKind kind)
{
    switch (kind)
    {
    case BoundaryKind::Homogeneous: return "homogeneous";
    case BoundaryKind::Exact:       return "exact";
    }
    return "unknown";
}

double forcing_value(ForcingKind kind, double x, double y)
{
    switch (kind)
    {
    case ForcingKind::Sine:
        return 8.0 * pi * pi * std::sin(2.0 * pi * x) * std::sin(2.0 * pi * y);
    case ForcingKind::Zero:
        return 0.0;
    case ForcingKind::Polynomial:
        return -4.0;
    }
    return 0.0;
}

double exact_solution(ForcingKind kind, double x, double y)
{
    switch (kind)
    {
    case ForcingKind::Sine:
        return std::sin(2.0 * pi * x) * std::sin(2.0 * pi * y);
    case ForcingKind::Zero:
        return 0.0;
    case ForcingKind::Polynomial:
        return x * x + y * y;
    }
    return 0.0;
}

double boundary_value(BoundaryKind boundary, ForcingKind forcing, double x, double y)
{
    switch (boundary)
    {
    case BoundaryKind::Homogeneous: return 0.0;
    case BoundaryKind::Exact:       return exact_solution(forcing, x, y);
    }
    return 0.0;
}

} // namespace laplace
