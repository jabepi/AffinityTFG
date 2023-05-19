#!/bin/bash
archivo="../../../../affinity"
if [ $# -eq 1 ] && [ "$1" == "1" ]; then
    clear
    g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"
fi

# Define un vector con elementos que contienen pares de números
vector=("0 0 0 2")

# Recorre el vector
for elemento in "${vector[@]}"; do
    # Divide el elemento actual en dos variables (num1 y num2)
    read -a numeros <<< "$elemento"
    H0=${numeros[0]}
    H1=${numeros[1]}
    D0=${numeros[2]}
    D1=${numeros[3]}

    sed -i "s/^nodeThread:.*$/nodeThread: $H0 $H1/g" args
    sed -i "s/^nodeSVector:.*$/nodeSVector: $D0 $D1/g" args

    ./"$archivo.out" args salida
done

