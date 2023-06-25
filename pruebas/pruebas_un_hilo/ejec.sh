#!/bin/bash
archivo="../../affinity"
if [ $# -eq 1 ] && [ "$1" == "1" ]; then
    clear
    g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"
fi

nodos=4
for (( i=0; i<$nodos; i++ ))
do
    sed -E -i "s/nodePerThread:[[:space:]]*[0-9]+/nodePerThread: $i/g" args
    for (( j=0; j<$nodos; j++ ))
    do
        sed -E -i "s/nodePerVector:[[:space:]]*[0-9]+/nodePerVector: $j/g" args
        ./"$archivo.out" args salida
        
    done 
done