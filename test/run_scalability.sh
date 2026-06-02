#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXEC="${EXEC:-${ROOT_DIR}/laplace_jacobi}"
MPIEXEC="${MPIEXEC:-mpiexec}"
MPIEXEC_FLAGS="${MPIEXEC_FLAGS:-}"
NS="${NS:-16 32 64 128 256}"
PROCS="${PROCS:-1 2 4}"
OMP_NUM_THREADS="${OMP_NUM_THREADS:-1}"
TOL="${TOL:-1e-6}"
MAX_IT="${MAX_IT:-200000}"
DATA_DIR="${ROOT_DIR}/test/data"
CSV_FILE="${DATA_DIR}/performance.csv"
RESULT_FILE="${ROOT_DIR}/test/RESULT.md"
PLOT_FILE="${ROOT_DIR}/test/performance.png"

mkdir -p "${DATA_DIR}"
read -r -a MPIEXEC_FLAGS_ARRAY <<< "${MPIEXEC_FLAGS}"

make -C "${ROOT_DIR}"

if command -v lscpu >/dev/null 2>&1; then
    lscpu > "${ROOT_DIR}/test/hw.info"
else
    uname -a > "${ROOT_DIR}/test/hw.info"
fi

echo "n,processes,omp_threads,iterations,converged,increment_norm,exact_l2_error,elapsed_seconds" > "${CSV_FILE}"

for n in ${NS}; do
    for p in ${PROCS}; do
        echo "Running n=${n}, MPI ranks=${p}, OpenMP threads=${OMP_NUM_THREADS}"
        line="$(OMP_NUM_THREADS="${OMP_NUM_THREADS}" "${MPIEXEC}" "${MPIEXEC_FLAGS_ARRAY[@]}" -n "${p}" "${EXEC}" \
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
OMP_NUM_THREADS=${OMP_NUM_THREADS}
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

The run with one MPI rank is the serial baseline. The parallel runs exchange only
one row with each adjacent rank at every iteration, so the communication pattern
is local and simple. For small grids the timing may be worse with more ranks,
because the amount of local work is too small to compensate for MPI
communication and synchronization. As n increases, each rank owns more interior
points and the parallel runs are expected to become more competitive.

The Jacobi method is intentionally simple and matrix-free, but it converges
slowly. If a row has \`converged = 0\`, the run reached \`MAX_IT\` before all
ranks satisfied the local stopping criterion.
EOF_RESULT

echo "Wrote ${CSV_FILE}"
echo "Wrote ${RESULT_FILE}"
if [ -f "${PLOT_FILE}" ]; then
    echo "Wrote ${PLOT_FILE}"
fi
