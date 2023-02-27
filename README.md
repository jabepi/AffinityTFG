--V2. Esta implementación es temporal--
Ejecución:  ./a.out 2 0 0 0 1000
                -Número de hilos 
                -lista de nodos por hilo 
                -nodo de la zona compartida
                -tamaño de los vectores 


//Características nuevas.
1. Hay 4 ejecutables con distintas pruebas para conseguir entender el comportamiento de los hilos 
lejanos y cercanos y sus retrasos. 
    
# Resultados:
Resultados finales en los que el acceso a datos compartido ya es más lento que el acceso a datos privados,
una vez de realizan varias iteracioines de lectura/escritura sobre un mismo elemento. 
## Hilos muy afines: 

```
-Datos- 
Tamaño de los sets privados: 26214400
Tamaño de los sets compartidos: 26214400
Hilo: 1 - CPU: 4 - Node: 0
Hilo: 0 - CPU: 24 - Node: 0

-Resultados-
Resultado hilo 0 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura/escritura:     219.473             232.929

Resultado hilo 1 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura/escritura:     219.519             233.116
```
## Hilos muy poco afines: 

```
-Datos- 
Tamaño de los sets privados: 26214400
Tamaño de los sets compartidos: 26214400
Hilo: 1 - CPU: 34 - Node: 2
Hilo: 0 - CPU: 36 - Node: 0

-Resultados-
Resultado hilo 0 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura/escritura:     210.449             221.13

Resultado hilo 1 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura/escritura:     210.555             221.29
```
