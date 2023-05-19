#!/bin/bash
archivo="../../../affinity"
if [ $# -eq 1 ] && [ "$1" == "1" ]; then
    clear
    g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"
fi

for i in {0..3}
do
    sed -E -i "s/nodeSVector:[[:space:]]*[0-9]+/nodeSVector: $i/g" args
    for j in {0..3}
        do
            sed -E -i "s/nodeThread:[[:space:]]*[0-9]+/nodeThread: $j/g" args
            ./"$archivo.out" args salida
        done 
done