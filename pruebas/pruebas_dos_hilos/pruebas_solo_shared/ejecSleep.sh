#!/bin/bash
archivo="../../../affinity"
if [ $# -eq 2 ] && [ "$1" == "1" ]; then
    clear
    if [ "$2" == "gcc" ]; then 
        g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"
    elif [ "$2" == "icc" ];then 
        source /opt/intel/oneapi/setvars.sh
        /opt/intel/oneapi/compiler/latest/linux/bin/icpx "$archivo.cpp" -qno-opt-prefetch -qopt-prefetch-distance=0,0 -lnuma -fopenmp -O0 -o "$archivo.out"
    fi
fi

# Posiciones de los hilos y del vector
vector=( "0 2 0" "0 0 2" "0 0 0")
sleepValues=( "0" "10" "20" "30" "40" "100" "200" "300" "400" "500" "600" "700" "800" "900" )

for elemento in "${vector[@]}"; do

    # Obtengo las posiciones de los hilos y del vector
    read -a numeros <<< "$elemento"
    H0=${numeros[0]}
    H1=${numeros[1]}
    D0=${numeros[2]}

    for sleepValue in "${sleepValues[@]}"; do
        sed -i "s/^nodePerThread:.*$/nodePerThread: $H0 $H1/g" args
        sed -i "s/^nodePerVector:.*$/nodePerVector: $D0/g" args
        sed -i -r "s/(sleep:)[0-9]+/\1$sleepValue/g" args
        ./"$archivo.out" args salidaSleep
    done
done