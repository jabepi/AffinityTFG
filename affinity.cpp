#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <math.h>
using namespace std;

#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <omp.h>
#include "CLI11.hpp"
#include "numaalloc.hpp"

//TODO borrar o poner como un argumento más
#define ITER 10
#define L1 32768
#define L2 262144
#define L3 26214400

#define cacheLineSize 64

//Types def
typedef NumaAlloc::NumaAlloc<char> na_t;
//TODO typedef vector data type
typedef char vectorType;
typedef long long int vectorSize; 

//Aux
template<typename T> 
	inline void doNotOptimizeAway(T&& datum) {
	asm volatile ("" : "+r" (datum));
}

//Class def: contains all vector and the operations over them
//TODO en caso de crecer mucho, trasladar la clase a un fichero independiente
class Thread_array{

	private:
		//TODO cuando generalice esto es un vector de vectores
		static vectorType* shared_vec;

		//La zona privada siempre es única
		vectorType* priv_vec; 

		static void initVector(vectorType* vec, const vectorSize size, const char init_val){
			for(vectorSize i = 0; i < size; i++){
				vec[i] = init_val;
			}
		}


	public:
		Thread_array(const long tam_priv, const short node){
			priv_vec = (vectorType*) numa_alloc_onnode(tam_priv, node);

			//Inicializamos el vector 
			initVector(priv_vec, tam_priv, 'A');
		}
		void delete_private(){
			numa_free(priv_vec, sizeof(vectorType));
		}
			
		static void init_shared(const long tam_shared, const short node){
			shared_vec = (vectorType*) numa_alloc_onnode(tam_shared, node);

			//Inicializamos el vector
			initVector(shared_vec, tam_shared, 'A');
		}
		static void free_shared(){
			numa_free(shared_vec, sizeof(vectorType));
		}



		// Fuctions over vectors data
		// 1. Reads
		int read_private(const vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			//Definimos el salto entre los elementos del vector
			vectorSize dist = cacheLineSize*4;
			//En función del salto en cada iteración se leerán sec elementos
			vectorSize sec = size/dist;
			
			while(j < dist){
				i = 0;
				while(i < sec){
					add = priv_vec[i*dist + j];
					i++;
				}
				j++;
			}

			return add;
		}

		int read_private2(const vectorSize size){
			volatile int add = 0;
			volatile vectorSize j = 0;

			while(j < size){
				add = priv_vec[j];
				j++;
			}

			return add;
		}

		int read_shared(const vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			//Definimos el salto entre los elementos del vector
			vectorSize dist = cacheLineSize*4;
			//En función del salto en cada iteración se leerán sec elementos
			vectorSize sec = size/dist;
			
			while(j < dist){
				i = 0;
				while(i < sec){
					add = shared_vec[i*dist + j];
					i++;
				}
				j++;
			}

			return add;
		}

		int read_shared2(const vectorSize size){
			volatile int add = 0;
			volatile vectorSize j = 0;

			while(j < size){
				add = shared_vec[j];
				j++;
			}

			return add;
		}

		// 2. Writes
		int write_private(const vectorSize size){
			volatile vectorSize i = 0;
			volatile int j = 0;
			volatile int add = 0;

			while(i < size){
				j = 0;
				while(j < 40){
					add = 'X';
					add = (add*add-add*add) + (add / 2) + (44 + add/add);
					priv_vec[i] = add;
					j++;
				}
				i++;
			}

			return i;
		}

		int write_shared(const vectorSize size){
			volatile vectorSize i = 0;
			volatile int j = 0;
			volatile int add = 0;
			
			while(i<size){
				j = 0;
				while(j < 40){
					add = 'X';
					add = (add*add-add*add) + (add / 2) + (44 + add/add);
					shared_vec[i] = add;
					j++;
				}
				i++;
			}		

			return i;
		}

		// 3. Read-Write
		int rw_private(const vectorSize size){
			volatile char aux; 
			volatile vectorSize i = 0;
			volatile int j = 0;

			while(i < size){
				j = 0;
				while(j < 40){
					aux = priv_vec[i];
					priv_vec[i] = (aux*aux-aux*aux) + (aux / 2) + (44 + aux/aux); 

					j++;
				}	
				i++;
			}
			return i;
		}

		int rw_shared(const vectorSize size){
			volatile char aux; 
			volatile vectorSize i = 0;
			volatile int j = 0;

			while(i<size){
				j = 0;
				while(j < 40){
					aux = shared_vec[i];
					shared_vec[i] = (aux*aux-aux*aux) + (aux / 2) + (44 + aux/aux); 
					j++;
				}
				i++;
			}
			return i;
		}


};

vectorType* Thread_array::shared_vec;

//Vector of cpus by node	
vector<cpu_set_t> getHardwareData(){
	int num_nodes = numa_max_node() + 1;
	vector<cpu_set_t> cpus_vec(num_nodes);
	
	//Get cpus by node	
	for(int i = 0; i < num_nodes; i++){
		struct bitmask *cpus = numa_allocate_cpumask();
		numa_node_to_cpus(i,cpus);
		CPU_ZERO(&cpus_vec[i]);
		for(int j=0; j<cpus->size; j++){
			if(numa_bitmask_isbitset(cpus,j)){
				CPU_SET(j,&cpus_vec[i]);
			}
		}
		numa_free_cpumask(cpus);
	}
	return cpus_vec;
}

//Flush all cache of a specific node
int flushCache(const short node){
	//TODO prevenir optimizaciones de este bucle para cuando se compile con -O3
    char *ptr = (char*)numa_alloc_onnode(2*L3, node);  
	int b;
	for(long a = 0; a < 2*L3; a++){
		b += ptr[a];
	}
    numa_free(ptr, 2*L3);  
	return b;
}

int main(int argc, char* argv[]){

	vector<size_t> list_thr_node;
	size_t thread_num;
	long psets_size;
	long ssets_size;
	short sset_node;

	//-------------------------------------------Temporal------------------------------------------------
	//TODO son muchas combinaciones y datos, mejor leer de un archivo y parsearlo, para que sea más legible 
	if(argc < 2){
		cout << "Introducir nodo de vector compartido." << endl;
		return 0;
	}
	//Número de hilos
	thread_num = atoi(argv[1]);
	int i = 0;
	//Nodo de cada hilo 
	while(i < thread_num){
		list_thr_node.push_back(atoi(argv[i+2]));
		i++;	
	}
	//Nodo del vector compartido
	sset_node = atoi(argv[i+2]);
	i++;
	//Tamaño de los sets privado y compartico
	ssets_size = atol(argv[i+2]);
	psets_size = ssets_size;

	//Archivo de salida 
	string salida = argv[i+3];
	ofstream outfile(salida+"Visual.txt", std::ios::app);
    if (!outfile.is_open()) {
       cout << "No se ha podido abrir el archivo " << salida+"Visual.txt" << endl;
       return 1;
    }
	//---------------------------------------------------------------------------------------------------

	//Get cpu and nodes information 
	vector<cpu_set_t> cpus_vec = getHardwareData();
	
	//Openmp clauses
	omp_set_num_threads(thread_num);
	omp_set_dynamic(false);

	//1. Shared set creation
	// TODO generalizar para n sets compartidos
	Thread_array::init_shared(ssets_size, sset_node);

	//2. Time marks vector
	double r_tmarks[thread_num];
	double r_tmarkp[thread_num];
	double r_tmarks_S[thread_num];
	double r_tmarkp_S[thread_num];
	// double w_tmarks[thread_num];
	// double w_tmarkp[thread_num];
	// double rw_tmarks[thread_num];
	// double rw_tmarkp[thread_num];
	

	#pragma omp parallel
	{
		//Variables
		int i; 

		//Auxiliar variables
		int r1, r2;
		int r1_S, r2_S;
		// int w1, w2;
		// int rw1, rw2;

		//Allocated thread and  vector at the same node
		int thread = omp_get_thread_num();
		Thread_array array(psets_size, list_thr_node[thread]);
		#pragma omp critical
        {
			sched_setaffinity(0, sizeof(cpus_vec[list_thr_node[thread]]), &cpus_vec[list_thr_node[thread]]);
			outfile << "Hilo: " << thread << " - CPU: " << sched_getcpu() << " - Node: " << list_thr_node[thread] << endl;
		}
		#pragma omp barrier	
		#pragma omp single
		{
			outfile << endl << "-Resultados-" << endl;

			cout << "Tamaño privado:" <<  psets_size << endl;
			cout << "Tamaño compartido:" <<  ssets_size << endl;
		}
		flushCache(list_thr_node[thread]);	
		
		
		// //1. Memory writes
		// i = 0;
		// #pragma omp barrier	
		// auto start = std::chrono::high_resolution_clock::now();
		// while(i<ITER){
		// 	w1 = array.write_private(ssets_size);
		// 	i++;
		// }
		// auto end = std::chrono::high_resolution_clock::now();

        // std::chrono::duration<double> diff = end - start;
		// w_tmarkp[thread] = diff.count()/ITER;
		// #pragma omp single
		// {
		// 	cout << "Terminadas las escrituras privadas" << endl;
		// }
		
		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	w2 = array.write_shared(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
		// diff = end - start;
		// w_tmarks[thread] = diff.count()/ITER;
		
		// flushCache(list_thr_node[thread]);
		// doNotOptimizeAway(w1);
		// doNotOptimizeAway(w2);
		// #pragma omp single
		// {
		// 	cout << "Terminadas las escrituras compartidas" << endl;
		// }

		//2. Memory reads
		i=0;
		#pragma omp barrier
		auto start = std::chrono::high_resolution_clock::now();
		while(i < ITER){
			r1 = array.read_private(ssets_size);
			i++;
		}
		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
		r_tmarkp[thread] = diff.count()/ITER;
		#pragma omp single
		{
			cout << "Terminadas las lecturas privadas" << endl;
		}
		
		//Lectura secuencial 
		flushCache(list_thr_node[thread]);
		i=0;
		#pragma omp barrier
		start = std::chrono::high_resolution_clock::now();
		while(i < ITER){
			r1_S = array.read_private2(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarkp_S[thread] = diff.count()/ITER;
		#pragma omp single
		{
			cout << "Terminadas las lecturas privadas secuenciales" << endl;
		}

		i=0;
		#pragma omp barrier
		start = std::chrono::high_resolution_clock::now();
		while(i < ITER){
			r2 = array.read_shared(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarks[thread] = diff.count()/ITER;
		#pragma omp single
		{
			cout << "Terminadas las lecturas compartidas" << endl;
		}

		i=0;
		flushCache(list_thr_node[thread]);
		#pragma omp barrier
		start = std::chrono::high_resolution_clock::now();
		while(i < ITER){
			r2_S = array.read_shared2(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarks_S[thread] = diff.count()/ITER;
		#pragma omp single
		{
			cout << "Terminadas las lecturas compartidas secuenciales" << endl;
		}
		
		flushCache(list_thr_node[thread]);
		doNotOptimizeAway(r1);
		doNotOptimizeAway(r2);
		doNotOptimizeAway(r1_S);
		doNotOptimizeAway(r2_S);
		
		

		// //3. Memory reads and writes
		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	rw1 = array.rw_private(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
		// diff = end - start;
		// rw_tmarkp[thread] = diff.count()/ITER;
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas y escrituras privadas" << endl;
		// }

		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	rw2 = array.rw_shared(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
		// diff = end - start;
		// rw_tmarks[thread] = diff.count()/ITER;
		// doNotOptimizeAway(rw1);
		// doNotOptimizeAway(rw2);
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas y escrituras compartidas" << endl;
		// }

		//Destruimos el objeto 
		array.delete_private();
	}
	cout << endl;
	Thread_array::free_shared();
	
	//Escribimos los resultados de forma visual
	outfile << "-Datos- " << endl;
	outfile << "Tamaño de los sets privados: " << psets_size << endl;
	outfile << "Tamaño de los sets compartidos: " << ssets_size << endl;
	for(int j = 0; j < thread_num; j ++){

		outfile << "Resultado hilo " << j << " :\n";
		outfile << setw(25) << " "  << std::left << setw(20) << "T_Datos_Privados" << std::left << setw(20) << "T_Datos_Compartidos" << endl;
		
		outfile << setw(25) << "T_lectura: " << std::left << setw(20) << r_tmarkp[j] << r_tmarks[j] << endl;
		outfile << setw(25) << "T_lectura_S: " << std::left << setw(20) << r_tmarkp_S[j] << r_tmarks_S[j] << endl;
		//outfile << setw(25) << "T_escritura: " << std::left << setw(20) << w_tmarkp[j] << w_tmarks[j] << endl;
		// outfile << setw(25) << "T_lectura/escritura: " << std::left << setw(20) << rw_tmarkp[j] << rw_tmarks[j] << endl;
		outfile << endl;
	}
	outfile.close();

	//Escribir los resultados de forma procesable
	outfile.open(salida+"Proc.txt", std::ios::app);
    if (!outfile.is_open()) {
       cout << "No se ha podido abrir el archivo " << salida+"Proc.txt" << endl;
       return 1;
    }
	outfile << "privados:" << psets_size << endl;
	outfile << "compartidos:" << ssets_size << endl;
	for(int j = 0; j < thread_num; j ++){	
		// outfile << r_tmarkp[j] << " " << r_tmarks[j] << " " << w_tmarkp[j] << " " << w_tmarks[j] << " " << rw_tmarkp[j] << " " << rw_tmarks[j] << endl;
		outfile << r_tmarkp[j] << " " << r_tmarks[j] << " " << endl;
	}
	outfile.close();
	
	return 0;
}