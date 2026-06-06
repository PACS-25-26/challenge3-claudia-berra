#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXEC="${EXEC:-${ROOT_DIR}/laplace_jacobi}"
MPIEXEC="${MPIEXEC:-mpiexec}"
MPIEXEC_FLAGS="${MPIEXEC_FLAGS:-}"
NS="${NS:-16 32 64 128 256}"
PROCS="${PROCS:-1 2 4}"
TOTAL_CORES="${TOTAL_CORES:-8}"
TOL="${TOL:-1e-6}"
MAX_IT="${MAX_IT:-200000}"
DATA_DIR="${ROOT_DIR}/test/data"
CSV_FILE="${DATA_DIR}/performance.csv"
RESULT_FILE="${ROOT_DIR}/test/RESULT.md"
PLOT_FILE="${ROOT_DIR}/test/performance.png"

mkdir -p "${DATA_DIR}"
read -r -a MPIEXEC_FLAGS_ARRAY <<< "${MPIEXEC_FLAGS}"

if ! [[ "${TOTAL_CORES}" =~ ^[0-9]+$ ]] || (( TOTAL_CORES < 1 )); then
    echo "TOTAL_CORES must be a positive integer." >&2
    exit 1
fi

make -C "${ROOT_DIR}"

if command -v lscpu >/dev/null 2>&1; then
    lscpu > "${ROOT_DIR}/test/hw.info"
else
    uname -a > "${ROOT_DIR}/test/hw.info"
fi

echo "n,processes,omp_threads,iterations,converged,increment_norm,exact_l2_error,elapsed_seconds" > "${CSV_FILE}"

for n in ${NS}; do
    for p in ${PROCS}; do
        if ! [[ "${p}" =~ ^[0-9]+$ ]] || (( p < 1 )); then
            echo "PROCS entries must be positive integers." >&2
            exit 1
        fi

        threads=$(( TOTAL_CORES / p ))
        if (( threads < 1 )); then
            threads=1
        fi

        echo "Running n=${n}, MPI ranks=${p}, OpenMP threads=${threads}"
        line="$(OMP_NUM_THREADS="${threads}" "${MPIEXEC}" "${MPIEXEC_FLAGS_ARRAY[@]}" -n "${p}" "${EXEC}" \
            --n "${n}" \
            --tol "${TOL}" \
            --max-it "${MAX_IT}" \
            --no-output \
            --quiet | tail -n 1)"
        echo "${line}" >> "${CSV_FILE}"
    done
done

if command -v gnuplot >/dev/null 2>&1; then
    gnuplot -e "input='${CSV_FILE}'; output='${PLOT_FILE}'" "${ROOT_DIR}/test/plot_performance.gp"
fi

cat > "${RESULT_FILE}" <<EOF_RESULT
# Results

The scalability test was run with:

\`\`\`text
NS=${NS}
PROCS=${PROCS}
MPIEXEC_FLAGS=${MPIEXEC_FLAGS}
TOTAL_CORES=${TOTAL_CORES}
OMP_NUM_THREADS=floor(TOTAL_CORES / MPI ranks), minimum 1
TOL=${TOL}
MAX_IT=${MAX_IT}
\`\`\`

Raw data are stored in \`test/data/performance.csv\`.

| n | MPI ranks | OpenMP threads | iterations | converged | increment norm | exact L2 error | elapsed seconds |
|---:|---:|---:|---:|---:|---:|---:|---:|
EOF_RESULT

tail -n +2 "${CSV_FILE}" | while IFS=, read -r n p threads iterations converged increment error elapsed; do
    printf "| %s | %s | %s | %s | %s | %s | %s | %s |\n" \
        "${n}" "${p}" "${threads}" "${iterations}" "${converged}" "${increment}" "${error}" "${elapsed}" >> "${RESULT_FILE}"
done

cat >> "${RESULT_FILE}" <<EOF_RESULT

## Discussion

The run with one MPI rank is the baseline for the same total core budget. For
parallel runs, the script keeps the total OpenMP thread budget approximately
constant by assigning fewer OpenMP threads to each rank as the number of MPI
ranks increases.

Each Jacobi iteration exchanges one row with each adjacent rank and then uses a
global MPI_Allreduce to compute the h-weighted increment norm. For small grids
the timing may be worse with more ranks, because the amount of local stencil
work is too small to compensate for MPI communication and synchronization. As n
increases, each rank owns more interior points and the parallel runs are
expected to become more competitive.

The Jacobi method is intentionally simple and matrix-free, but it converges
slowly. If a row has \`converged = 0\`, the run reached \`MAX_IT\` before the
global increment norm went below the requested tolerance.
EOF_RESULT

echo "Wrote ${CSV_FILE}"
echo "Wrote ${RESULT_FILE}"
if [ -f "${PLOT_FILE}" ]; then
    echo "Wrote ${PLOT_FILE}"
fi
