#!/bin/bash
archivo="../../affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"

#Ejecutamos todas las pruebas
find . -type f -name "salida*" -exec rm -f {} \;
carpetas=$(find . -maxdepth 1 -type d -name "pruebas*")
IFS=$'\n' read -rd '' -a carpetas <<< "$carpetas"

for elemento in "${carpetas[@]}"; do
    cd "$elemento"
    carpetasAux=$(find . -maxdepth 1 -type d -name "pruebas*")
    IFS=$'\n' read -rd '' -a carpetasAux <<< "$carpetasAux"

    if [ -z "$carpetasAux" ]; then
        sh ejec.sh
    else
        for elementoAux in "${carpetasAux[@]}"; do
            cd "$elementoAux"
            sh ejec.sh
            cd ..
        done
    fi
    cd ..
done

