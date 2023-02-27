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

//Aux
template<typename T> 
	inline void doNotOptimizeAway(T&& datum) {
	asm volatile ("" : "+r" (datum));
}

//Class def: contains all vector and the operations over them
//TODO en caso de crecer mucho, trasladar la clase a un fichero independiente
class Thread_array{

	private:
		//La zona privada siempre es única
		vector<char,na_t > priv_vec;

	public:
		Thread_array(long tam_priv, short node) :
			priv_vec(tam_priv,'0',na_t(node))
		{}

		// Fuctions over vectors data
		int read_private(){
			long size = priv_vec.size();
			volatile int add = 0;
			int i = 0;
			
			while(i < size){
				add += priv_vec[i];
				i++;
			}
			return add;
		}
		int write_private(){

			long size = priv_vec.size();
			volatile int i = 0;
			
			while(i < size){
				priv_vec[i] = 'X';
				i++;
			}

			return i;
		}
		int rw_private(){
			
			long size = priv_vec.size();
			volatile char aux; 
			volatile int i = 0;

			while(i < size){
				aux = priv_vec[i];
				priv_vec[i] = aux + 1;
				i++;
			}

			return i;
		}
};

//Vector of cpus by node	
vector<cpu_set_t> getHardwareData(){
	int num_nodes = numa_max_node() + 1;
	vector<cpu_set_t> cpus_vec(num_nodes);
	long psets_size = 2621440000; 
	short sset_node = 0;

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
	psets_size = atol(argv[i+2]);
	//---------------------------------------------------------------------------------------------------

	//Get cpu and nodes information 
	vector<cpu_set_t> cpus_vec = getHardwareData();
	
	//Openmp clauses
	omp_set_num_threads(thread_num);
	omp_set_dynamic(false);

	//1. Time marks vector
	vector<double> r_tmarkp(thread_num);
	vector<double> w_tmarkp(thread_num);
	vector<double> rw_tmarkp(thread_num);

	cout << "-Datos- " << endl;
	cout << "Tamaño de los sets: " << psets_size << endl;
	#pragma omp parallel
	{
		//Auxiliar variables
		int i; 
		int r1, w1, rw1;

		//Allocate thread at node N and private vector at node 0 
		int thread = omp_get_thread_num();
		Thread_array array(psets_size, 0);
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
			w1 = array.write_private();
			i++;
		}
		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
		w_tmarkp[thread] = diff.count();
		#pragma omp barrier
		
		flushCache(list_thr_node[thread]);
		doNotOptimizeAway(w1);
		#pragma omp barrier

		//2. Memory reads
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			r1 = array.read_private();
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		r_tmarkp[thread] = diff.count();
		#pragma omp barrier

		flushCache(list_thr_node[thread]);
		doNotOptimizeAway(r1);
		#pragma omp barrier

		//3. Memory reads and writes
		start = std::chrono::high_resolution_clock::now();
		i=0;
		while(i < ITER){
			rw1 = array.rw_private();
			i++;
		}
		end = std::chrono::high_resolution_clock::now();
		diff = end - start;
		rw_tmarkp[thread] = diff.count();
		doNotOptimizeAway(rw1);
	}
	
	for(int j = 0; j < thread_num; j ++){
		cout << "Resultado hilo " << j << " :\n";
		cout << setw(25) << " "  << std::left << setw(20) << "T_Datos_Privados" << std::left << setw(20) << endl;
		
		cout << setw(25) << "T_lectura: " << std::left << setw(20) << r_tmarkp[j] << endl;
		cout << setw(25) << "T_escritura: " << std::left << setw(20) << w_tmarkp[j] << endl;
		cout << setw(25) << "T_lectura/escritura: " << std::left << setw(20) << rw_tmarkp[j] << endl;
		cout << endl;
	}
	
	return 0;
}

