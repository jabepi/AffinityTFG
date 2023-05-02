#!/bin/bash
clear
archivo="../../affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"

# Define un vector con elementos que contienen pares de números
vector=("0 2 0" "0 0 2" "0 0 0")
elmRecorer=("209715200 209715200" "209715200 104857600")

# Recorre el vector

for elmRecorer in "${elmRecorer[@]}"; do
    # Divide el elemento actual en dos variables (num1 y num2)
    read -a elmRecorer <<< "$elmRecorer"
    Nelm0=${elmRecorer[0]}
    Nelm1=${elmRecorer[1]}

    for elemento in "${vector[@]}"; do
        # Divide el elemento actual en dos variables (num1 y num2)
        read -a numeros <<< "$elemento"
        H0=${numeros[0]}
        H1=${numeros[1]}
        D0=${numeros[2]}

        sed -i "s/^nodeThread:.*$/nodeThread: $H0 $H1/g" args
        sed -i "s/^nodeSVector:.*$/nodeSVector: $D0/g" args
        sed -i "s/^numSElmProc:.*$/numSElmProc: $Nelm0 $Nelm1/g" args

        ./"$archivo.out" args salida
    done

done
