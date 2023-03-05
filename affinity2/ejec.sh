
archivo="affinity"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"

#Tamaños de L1, L2, L3, 10*L3
#tam="32768 262144 26214400 262144000"
tam="10000 20000 32768 40000 80000 200000 262144 400000 800000 20000000 26214400 40000000 80000000 200000000"

echo "-------------------Hilos muy afines------------------- " >> salida
for i in $tam
    do 
    ./"$archivo.out" 2 0 0 0 $i salida
done

echo "-------------------Hilos muy poco afines-------------------" >> salida
for i in $tam
    do 
    ./"$archivo.out" 2 0 2 0 $i salida
done