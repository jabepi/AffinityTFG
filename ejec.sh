
clear
archivo="affinity0"
g++ "$archivo.cpp" -lnuma -fopenmp -O0 -o "$archivo.out"

#./"$archivo.out" args salida

# #Tamaños de L1, L2, L3
# #tam="32768 262144 26214400"                                                                  
# #tam="8192 16384 32768 65526 131072 262144 524288 1048576 26214400 52428800 104857600 209715200"
# tam="209715200"

# # echo "-------------------Hilos muy afines------------------- " >> salidaVisual.txt
# # for i in $tam
# #     do 
# #     #perf record -e LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,l2_rqsts.all_demand_miss,l2_rqsts.all_demand_references -o "datosA$i.data" 
# #     ./"$archivo.out" 2 0 0 2 $i salidaAfines2Visual
# # done

# #echo "-------------------Hilos muy poco afines-------------------" >> salidaVisual.txt
# for i in $tam
#     do 
#     #perf record -e LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,l2_rqsts.all_demand_miss,l2_rqsts.all_demand_references -o "datosNA$i.data"  
#     ./"$archivo.out" 4 2 2 0 0 0 $i salida5
# done
