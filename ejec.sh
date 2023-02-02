g++ affinity.cpp -lnuma -fopenmp -O0

#Tamaños de L1, L2, L3, 10*L3 y 40*L3
tam="32768 262144 26214400 262144000 1048576000"

echo "-------------------Hilos muy afines------------------- " >> salida.txt
for i in $tam
    do 
    ./a.out 2 0 0 0 $i >> salida.txt 
done

echo "-------------------Hilos muy poco afines-------------------" >> salida.txt 
for i in $tam
    do 
    ./a.out 2 0 2 0 $i >> salida.txt 
done
