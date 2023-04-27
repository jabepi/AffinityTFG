clear
archivo="affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out" -Wall
