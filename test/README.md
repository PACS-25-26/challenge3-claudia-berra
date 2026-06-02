# Test Folder

This folder contains a reproducible scalability test for the hybrid MPI/OpenMP
Jacobi solver.

Run from the repository root:

```bash
bash test/run_scalability.sh
```

The script uses these defaults:

```text
NS="16 32 64 128 256"
PROCS="1 2 4"
OMP_NUM_THREADS=1
TOL=1e-6
MAX_IT=200000
```

Outputs:

```text
test/data/performance.csv   Raw timing and error data.
test/performance.png        Timing plot, if gnuplot is installed.
test/hw.info                Hardware information.
test/RESULT.md              Short discussion generated after the run.
```

For a quick smoke test:

```bash
NS="16 32" PROCS="1 2" TOL=1e-4 MAX_IT=10000 bash test/run_scalability.sh
```

If OpenMPI refuses the 4-rank run on a small laptop or WSL instance, use:

```bash
MPIEXEC_FLAGS="--oversubscribe" bash test/run_scalability.sh
```
