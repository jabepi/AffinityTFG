#!/bin/bash
#SBATCH --nodelist=ilk-4,ilk-5,ilk-6,ilk-7
#SBATCH --mem 64G
#SBATCH -t 01:00:00
#SBATCH --exclusive
clear
archivo="affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out" -fsanitize=address

srun "$archivo.out" args/args0 salida333


