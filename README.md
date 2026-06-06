# Matrix-Free Hybrid Jacobi Solver

This repository contains my solution for Challenge 3 of the PACS course: a
matrix-free Jacobi solver for

```text
-Delta u = f  in (0,1)^2,
u = g         on the boundary.
```

The default test problem is the one from the assignment:

```text
f(x,y) = 8*pi^2 sin(2*pi*x) sin(2*pi*y),
u(x,y) = sin(2*pi*x) sin(2*pi*y).
```

I kept the implementation close to the tools used during the course: standard
C++, MPI, OpenMP, a direct `Makefile`, `<chrono>`, and `<filesystem>`. The MPI
calls used in the code are `MPI_Init`, `MPI_Comm_rank`, `MPI_Comm_size`,
`MPI_Sendrecv`, `MPI_Allreduce`, `MPI_Gatherv`, and `MPI_Barrier`.

## Repository Layout

```text
include/            Public headers
src/                Source files
test/               Scalability script and discussion
Makefile            Build rules
README.md           This file
```

The code does not assemble the matrix of the discrete Laplacian. Each MPI rank
owns a block of consecutive rows and two ghost rows, one above and one below the
local block.

## Build

The expected environment is Linux with an MPI C++ compiler wrapper and OpenMP
support:

```bash
make
```

By default the `Makefile` uses `mpic++` and `-fopenmp`. These can still be
overridden from the command line, for example:

```bash
make CXX=mpic++ CXXFLAGS="-std=c++20 -O3 -Wall -Wextra -pedantic -fopenmp"
```

## Run

The number of MPI ranks is chosen with `mpiexec -n`:

```bash
mpiexec -n 4 ./laplace_jacobi --n 128 --tol 1e-8 --max-it 200000
```

The number of OpenMP threads is set with `OMP_NUM_THREADS`:

```bash
OMP_NUM_THREADS=2 mpiexec -n 4 ./laplace_jacobi --n 128
```

Useful options:

```text
--n <int>              Grid points per direction.
--tol <real>           Global convergence tolerance.
--max-it <int>         Maximum Jacobi iterations.
--forcing <name>       sine, zero, poly.
--boundary <name>      homogeneous, exact.
--vtk <file>           VTK output path.
--csv <file>           CSV output path.
--no-output            Disable VTK and CSV output.
--quiet                Print one CSV result line only.
```

The `poly` forcing is an extra check with non-homogeneous Dirichlet data:
`u(x,y)=x^2+y^2`, `f=-4`, and `--boundary exact`.

## Numerical Method

Rows are distributed as evenly as possible among the MPI ranks. At every Jacobi
iteration:

1. each rank exchanges its first and last owned rows with the adjacent ranks;
2. the interior points are updated with the five-point finite-difference stencil;
3. the local squared update is accumulated with an OpenMP reduction;
4. `MPI_Allreduce` sums the local contributions, and the solver stops when the
   global h-weighted increment norm is below `--tol`.

The increment norm and the exact error are reported with the discrete norm used
in the assignment:

```text
sqrt(h * sum_ij value_ij^2).
```

## Output

Rank 0 gathers the full solution with `MPI_Gatherv`.

By default the code writes:

```text
output/solution.vtk
```

The VTK file is a legacy ASCII `STRUCTURED_POINTS` file and can be opened in
ParaView. It contains `solution`, `exact`, and `point_error` scalar fields.

To also write a CSV file with the grid coordinates:

```bash
mpiexec -n 4 ./laplace_jacobi --n 64 --csv output/solution.csv
```

## Tests and Scalability

The scalability script is:

```bash
bash test/run_scalability.sh
```

It builds the code and runs `n = 16, 32, 64, 128, 256` with 1, 2, and 4 MPI
ranks. The script writes the data in `test/data/performance.csv`, saves hardware
information in `test/hw.info`, and creates `test/performance.png` if `gnuplot`
is available.

By default it assumes an 8-core node. To keep the total number of OpenMP threads
fixed, the runs with 1, 2, and 4 MPI ranks use 8, 4, and 2 OpenMP threads per
rank.

The defaults can be changed without editing the script:

```bash
NS="16 32 64" PROCS="1 2" TOTAL_CORES=4 TOL=1e-6 MAX_IT=50000 bash test/run_scalability.sh
```

On small local OpenMPI installations, the 4-rank run may need oversubscription:

```bash
MPIEXEC_FLAGS="--oversubscribe" bash test/run_scalability.sh
```
