# Test Folder

This folder contains the script I used for the scalability test of the hybrid
MPI/OpenMP Jacobi solver.

Run it from the repository root:

```bash
bash test/run_scalability.sh
```

The default parameters are:

```text
NS="16 32 64 128 256"
PROCS="1 2 4"
TOTAL_CORES=8
TOL=1e-6
MAX_IT=200000
```

For each MPI process count `p`, the script sets
`OMP_NUM_THREADS=max(1, floor(TOTAL_CORES / p))`. With the default values, the
runs with 1, 2, and 4 MPI ranks use 8, 4, and 2 OpenMP threads per rank.

Outputs:

```text
test/data/performance.csv   Raw timing and error data.
test/performance.png        Timing plot, if gnuplot is installed.
test/hw.info                Hardware information.
test/RESULT.md              Result table and short discussion.
```

For a quick local check:

```bash
NS="16 32" PROCS="1 2" TOTAL_CORES=4 TOL=1e-4 MAX_IT=10000 bash test/run_scalability.sh
```

If OpenMPI refuses the 4-rank run on a small laptop or WSL instance, use:

```bash
MPIEXEC_FLAGS="--oversubscribe" bash test/run_scalability.sh
```
