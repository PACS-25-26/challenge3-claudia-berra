# Matrix-Free Hybrid Jacobi Solver

This repository contains Challenge 3 for the PACS course: a matrix-free solver for

```text
-Delta u = f  in (0,1)^2,
u = g         on the boundary.
```

The default problem is the one requested in the assignment:

```text
f(x,y) = 8*pi^2 sin(2*pi*x) sin(2*pi*y),
u(x,y) = sin(2*pi*x) sin(2*pi*y).
```

The implementation uses only standard C++ plus the MPI/OpenMP tools covered in the
lectures: `MPI_Init`, `MPI_Comm_rank`, `MPI_Comm_size`, `MPI_Sendrecv`,
`MPI_Allreduce`, `MPI_Gatherv`, `MPI_Barrier`, OpenMP `parallel for` with a
`reduction`, a direct `Makefile`, `<chrono>`, and `<filesystem>`.

## Repository Layout

```text
include/            Public headers
src/                Source files
test/               Scalability script and discussion
Makefile            Build rules
README.md           This file
```

No matrix for the discrete Laplacian is assembled. Each rank owns a balanced block
of consecutive grid rows, plus two ghost rows used to exchange data with adjacent
MPI ranks.

## Build

The expected build environment is a Linux machine or cluster with an MPI C++
compiler wrapper and OpenMP support:

```bash
make
```

By default the `Makefile` uses `mpic++` and `-fopenmp`. You can override them from
the command line:

```bash
make CXX=mpic++ CXXFLAGS="-std=c++20 -O3 -Wall -Wextra -pedantic -fopenmp"
```

## Run

The number of MPI tasks is selected by the user through `mpiexec -n`.

```bash
mpiexec -n 4 ./laplace_jacobi --n 128 --tol 1e-8 --max-it 200000
```

Set the number of OpenMP threads with the usual environment variable:

```bash
OMP_NUM_THREADS=2 mpiexec -n 4 ./laplace_jacobi --n 128
```

Useful options:

```text
--n <int>              Grid points per direction.
--tol <real>           Local convergence tolerance.
--max-it <int>         Maximum Jacobi iterations.
--forcing <name>       sine, zero, poly.
--boundary <name>      homogeneous, exact.
--vtk <file>           VTK output path.
--csv <file>           CSV output path.
--no-output            Disable VTK and CSV output.
--quiet                Print one CSV result line only.
```

The `poly` forcing is a small extra test with non-homogeneous Dirichlet data:
`u(x,y)=x^2+y^2`, `f=-4`, and `--boundary exact`.

## Numerical Method

Rows are distributed as evenly as possible among MPI ranks. At every Jacobi
iteration:

1. Each rank exchanges its first and last owned rows with adjacent ranks using
   `MPI_Sendrecv`.
2. Interior points are updated with the four-point stencil.
3. The local squared increment is accumulated with an OpenMP reduction.
4. Each rank checks its own local norm, then `MPI_Allreduce` sums the boolean
   convergence flags. The solver stops only when all ranks are locally converged.

The reported increment and exact error use the discrete norm requested in the
assignment:

```text
sqrt(h * sum_ij value_ij^2).
```

## Output

Rank 0 gathers the complete solution with `MPI_Gatherv`.

By default the code writes:

```text
output/solution.vtk
```

The VTK file is a legacy ASCII `STRUCTURED_POINTS` dataset and can be opened in
ParaView. It contains three scalar fields: `solution`, `exact`, and
`point_error`.

To also export explicit grid coordinates:

```bash
mpiexec -n 4 ./laplace_jacobi --n 64 --csv output/solution.csv
```

## Tests and Scalability

Run the scalability script with:

```bash
bash test/run_scalability.sh
```

It builds the code, runs `n = 2^k`, `k = 4,...,8`, with 1, 2, and 4 MPI ranks,
stores the data in `test/data/performance.csv`, records hardware information in
`test/hw.info`, and creates `test/performance.png` if `gnuplot` is available.

The defaults can be changed without editing the script:

```bash
NS="16 32 64" PROCS="1 2" OMP_NUM_THREADS=2 TOL=1e-6 MAX_IT=50000 bash test/run_scalability.sh
```

On small local OpenMPI installations you may need oversubscription for 4 ranks:

```bash
MPIEXEC_FLAGS="--oversubscribe" bash test/run_scalability.sh
```
