#!/bin/bash
archivo="../../../../affinity"
clear
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"
./"$archivo.out" args salida