#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
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


	public:
		Thread_array(long tam_priv, short node){
			priv_vec = (vectorType*) numa_alloc_onnode(tam_priv, node);

			//inicializamos el vector 
			for(long i = 0; i < tam_priv; i++){
				priv_vec[i] = 'A';
			}
		}
			
		static void init_shared(long tam_shared, short node){
			shared_vec = (vectorType*) numa_alloc_onnode(tam_shared, node);

			//inicializamos el vector
			for(long i = 0; i < tam_shared; i++){
				shared_vec[i] = 'A';
			}
		}

		// Fuctions over vectors data
		int read_private(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;

			while(i < size){
				add += priv_vec[i];
				i++;
			}
			return add;
		}

		int read_shared(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
		
			while(i < size){
				add += shared_vec[i];
				i++;
			}
			return add;
		}

		int write_private(vectorSize size){
			volatile vectorSize i = 0;

			while(i < size){
				priv_vec[i] = 'X';
				i++;
			}

			return i;
		}

		int write_shared(vectorSize size){
			volatile vectorSize i = 0;
			
			while(i<size){
				shared_vec[i] = 'X';
				i++;
			}		

			return i;
		}

		int rw_private(vectorSize size){
			volatile char aux; 
			volatile vectorSize i = 0;

			while(i < size){
				aux = priv_vec[i];
				priv_vec[i] = aux + 1;
				i++;
			}

			return i;
		}

		int rw_shared(vectorSize size){
			volatile char aux; 
			volatile vectorSize i = 0;

			while(i<size){
				aux = shared_vec[i];
				shared_vec[i] = aux + 1;
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
int flushCache(short node){
	//TODO prevenir optimizaciones de este bucle para cuando se compile con -O3
    char *ptr = (char*)numa_alloc_onnode(L3, node);  
	int b;
	for(long a = 0; a < L3; a++){
		b += ptr[a];
	}
    numa_free(ptr, L3);  
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
	vector<double> r_tmarks(thread_num);
	vector<double> r_tmarkp(thread_num);
	vector<double> w_tmarks(thread_num);
	vector<double> w_tmarkp(thread_num);
	vector<double> rw_tmarks(thread_num);
	vector<double> rw_tmarkp(thread_num);

	
	cout << "-Datos- " << endl;
	cout << "Tamaño de los sets privados: " << psets_size << endl;
	cout << "Tamaño de los sets compartidos: " << ssets_size << endl;
	#pragma omp parallel
	{
		//Variables
		int i; 

		//Auxiliar variables
		int r1, r2;
		int w1, w2;
		int rw1, rw2;

		//Allocated thread and  vector at the same node
		int thread = omp_get_thread_num();
		Thread_array array(psets_size, list_thr_node[thread]);
		#pragma omp critical
        {
			sched_setaffinity(0, sizeof(cpus_vec[list_thr_node[thread]]), &cpus_vec[list_thr_node[thread]]);
			cout << "Hilo: " << thread << " - CPU: " << sched_getcpu() << " - Node: " << list_thr_node[thread] << endl;
		}
		#pragma omp barrier	
		if(thread == 0){
			cout << endl << "-Resultados-" << endl;
		}	
		#pragma omp barrier	
		
		//1. Memory writes
		auto start = std::chrono::high_resolution_clock::now();
		i = 0;
		while(i<ITER){
			w1 = array.write_private(ssets_size);
			i++;
		}
		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
		w_tmarkp[thread] = diff.count();
		#pragma omp barrier
		
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			w2 = array.write_shared(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
		diff = end - start;
		w_tmarks[thread] = diff.count();
		
		flushCache(list_thr_node[thread]);
		doNotOptimizeAway(w1);
		doNotOptimizeAway(w2);
		#pragma omp barrier

		//2. Memory reads
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			r1 = array.read_private(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarkp[thread] = diff.count();
		#pragma omp barrier
		
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			r2 = array.read_shared(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarks[thread] = diff.count();

		flushCache(list_thr_node[thread]);
		doNotOptimizeAway(r1);
		doNotOptimizeAway(r2);
		#pragma omp barrier

		//3. Memory reads and writes
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			rw1 = array.rw_private(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
		diff = end - start;
		rw_tmarkp[thread] = diff.count();

		#pragma omp barrier
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			rw2 = array.rw_shared(ssets_size);
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
		diff = end - start;
		rw_tmarks[thread] = diff.count();

		doNotOptimizeAway(rw1);
		doNotOptimizeAway(rw2);
	}
	
	for(int j = 0; j < thread_num; j ++){
		cout << "Resultado hilo " << j << " :\n";
		cout << setw(25) << " "  << std::left << setw(20) << "T_Datos_Privados" << std::left << setw(20) << "T_Datos_Compartidos" << endl;
		
		cout << setw(25) << "T_lectura: " << std::left << setw(20) << r_tmarkp[j] << r_tmarks[j] << endl;
		cout << setw(25) << "T_escritura: " << std::left << setw(20) << w_tmarkp[j] << w_tmarks[j] << endl;
		cout << setw(25) << "T_lectura/escritura: " << std::left << setw(20) << rw_tmarkp[j] << rw_tmarks[j] << endl;
		cout << endl;
	}
	
	return 0;
}