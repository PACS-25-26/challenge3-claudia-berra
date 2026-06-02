# Results

This file is intentionally lightweight in the repository because the meaningful
timings depend on the target machine. Running

```bash
bash test/run_scalability.sh
```

overwrites this file with the full table for `n = 16, 32, 64, 128, 256` and
MPI ranks `1, 2, 4`.

## Local Smoke Test

I verified the code in WSL with a reduced run:

```text
NS=16
PROCS=1 2
OMP_NUM_THREADS=1
TOL=1e-6
MAX_IT=20000
```

| n | MPI ranks | OpenMP threads | iterations | converged | increment norm | exact L2 error | elapsed seconds |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 16 | 1 | 1 | 135 | 1 | 9.289763056e-07 | 0.02855500475 | 0.001028902 |
| 16 | 2 | 1 | 131 | 1 | 1.333779343e-06 | 0.02855072729 | 0.000763453 |

## Discussion

The run with one MPI rank is the serial baseline. The parallel runs exchange
only one row with each adjacent rank at every iteration, so communication is
local and limited. For small grids the parallel result can be disappointing,
because MPI communication and synchronization are comparable with the stencil
work. As `n` increases, each rank owns more interior points and the parallel
runs are expected to become more competitive.

The Jacobi method is matrix-free and simple, but it converges slowly. If a full
scalability run reports `converged = 0`, that row reached `MAX_IT` before all
ranks satisfied the local stopping criterion.
