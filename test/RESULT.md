# Results

I ran the scalability test on the Politecnico di Milano HPC cluster, on node
`cpu01.mate.polimi.it`, with this interactive PBS allocation:

```text
qsub -I -q cpu -l select=1:ncpus=8:mpiprocs=4
NS=16 32 64 128 256
PROCS=1 2 4
MPIEXEC_FLAGS=
TOTAL_CORES=8
OMP_NUM_THREADS=floor(TOTAL_CORES / MPI ranks), minimum 1
TOL=1e-6
MAX_IT=200000
```

The raw data are in `test/data/performance.csv`.

| n | MPI ranks | OpenMP threads | iterations | converged | increment norm | exact L2 error | elapsed seconds |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 16 | 1 | 8 | 135 | 1 | 9.289763056e-07 | 0.02855500475 | 0.014566162 |
| 16 | 2 | 4 | 135 | 1 | 9.289763056e-07 | 0.02855500475 | 0.00777672 |
| 16 | 4 | 2 | 135 | 1 | 9.289763056e-07 | 0.02855500475 | 0.619318893 |
| 32 | 1 | 8 | 531 | 1 | 9.92307949e-07 | 0.009502410121 | 0.074331206 |
| 32 | 2 | 4 | 531 | 1 | 9.92307949e-07 | 0.009502410121 | 0.033473784 |
| 32 | 4 | 2 | 531 | 1 | 9.92307949e-07 | 0.009502410121 | 5.873073995 |
| 64 | 1 | 8 | 1987 | 1 | 9.967913979e-07 | 0.003091599435 | 0.610916005 |
| 64 | 2 | 4 | 1987 | 1 | 9.967913979e-07 | 0.003091599435 | 0.383368286 |
| 64 | 4 | 2 | 1987 | 1 | 9.967913979e-07 | 0.003091599435 | 1.645076001 |
| 128 | 1 | 8 | 7221 | 1 | 9.990061106e-07 | 0.0003340057417 | 5.819977323 |
| 128 | 2 | 4 | 7221 | 1 | 9.990061106e-07 | 0.0003340057417 | 3.894257863 |
| 128 | 4 | 2 | 7221 | 1 | 9.990061106e-07 | 0.0003340057417 | 5.221538625 |
| 256 | 1 | 8 | 25671 | 1 | 9.998144268e-07 | 0.00288878836 | 73.43973225 |
| 256 | 2 | 4 | 25671 | 1 | 9.998144268e-07 | 0.00288878836 | 44.12249883 |
| 256 | 4 | 2 | 25671 | 1 | 9.998144268e-07 | 0.00288878836 | 16.97913865 |

## Speed-up

I computed the speed-up as S(p) = T(1) / T(p) for each value of `n`. The total
OpenMP thread budget is kept fixed at 8 threads.

| n | T(1) s | T(2) s | S(2) | T(4) s | S(4) |
|---:|---:|---:|---:|---:|---:|
| 16 | 0.0146 | 0.00778 | 1.87 | 0.619 | 0.02 |
| 32 | 0.0743 | 0.0335 | 2.22 | 5.87 | 0.01 |
| 64 | 0.611 | 0.383 | 1.59 | 1.65 | 0.37 |
| 128 | 5.82 | 3.89 | 1.49 | 5.22 | 1.11 |
| 256 | 73.4 | 44.1 | 1.66 | 17.0 | 4.33 |

## Discussion

All runs converged before reaching `MAX_IT`. For a fixed grid size, the number
of iterations, the final increment norm, and the exact L2 error are the same for
1, 2, and 4 MPI ranks. This is a useful check: after changing the stopping test
to a global norm, the numerical result does not depend on the row decomposition.

The small cases are not very meaningful for timing. With `n=16` or `n=32`, each
rank has very few rows, so communication and synchronization cost more than the
actual stencil work. This is especially visible in the 4-rank runs.

For larger grids the behavior improves because each rank has more interior
points to update. The best time in this run is for `n=256` with 4 MPI ranks and
2 OpenMP threads per rank. In that case the speed-up is 4.33 compared with the
1-rank run with 8 OpenMP threads.

The exact L2 error decreases from `n=16` to `n=128`. For `n=256` it increases
again, which means that with `TOL=1e-6` the Jacobi iteration error is still
visible. To study only the spatial discretisation error, I would need to repeat
the test with a smaller tolerance.
