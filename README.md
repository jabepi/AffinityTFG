--Esta implementación es temporal--
Ejecución:  ./a.out 2 0 0 0 1000
                -Número de hilos 
                -lista de nodos por hilo 
                -nodo de la zona compartida
                -tamaño de los vectores 

//TODO el tiempo está expresado por cada hilo de forma individual
    1. Debería estudiarse en el program en global también (por ahora 
    se coje el tiempo del hilo más lento como el tiempo global)

//RESULTADOS: para hilos lo más cercanos y los más alejados posibles
//TODO: para que los resultados de los tamaños más pequeños que no llevan la 
caché abría que hacer un flush.  

Los resultados son 

# Resultados:
## Hilos muy afines: 

```
-Datos- 
Hilo: 0 - CPU: 4 - Node: 0
Hilo: 1 - CPU: 16 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 0.00271154
T_escritura: 0.00221991
T_lectura/escritura: 0.00406873

Resultado hilo 1 :
T_lectura: 0.00271228
T_escritura: 0.00223152
T_lectura/escritura: 0.00408958

-Datos- 
Hilo: 0 - CPU: 8 - Node: 0
Hilo: 1 - CPU: 28 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 0.0213086
T_escritura: 0.0181628
T_lectura/escritura: 0.0331759

Resultado hilo 1 :
T_lectura: 0.0213219
T_escritura: 0.0181686
T_lectura/escritura: 0.0331931

-Datos- 
Hilo: 0 - CPU: 32 - Node: 0
Hilo: 1 - CPU: 12 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 2.03889
T_escritura: 1.91711
T_lectura/escritura: 3.31532

Resultado hilo 1 :
T_lectura: 2.03889
T_escritura: 1.9152
T_lectura/escritura: 3.31294

-Datos- 
Hilo: 0 - CPU: 36 - Node: 0
Hilo: 1 - CPU: 8 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 20.383
T_escritura: 19.0656
T_lectura/escritura: 32.7431

Resultado hilo 1 :
T_lectura: 20.383
T_escritura: 19.0256
T_lectura/escritura: 32.727

-Datos- 
Hilo: 0 - CPU: 0 - Node: 0
Hilo: 1 - CPU: 16 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 80.3056
T_escritura: 75.0798
T_lectura/escritura: 128.953

Resultado hilo 1 :
T_lectura: 80.3056
T_escritura: 74.8972
T_lectura/escritura: 129.02

```
## Hilos muy poco afines: 

```
-Datos- 
Hilo: 0 - CPU: 12 - Node: 0
Hilo: 1 - CPU: 26 - Node: 2

-Resultados-
Resultado hilo 0 :
T_lectura: 0.00257229
T_escritura: 0.00226234
T_lectura/escritura: 0.00412258

Resultado hilo 1 :
T_lectura: 0.00263696
T_escritura: 0.00235637
T_lectura/escritura: 0.00426873

-Datos- 
Hilo: 1 - CPU: 26 - Node: 2
Hilo: 0 - CPU: 16 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 0.0203549
T_escritura: 0.0178241
T_lectura/escritura: 0.0322155

Resultado hilo 1 :
T_lectura: 0.0201501
T_escritura: 0.0180032
T_lectura/escritura: 0.0322401

-Datos- 
Hilo: 0 - CPU: 0 - Node: 0
Hilo: 1 - CPU: 18 - Node: 2

-Resultados-
Resultado hilo 0 :
T_lectura: 2.00372
T_escritura: 1.82286
T_lectura/escritura: 3.21593

Resultado hilo 1 :
T_lectura: 2.02842
T_escritura: 1.83275
T_lectura/escritura: 3.2188

-Datos- 
Hilo: 1 - CPU: 26 - Node: 2
Hilo: 0 - CPU: 4 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 20.3103
T_escritura: 18.691
T_lectura/escritura: 32.165

Resultado hilo 1 :
T_lectura: 20.3112
T_escritura: 18.7404
T_lectura/escritura: 32.1766

-Datos- 
Hilo: 1 - CPU: 22 - Node: 2
Hilo: 0 - CPU: 8 - Node: 0

-Resultados-
Resultado hilo 0 :
T_lectura: 81.3991
T_escritura: 74.7056
T_lectura/escritura: 128.66

Resultado hilo 1 :
T_lectura: 81.4068
T_escritura: 74.9402
T_lectura/escritura: 128.671

```
