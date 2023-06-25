# Manual de uso

El `benchmark` está compuesto por tres archivos principales, `affinity.cpp`, `parser.hpp` y `types.h` y dos archivos de apoyo, `numaalloc.hpp` y `CLI11.hpp`. 
Los primeros 3 archivos son los más susceptibles a modificaciones para la creación de nuevas pruebas, en cambio los otros dos ofrecen funciones auxiliares útiles, como `numa_alloc_onnode` para reservar memoria en un nodo específico. 

## Características generales

Los 3 archivos principales poseen las siguientes funciones: 

- `affinity.cpp`: contiene toda la lógica de las operaciones sobre las que se realizarán las pruebas incluyendo: 
    - Funciones necesarias para realizar la medición de tiempo. 
    - Gestión para la creación de hilos con OpenMP. 
    - El código de las funciones que se probarán. 
    - El código necesario para imprimir los resultados de forma visual. 
    - El código del cálculo del speedup de las operaciones elegidas. 
- `parser.hpp`: contiene el código para procesar el archivo con los argumentos de la ejecución.
- `types.h`: contiene los tipos de datos que se emplearán durante la ejecución. Por defecto, contiene el tipo de datos del vector y el tipo de dato asociado a su tamaño. 

## Modificaciones sobre el software

El código se diseñó con las modificaciones en mente de forma que fuese fácilmente adaptable a los requerimiento específicos de las pruebas que quieran realizarse. En concreto, por cada archivo, la estructura es la expuesta a continuación.

### Affinity.cpp

Como ya se expuso, en él se introducirán las pruebas a realizar y permitirá la obtención de costes de ejecución y speedup entre los distintos experimentos. 

En específico, dispone de una clase llamada `Thread_array` que contendrá la información privada de cada hilo. 
Dentro de esta clase, se definirán las pruebas a realizar como una función nueva. Todas la funciones definidas deberán tener las estructura definida en el template del código siguiente dónde, se recibe como argumento el tamaño del vector en un tipo de dato `vectorSize` y se devuelve un `int`. 

```cpp
typedef int (Thread_array::*MemFuncPtr)(vectorSize);
```
Una vez creada la función para la realización de las pruebas en la clase, es necesario modificar el contenido de la función `setFunctionsPointers` para añadirla al mapa de funciones disponibles y asignarle un identificador. 
En el siguiente código vemos un ejemplo, de cómo añadir la función `ejemplo` y definir el identificador `ej`.

```cpp
unordered_map<std::string, MemFuncPtr> setFunctionsPointers(){
    functMap["ej"] = &Thread_array::ejemplo;
    return functMap;
}
```

Para poder emplear otros argumentos además del vector a recorrer, es posible acceder dentro de cada función al _hashmap_ `arg`, que contiene todos los argumentos definidos en el archivo procesado por el _parser_. A continuación, presentamos un ejemplo de una función que, además de emplear el tamaño del vector, emplea un argumento llamado _sleep_ obtenido del _hashmap_.

```cpp
int read_s(vectorSize size){
    vectorSize i = 0; 
    volatile int add = 0;
    
    int sleep = stoi(arg["sleep"]);
    if(omp_get_thread_num() == 1){
        this_thread::sleep_for(chrono::milliseconds(sleep));
    }
    for(i = 0; i < size; i++){
        add = priv_shared_vec[i]; 
        i+=dist;
    }
    return add;
}
```

Por último, todos los hilos acceden al vector de datos empleado el puntero `priv_shared_vec`, dado que este es local al hilo. Además, es necesario destacar que:

- El mapa de argumentos es local y privado para cada hilo para evitar problemas de sincronización al realizar los accesos. Además, es un _hashmap_ por lo que el acceso es O(1) para todos los argumentos, evitando variaciones artificiales sobre las ejecuciones.
- Todos los argumentos definidos en el mapa, son `strings` por lo que es necesario convertirlo al tipo de dato requerido antes de emplearlos.

De esta manera, para la incorporación de nuevas pruebas, únicamente es necesario añadir el código como una función nueva en la clase `Thread_array` e incorporarla al _hashmap_ indicando el identificador.

### Parser.cpp
Este archivo procesa y almacena los argumentos contenidos en otro archivo externo de nombre a elegir. Por defecto, permite procesar un archivo con la estructura expuesta en el siguiente código: 

En este archivo se especifica: 

- `opType`: las pruebas a realizar separadas por un espacio. Las operaciones se indicarán introduciendo el identificador definido en el archivo `affinity.cpp`.
- `nodePerThread`: el nodo donde se localizará cada hilo separado por espacios. Vemos como el número de hilos es intrínseco a este argumento, si por ejemplo se indica `0 1`, se crearán dos hilos, en concreto, el hilo 0 se creará en el nodo 0 y el hilo 1 se creará en el nodo 1.
- `nodePerVector`: el nodo donde se localizarán los vectores de datos. De nuevo, el número de hilos que se creará dependerá del número de nodos especificados. En el siguiente código, se crearán dos conjuntos de datos en el nodo 0.
- `tamPerVector`: el tamaño de los vectores creados. El número de tamaños deberá coincidir con el número de vectores especificados en el argumento `nodePerVector`.
- `vectorPerThread`: define qué hilos accederán a cada vector. En concreto, se separarán por comas los identificadores de los hilos que acceden a cada vector. Así en el siguiente código vemos como al vector 0 accederán los hilos 0 y 1, y al vector 1 accederán los hilos 2 y 3.
- `summaryType`: indica el tipo de resumen que se mostrará en el archivo de salida, pudiendo ser de tipo "max" para mostrar el tiempo de ejecución mayor de entre todos los hilos, o "sum" para la suma de todos los tiempos de ejecución.
- `speedup`: indica qué _speedups_ se calcularán entre el conjunto de pruebas realizadas. En el siguiente código vemos como se calculará la relación entre el coste de las escrituras y las lecturas. Es posible calcular tantos como se crean necesarios, separando cada argumento por espacios.
- `numIter`: indica el número de veces que se repetirán cada una de las pruebas. El resultado mostrado por hilo, será el cálculo de la media de todas estas repeticiones.
- `argsExtra`: se emplea para introducir tantos argumentos extra como se consideren oportunos, separándolos por espacios en blanco. Por cada argumento se indica un identificador (cualquier cadena de caracteres sin espacios), seguido de dos puntos y el valor del argumento.

```txt
opType: write read
nodePerThread: 0 1 2 3
nodePerVector: 0 0
tamPerVector: 209715200 209715200
vectorPerThread: 0 1, 2 3
summaryType: max
speedupCalc: write/read
numIter: 10
argsExtra: sleep:100 stride:16
```

Los argumentos deben seguir siempre este orden, en caso que desear modificarlo, eliminar o añadir argumentos será necesario realizar ajustes sobre la función `parse_input_file`.

### types.h
En este archivo se introducirán los tipos de datos que se emplearán en la ejecución. Por defecto el contenido del archivo es el mostrado en el siguiente código. En este caso, se indica que el tipo de datos del vector sobre el que se realizarán los accesos será `char` y el tipo de dato del tamaño del vector será un `long long int`.

```cpp
#ifndef MYTYPES_H
#define MYTYPES_H

typedef char vectorType;
typedef long long int vectorSize;

#endif // MYTYPES_H
```

## Recomendaciones al crear pruebas
Para que las pruebas ofrezcan resultados consistentes, es importante tener en consideración:

1. Las variables relevantes empleadas deberían ser privadas y no estáticas dentro de la clase _Tread_array_. De esta forma, garantizamos que se localicen en el nodo del hilo asociado a la instancia de la instancia creada.
2. Aunque todo el código es susceptible a adaptaciones según se considere necesario, en general se recomienda:
   - Emplear el cuerpo de función estándar indicado.
   - En caso de necesitar más argumentos emplear el _hashmap_.
3. Para evitar optimizaciones del compilador, es recomendable:
   - Que exista una variable que se emplee cada vez que se realizan la/s operaciones a medir. Además, de devolver el valor de la variable al final de la función. De esta manera, la variable estará sujeta a la función `doNotOptimizeAway` y no se realizarán optimizaciones ni reordenaciones con ella.
   - Emplear el _qualifier_ `volatile` en los iteradores de los bucles, para evitar que el compilador los elimine.
   - Obtener el ensamblador del código antes de realizar al ejecución y asegurarnos de que las operaciones no están siendo ignoradas u optimizadas agresivamente por el compilador.
4. Emplear la función `getAdressNode` para confirmar que las reservas de memoria se están realizando correctamente en los nodos deseados.

# Ejecuciones de pruebas
Para la ejecución de pruebas, es necesario:

1. Definir los tipos de datos a emplear en el archivo `types.h` y crear la función de pruebas en el archivo `affinity.cpp`, tal y como se indicó en las secciones anteriores.
2. Definir el archivo de argumentos deseado siguiendo el esquema de la sección anterior.
3. Compilar el código con un compilador de C++, y las liberías `-lnuma` y `-fopenmp`.
4. Ejecutar el código, indicado en primer lugar el nombre del archivo de argumentos y, en segundo lugar el nombre del archivo de salida. Un ejemplo sería, `./affinity.out args salida`.

Tras cada ejecución se obtendrá una salida como la expuesta en el siguiente código, donde se muestra en la parte superior la información de la ejecución y en la parte inferior los resultados obtenidos por cada hilo y para cada operación.

```txt
                  --Execution data--
+ Vectors data
 - Vector 0     node: 0     size: 209715200

+ Threads data
 - Thread 0     node: 0     vector: 0
 - Thread 1     node: 2     vector: 0

+ Operation data
 - Operations to be performed: read_s, write_s, read_write_s,
 - type of summary: max

                       -Results-
                               Time_Cost(s)
               | Thread 0:     6.501979872         
read_s         | Thread 1:     6.501987319         
               | Total:        6.501987319         

               | Thread 0:     40.546937634        
write_s        | Thread 1:     40.614545821        
               | Total:        40.614545821        

               | Thread 0:     6.513637034         
read_write_s   | Thread 1:     6.530190013         
               | Total:        6.530190013         
```





