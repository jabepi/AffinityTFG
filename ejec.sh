#Indicar las cpu de los dos hilos
export GOMP_CPU_AFFINITY="0 4"
g++ affinity.cpp -lnuma -fopenmp

#indicar el nodo de cada hilo de del vector compartido
./a.out 0 0 0

