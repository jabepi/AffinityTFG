
#Crear la matriz 4x4 con los tiempos entre nodos y un thread
clear
archivo="../affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"

for i in {0..3}
do
    sed -E -i "s/nodeSVector:[[:space:]]*[0-9]+/nodeSVector: $i/g" args
    for j in {0..3}
        do
            sed -E -i "s/nodeThread:[[:space:]]*[0-9]+/nodeThread: $j/g" args
            ./"$archivo.out" args salida
        done 
done