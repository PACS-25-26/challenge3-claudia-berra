/**
 * @file CommandLine.cpp
 * @brief Command-line parsing for the Jacobi solver.
 *
 * Accepts both "--option value" and "--option=value" syntax.
 */

#include "CommandLine.hpp"

#include <iostream>
#include <stdexcept>

namespace laplace
{
namespace
{

/**
 * @brief Returns true if @p text starts with @p prefix.
 * @param text   String to check.
 * @param prefix Expected prefix.
 * @return True when @p text begins with @p prefix.
 */
bool starts_with(const std::string &text, const std::string &prefix)
{
    return text.rfind(prefix, 0) == 0;
}

/**
 * @brief Reads the next token from argv as the value for @p option.
 * @param index  Current position in argv (incremented on success).
 * @param argc   Total argument count.
 * @param argv   Argument array.
 * @param option Option name, used in the error message.
 * @return The next argument as a string.
 * @throws std::invalid_argument if there is no next argument.
 */
std::string read_value(int &index, int argc, char **argv, const std::string &option)
{
    if (index + 1 >= argc)
        throw std::invalid_argument("Missing value after " + option + ".");
    return argv[++index];
}

/**
 * @brief Tries to match "--option=value" syntax.
 * @param argument Full argument string.
 * @param option   Option name to match (without '=').
 * @param value    Set to the extracted value if matched.
 * @return True if the argument matched and @p value was set.
 */
bool option_value(const std::string &argument,
                  const std::string &option,
                  std::string &value)
{
    const std::string prefix = option + "=";
    if (starts_with(argument, prefix))
    {
        value = argument.substr(prefix.size());
        return true;
    }
    return false;
}

/**
 * @brief Parses an integer, rejecting trailing garbage.
 * @param text   String to parse.
 * @param option Option name, used in the error message.
 * @return Parsed integer value.
 * @throws std::invalid_argument on parse failure.
 */
int parse_int(const std::string &text, const std::string &option)
{
    std::size_t pos = 0;
    const int value = std::stoi(text, &pos);
    if (pos != text.size())
        throw std::invalid_argument("Invalid integer for " + option + ": '" + text + "'.");
    return value;
}

/**
 * @brief Parses a double, rejecting trailing garbage.
 * @param text   String to parse.
 * @param option Option name, used in the error message.
 * @return Parsed double value.
 * @throws std::invalid_argument on parse failure.
 */
double parse_double(const std::string &text, const std::string &option)
{
    std::size_t pos = 0;
    const double value = std::stod(text, &pos);
    if (pos != text.size())
        throw std::invalid_argument("Invalid floating-point value for " + option + ": '" + text + "'.");
    return value;
}

} // namespace

ProgramOptions parse_command_line(int argc, char **argv)
{
    ProgramOptions options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];
        std::string value;

        if (argument == "--help" || argument == "-h")
            options.help = true;
        else if (argument == "--n")
            options.n = parse_int(read_value(i, argc, argv, argument), argument);
        else if (option_value(argument, "--n", value))
            options.n = parse_int(value, "--n");
        else if (argument == "--tol")
            options.tolerance = parse_double(read_value(i, argc, argv, argument), argument);
        else if (option_value(argument, "--tol", value))
            options.tolerance = parse_double(value, "--tol");
        else if (argument == "--max-it")
            options.max_iterations = parse_int(read_value(i, argc, argv, argument), argument);
        else if (option_value(argument, "--max-it", value))
            options.max_iterations = parse_int(value, "--max-it");
        else if (argument == "--forcing")
            options.forcing = forcing_from_string(read_value(i, argc, argv, argument));
        else if (option_value(argument, "--forcing", value))
            options.forcing = forcing_from_string(value);
        else if (argument == "--boundary")
        {
            options.boundary = boundary_from_string(read_value(i, argc, argv, argument));
            options.boundary_was_set = true;
        }
        else if (option_value(argument, "--boundary", value))
        {
            options.boundary = boundary_from_string(value);
            options.boundary_was_set = true;
        }
        else if (argument == "--vtk")
        {
            options.vtk_file = read_value(i, argc, argv, argument);
            options.write_vtk = true;
        }
        else if (option_value(argument, "--vtk", value))
        {
            options.vtk_file = value;
            options.write_vtk = true;
        }
        else if (argument == "--csv")
        {
            options.csv_file = read_value(i, argc, argv, argument);
            options.write_csv = true;
        }
        else if (option_value(argument, "--csv", value))
        {
            options.csv_file = value;
            options.write_csv = true;
        }
        else if (argument == "--no-vtk")
            options.write_vtk = false;
        else if (argument == "--no-output")
        {
            options.write_vtk = false;
            options.write_csv = false;
        }
        else if (argument == "--quiet")
            options.quiet = true;
        else
            throw std::invalid_argument("Unknown option '" + argument + "'.");
    }

    if (options.help)
        return options;

    if (options.n < 3)
        throw std::invalid_argument("n must be at least 3.");
    if (options.tolerance < 0.0)
        throw std::invalid_argument("The tolerance cannot be negative.");
    if (options.max_iterations < 1)
        throw std::invalid_argument("The maximum number of iterations must be positive.");
    if (!options.boundary_was_set && options.forcing == ForcingKind::Polynomial)
        options.boundary = BoundaryKind::Exact;

    return options;
}

void print_usage(std::ostream &out, const char *program_name)
{
    out << "Usage: mpiexec -n <tasks> " << program_name << " [options]\n\n"
        << "Options:\n"
        << "  --n <int>              Number of grid points in each direction (default: 64).\n"
        << "  --tol <real>           Global Jacobi increment tolerance (default: 1e-8).\n"
        << "  --max-it <int>         Maximum number of Jacobi iterations (default: 100000).\n"
        << "  --forcing <name>       Forcing term: sine, zero, poly (default: sine).\n"
        << "  --boundary <name>      Dirichlet data: homogeneous, exact (default: homogeneous).\n"
        << "  --vtk <file>           Write a legacy VTK file (default: output/solution.vtk).\n"
        << "  --csv <file>           Also write x,y,solution,exact,error as CSV.\n"
        << "  --no-vtk               Disable VTK output.\n"
        << "  --no-output            Disable VTK and CSV output.\n"
        << "  --quiet                Print one CSV result line only.\n"
        << "  --help                 Show this help message.\n";
}

} // namespace laplace
