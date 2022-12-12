--Esta implementación es temporal--
El tamaño del vector es 100 veces el tamaño de la caché L3, NO HAY flush de caché
(como los vectores son varias veces más grandes que la caché, cuando se pase de 
privado a compartido a privado, no habrán elementos en la caché del "primer privado")

+Para los nodos más cercanos posibles (dentro de eje.sh)
export GOMP_CPU_AFFINITY="0 4"
./a.out 0 0 0

+Para los nodos más lejanos posibles (dentro de eje.sh)
export GOMP_CPU_AFFINITY="0 2"
./a.out 0 2 0

Ejecución: ./ejec.sh

-Dentro del script:
    +se define la posición de los hilos en terminos de CPU 
    +se define la posición de los vectores en terminos de CPU 
(está duplicado)

//TODO esto se modificará para:
    1. leer la salida del comando numactl y ver la topología de los nodos
    2. usar omp places para definir los grupos de cpu's de cada nodo 
    3. hacer las definiones solo en terminos de los nodos.

//TODO el tiempo está expresado por cada hilo de forma individual
    1. Debería estudiarse en el program en global también (por ahora 
    se coje el tiempo del hilo más lento como el tiempo global)


//RESULTADOS:
Hay diferencias sobre todo en lectura y escritura, aunque el hilo 0 no se haya cambiado 
de nodo, va más rápido cuando el hilo 1 está cerca de él (los resultados se repiten 
siempre de forma constante).
(Resultados pendientes de revisión)

#Resultados:
#Nodos: 0 y 2 (los más lejanos)
# Resultado hilo 0:
# T_lectura: 7.33e-07
# T_escritura: 8.3e-08
# T_lectura/escritura: 1.85e-07

# Resultado hilo 2:
# T_lectura: 6.22e-07
# T_escritura: 1.8e-07
# T_lectura/escritura: 1.83e-07

#Nodos: 0 y 0 (los más cercanos)
# Resultado hilo 0:
# T_lectura: 3.76e-07
# T_escritura: 8.7e-08
# T_lectura/escritura: 8.9e-08

# Resultado hilo 1:
# T_lectura: 2.76e-07
# T_escritura: 8.6e-08
# T_lectura/escritura: 8.9e-08