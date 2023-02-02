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

# Resultados:

## Hilos muy afines: 
```
-Datos-                                                             
Tamaño de los sets privados: 1048576000                             
Tamaño de los sets compartidos: 1048576000                          
Hilo: 1 - CPU: 10 - Node: 2                                         
Hilo: 0 - CPU: 4 - Node: 0                                          

-Resultados-                                                        
Resultado hilo 0 :                                                  
                         T_Datos_Privados    T_Datos_Compartidos    
T_lectura:               40.3633             40.4381                
T_escritura:             38.9513             40.7471                
T_lectura/escritura:     71.4319             66.41                  

Resultado hilo 1 :                                                  
                         T_Datos_Privados    T_Datos_Compartidos    
T_lectura:               40.4865             40.4373                
T_escritura:             39.041              40.7463                
T_lectura/escritura:     71.2954             66.2069            

```
## Hilos muy poco afines: 

```

-Datos- 
Tamaño de los sets privados: 1048576000
Tamaño de los sets compartidos: 1048576000
Hilo: 0 - CPU: 0 - Node: 0
Hilo: 1 - CPU: 32 - Node: 0

-Resultados-
Resultado hilo 0 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura:               40.5948             42.7865
T_escritura:             38.9353             41.5076
T_lectura/escritura:     71.7079             66.5035

Resultado hilo 1 :
                         T_Datos_Privados    T_Datos_Compartidos 
T_lectura:               40.5202             39.51
T_escritura:             39.1679             38.9819
T_lectura/escritura:     71.5789             66.5052
```
