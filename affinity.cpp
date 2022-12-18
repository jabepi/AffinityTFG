#include <iostream>
#include <chrono>
#include <vector>
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

//Class def: contains all vector and the operations over them
//TODO en caso de crecer mucho, trasladar la clase a un fichero independiente
class Thread_array{

	private:
		//TODO cuando generalice esto es un vector de vectores
		static vector<char, na_t> shared_vec;

		//La zona privada siempre es única
		vector<char,na_t > priv_vec;

	public:
		Thread_array(long tam_priv, short node) :
			priv_vec(tam_priv,'0',na_t(node))
		{}

		static void init_shared(long tam_shared, short node){
			shared_vec = vector<char,na_t>(tam_shared, '1', na_t(node));
		}

		// Fuctions over vectors data
		int read_private(){
			int size = priv_vec.size();
			int add = 0;
			for(int i = 0; i< size; i ++){
				add += priv_vec[i];
			}
			return add;
		}

		int read_shared(){
			int size = shared_vec.size();
			int add = 0;
			for(int i = 0; i< size; i ++){
				add += shared_vec[i];
			}
			return add;
		}

		void write_private(){
			int size = priv_vec.size();
			for(int i = 0; i< size; i ++){
				priv_vec[i] = 'X';
			}
		}

		void write_shared(){
			int size = priv_vec.size();
			for(int i = 0; i< size; i ++){
				priv_vec[i] = 'X';
			}
		}

		void rw_private(){
			int size = priv_vec.size();
			for(int i = 0; i< size; i ++){
				priv_vec[i] = priv_vec[i] + 1;
			}
		}

		void rw_shared(){
			int size = priv_vec.size();
			for(int i = 0; i< size; i ++){
				priv_vec[i] = priv_vec[i] + 1;
			}
		}
};

vector<char,na_t> Thread_array::shared_vec;

//Vector of cpus by node	
vector<cpu_set_t> getHardwareData(){
	int num_nodes = numa_max_node() + 1;
	vector<cpu_set_t> cpus_vec(num_nodes);
	long psets_size = 2621440000; 
	long ssets_size = 2621440000;
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
	vector<double> w_tmarks(thread_num);
	vector<double> rw_tmarks(thread_num);

	
	cout << "-Datos- " << endl;
	#pragma omp parallel
	{
		//Variables
		int i; 

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
		
		//1. Memory reads
		auto start = std::chrono::high_resolution_clock::now();
		for (i = 0; i < ITER; i++ ){
			array.read_private();
		}
		#pragma omp barrier
		for(i = 0; i < ITER; i++){
			array.read_shared();
		}
		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
		r_tmarks[thread] = diff.count();

		flushCache(list_thr_node[thread]);
		#pragma omp barrier

		//2. Memory writes
		start = std::chrono::high_resolution_clock::now();
		for(i = 0; i < ITER; i++){
			array.write_private();
		}
		#pragma omp barrier
		for(i = 0; i < ITER; i++){
			array.write_shared();
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		w_tmarks[thread] = diff.count();
		
		flushCache(list_thr_node[thread]);
		#pragma omp barrier

		//3. Memory reads and writes
		start = std::chrono::high_resolution_clock::now();
		for(i = 0; i < ITER; i++){
			array.rw_private();
		}
		#pragma omp barrier
		for(i = 0; i < ITER; i++){
			array.rw_shared();
		}
		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		rw_tmarks[thread] = diff.count();
	}
	
	for(int j = 0; j < thread_num; j ++){
		cout << "Resultado hilo " << j << " :\n";
		cout << "T_lectura: " << r_tmarks[j] << endl;
		cout << "T_escritura: " << w_tmarks[j] << endl;
		cout << "T_lectura/escritura: " << rw_tmarks[j] << "\n\n";
	}
	
	return 0;
}

//Auxiliares
//1. Obtener CPU de cada hilo
//cout << sched_getcpu();

//2. Imprimir la lista de CPUs de cada nodo
// //Print cpus by node
// for(int i = 0; i < num_nodes; i++){
// 	cout << "Node " << i << ": ";
// 	for(int j=0; j<CPU_SETSIZE; j++){
// 		if(CPU_ISSET(j,&cpus_vec[i])){
// 			cout << j << " ";
// 		}
// 	}
// 	cout << endl;
// }