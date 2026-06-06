if (!exists("input")) input = "test/data/performance.csv"
if (!exists("output")) output = "test/performance.png"

set terminal pngcairo size 1000,700
set output output
set datafile separator comma
set datafile columnheaders
set title "Hybrid Jacobi scalability"
set xlabel "Grid points per direction (n)"
set ylabel "Elapsed time [s]"
set grid
set key outside

plot input using 1:($2 == 1 ? $8 : 1/0) with linespoints linewidth 2 title "1 MPI rank", \
     input using 1:($2 == 2 ? $8 : 1/0) with linespoints linewidth 2 title "2 MPI ranks", \
     input using 1:($2 == 4 ? $8 : 1/0) with linespoints linewidth 2 title "4 MPI ranks"
