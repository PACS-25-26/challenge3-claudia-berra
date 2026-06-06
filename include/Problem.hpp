#ifndef LAPLACE_PROBLEM_HPP
#define LAPLACE_PROBLEM_HPP

#include <string>

namespace laplace
{

/**
 * @brief Forcing term for the right-hand side of the Laplace equation.
 *
 * Sine:       f = 8π²sin(2πx)sin(2πy), exact solution u = sin(2πx)sin(2πy)
 * Zero:       f = 0
 * Polynomial: f = -4, exact solution u = x²+y²
 */
enum class ForcingKind
{
    Sine,
    Zero,
    Polynomial
};

/**
 * @brief Type of Dirichlet boundary condition.
 *
 * Homogeneous: u = 0 on the boundary.
 * Exact:       u = exact solution on the boundary (for non-homogeneous tests).
 */
enum class BoundaryKind
{
    Homogeneous,
    Exact
};

/**
 * @brief Parses a ForcingKind from string ("sine", "zero", "poly"/"polynomial").
 * @param name The string to parse.
 * @return The corresponding ForcingKind.
 * @throws std::invalid_argument if the name is unknown.
 */
ForcingKind forcing_from_string(const std::string &name);

/**
 * @brief Parses a BoundaryKind from string ("homogeneous"/"zero", "exact").
 * @param name The string to parse.
 * @return The corresponding BoundaryKind.
 * @throws std::invalid_argument if the name is unknown.
 */
BoundaryKind boundary_from_string(const std::string &name);

/**
 * @brief String representation of a ForcingKind value.
 * @param kind The forcing term.
 * @return Human-readable name.
 */
std::string to_string(ForcingKind kind);

/**
 * @brief String representation of a BoundaryKind value.
 * @param kind The boundary condition type.
 * @return Human-readable name.
 */
std::string to_string(BoundaryKind kind);

/**
 * @brief Evaluates f(x,y) at the given grid point.
 * @param kind Which forcing term to use.
 * @param x x-coordinate in [0,1].
 * @param y y-coordinate in [0,1].
 * @return Value of f(x,y).
 */
double forcing_value(ForcingKind kind, double x, double y);

/**
 * @brief Evaluates the analytical solution u(x,y).
 * @param kind Forcing term (determines which exact solution to use).
 * @param x x-coordinate in [0,1].
 * @param y y-coordinate in [0,1].
 * @return Exact value u(x,y).
 */
double exact_solution(ForcingKind kind, double x, double y);

/**
 * @brief Evaluates the Dirichlet boundary datum at (x,y).
 * @param boundary Boundary condition type.
 * @param forcing  Forcing term (needed for the Exact case).
 * @param x x-coordinate.
 * @param y y-coordinate.
 * @return Boundary value.
 */
double boundary_value(BoundaryKind boundary, ForcingKind forcing, double x, double y);

} // namespace laplace

#endif
