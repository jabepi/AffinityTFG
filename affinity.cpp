//Standard libraries
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <math.h>
using namespace std;

//Affinity and parallel libraries
#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <omp.h>

//Custom libraries
#include "CLI11.hpp"
#include "numaalloc.hpp"
#include "parser.hpp"
#include "types.h"

//TODO borrar o poner como un argumento más
#define nOP 3
#define ITER 1
#define INTVALVEC 'A'
#define L1 32768
#define L2 262144
#define L3 26214400

//TODO IMPORTANTE Simplificar con numa_run_on_node  
//#define cacheLineSize 64
#define dist 333

//Avoid compiler optimization
template<typename T> 
	inline void doNotOptimizeAway(T&& datum) {
	asm volatile ("" : "+r" (datum));
}

//Funciones auxioliares 
int getAdressNode(void* pointer){
	int node;
	if (get_mempolicy(&node, NULL, 0, pointer, MPOL_F_NODE | MPOL_F_ADDR) == -1) {
		perror("Error al obtener la política de memoria");
		return -1;
	}
	printf("La dirección de memoria %p pertenece al nodo NUMA %d.\n", (void *)pointer, node);
	return node;
}


//Class def: contains all vector and the operations over them
class Thread_array{

	private:
		static vectorType** shared_vec;
		
		//La zona privada siempre es única
		vectorType* priv_vec; 
		vectorType* priv_shared_vec;
		
		//Funcion pointer
		typedef int (Thread_array::*MemFuncPtr)(vectorSize);
        MemFuncPtr function[2*nOP]; 
		string function_name[nOP];

	public:

		//Constructor
		Thread_array(){};
		void init_Private(const int threadN, Parser& parser){

			//Allocate private vector
			if(parser.get_p_data()){
				vectorSize size = parser.get_tam_p_vector(threadN);
				priv_vec = (vectorType*) numa_alloc_onnode(size, parser.get_node_p_vector(threadN));

				for(vectorSize i = 0; i < size; i++){
					priv_vec[i] = INTVALVEC;
				}

				for(vectorSize i = 0; i < size; i++){
					priv_vec[i] = INTVALVEC;
				}


			}

			//Get pointer to shared vector
			priv_shared_vec = shared_vec[parser.get_s_vector_per_thread(threadN)];

			//Init function pointers
			function[0] = &Thread_array::read_private; 
			function[1] = &Thread_array::read_shared;
			function[2] = &Thread_array::write_private;
			function[3] = &Thread_array::write_shared;
			function[4] = &Thread_array::rw_private;
			function[5] = &Thread_array::rw_shared; 

			//Function names
			function_name[0] = "Reads";
			function_name[1] = "Writes";
			function_name[2] =  "Reads/Writes";
		}
		void delete_private(){
			numa_free(priv_vec, sizeof(vectorType));
		}
		
		//Init shared data
		static void init_shared(Parser& parser){

			shared_vec = (vectorType**) malloc(parser.get_num_s_vector()*sizeof(vectorType*));
			for(int i = 0; i < parser.get_num_s_vector(); i++){
				vectorSize size = parser.get_tam_p_vector(i);
				shared_vec[i] = (vectorType*) numa_alloc_onnode(size, parser.get_node_s_vector(i));
				
				for(vectorSize j = 0; j < size; j++){
					shared_vec[i][j] = INTVALVEC;
				}
			}
		}
		static void free_shared(Parser& parser){
			for(int i = 0; i < parser.get_num_s_vector(); i++){
				numa_free(shared_vec[i], sizeof(vectorType));
			}
			free(shared_vec);
		}

		//Call functions
		int call_function(int index, vectorSize size) {
            return (this->*function[index])(size);
        }
		string get_function_name(int index){
			cout << function_name[0] << endl;
			return function_name[index];
		}

		// Fuctions over vectors data
		// 1. Reads
		int read_private(const vectorSize size){ 
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;			
			long long int aux = 0;
			cout << size << endl;
			printf("%d-%p\n", omp_get_thread_num(), priv_vec);
			while(j < dist){
				i = j;
				while(i < size){
					// add = priv_vec[i];
					
					i+=dist;
					aux ++;
				}
				j++;
			}
			printf("%lld\n", aux);
			return add;
		}

		int read_shared(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			while(j < dist){
				i = j;
				while(i < size){
					add = priv_shared_vec[i]; 
					i+=dist;
				}
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
					priv_shared_vec[i] = add;
					j++;
				}
				i++;
			}		

			return i;
		}

		// 3. Read-Write
		int rw_private(vectorSize size){
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
		int rw_shared(vectorSize size){
			volatile char aux; 
			volatile vectorSize i = 0;
			volatile int j = 0;

			
			while(i<size){
				j = 0;
				while(j < 40){
					aux = priv_shared_vec[i];
					priv_shared_vec[i] = (aux*aux-aux*aux) + (aux / 2) + (44 + aux/aux); 
					j++;
				}
				i++;
			}
			return i;
		}
};

vectorType** Thread_array::shared_vec;

//Vector of cpus by node	
vector<cpu_set_t> getHardwareData(){
	int num_nodes = numa_max_node() + 1;
	vector<cpu_set_t> cpus_vec(num_nodes);
	
	//Get cpus by node	
	for(int i = 0; i < num_nodes; i++){
		struct bitmask *cpus = numa_allocate_cpumask();
		numa_node_to_cpus(i,cpus);
		CPU_ZERO(&cpus_vec[i]);
		for(long unsigned int j=0; j<cpus->size; j++){
			if(numa_bitmask_isbitset(cpus,j)){
				CPU_SET(j,&cpus_vec[i]);
			}
		}
		numa_free_cpumask(cpus);
	}
	return cpus_vec;
}

//Flush all cache of all nodes
int flushCache(int node){

	int a = 0;	
	char *ptr = (char *)numa_alloc_onnode(2 * L3, node);
	
	// Allocate memory in the node to flush cache
	int b = 0;
	while (b < 3*L3){
		a += ptr[b];
		b++;
	}
	numa_free(ptr, 3*L3);  
	return a;
}

int main(int argc, char* argv[]){

	if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <output>" << std::endl;
        return 1;
    }
	string inputFile = argv[1];
    string outputFile = argv[2];

	//Parse arguments in input file
	Parser parser;
    if(!parser.parse_input_file(inputFile)){
		cout << "Error parsing input file" << endl;
		return 1;
	}

	parser.print(); //TODO: borrar
	
	//Open output file
	ofstream outfile(outputFile+"Visual.txt", std::ios::app);
    if (!outfile.is_open()) {
       cout << "Error openning file: " << outputFile+"Visual.txt" << endl;
       return 1;
    }
	
	//Auxiliar variables
	int half = ceil(parser.get_num_threads()/2);
	int increment = 0;
	
	//Openmp clauses
	omp_set_num_threads(parser.get_num_threads());
	omp_set_dynamic(false);

	//Variables for cache flush 
	int num_nodes = numa_max_node() + 1;
    std::vector<int> primer_hilo_por_nodo(num_nodes, -1);
	
	//Shared set creation
	Thread_array::init_shared(parser);

	//Get parser information
	bool pData = parser.get_p_data();

	#pragma omp parallel
	{
		//Variables
		int i; 
		int op = 0, k = 0;

		//Auxiliar variables
		int result, result2;
		vectorSize tam;

		//Time marks
		chrono::high_resolution_clock::time_point start, end;
		std::chrono::duration<double> diff;
		double tmarkp;
		double tmarks;

		//Allocate thread on node
		int thread = omp_get_thread_num();
		int node = parser.get_node_thread(thread);
		numa_run_on_node(node);
		node = numa_node_of_cpu(sched_getcpu());
		
		#pragma omp single
		{
			outfile << setw(50) << "-Datos-" << endl;
		}
		#pragma omp critical
        {
			//Print thread information
			outfile << "-Hilo: " << thread << endl;
			outfile << " +CPU: " << sched_getcpu() << " - Node: " << node << endl;
			if(pData){
				outfile << " +Tamaño del P_SET(" << parser.get_node_p_vector(thread) << "): " << parser.get_tam_p_vector(thread) << endl;
			}
			outfile << " +Datos recorridos del P_SHARED(" << parser.get_s_vector_per_thread(thread) << "): " << parser.get_num_s_elm_proc(thread) 
			<< "/" << parser.get_tam_s_vector(parser.get_s_vector_per_thread(thread))  << endl;
		}
		#pragma omp barrier	
        
		//Configure cache flush
        #pragma omp critical
        {
            if (primer_hilo_por_nodo[node] == -1) {
                primer_hilo_por_nodo[node] = thread;
            }
        }
        #pragma omp barrier

		//Allocate private data for each thread
		Thread_array array;
		array.init_Private(thread, parser);

		//Variables axiliares //TODO 
		bool condition;

		#pragma omp single
		{	
			outfile << endl;
			outfile << setw(50) << "-Resultados-" << endl;
			if(parser.get_p_data()){
				outfile << setw(25) << " "  << left << setw(20) << "T_Datos_Privados" ;
			}else{
				outfile << setw(25) << " ";
			}
			outfile << left << setw(20) << "T_Datos_Compartidos" << endl;
		}

		//Operations 
		while (op < nOP){

			condition = parser.get_op_type(op);
			
			//Check if the operation is active
			if(condition){

				//1.1. Private
				if(pData){
					
					
					i = 0;
					tam = parser.get_tam_p_vector(thread);
					tmarkp = 0;
					while(i<ITER){
						
						//Flush cache
						if (thread == primer_hilo_por_nodo[node]) {
							flushCache(node);
						}

						#pragma omp barrier	
						start = std::chrono::high_resolution_clock::now();
						
						// result = array.call_function(k, tam);z
						array.read_private(tam);
						
						end = std::chrono::high_resolution_clock::now();
						diff = end - start;
						tmarkp += diff.count();
						i++;
					}
					tmarkp /= ITER;
					doNotOptimizeAway(result);
				}
				
				// //1.2. Shared
				i=0;
				#pragma omp critical
				{
					tam = parser.get_num_s_elm_proc(thread);
				}

				#pragma omp barrier
				start = std::chrono::high_resolution_clock::now();
				while(i < ITER){
					result2 = array.call_function(k+1, tam);
					i++;
				}
				end = std::chrono::high_resolution_clock::now();
				diff = end - start;
				tmarks = diff.count()/ITER;
				doNotOptimizeAway(result2);

				//Print results
				#pragma omp critical
				{	
					if(increment == half){
						outfile << setw(10)  <<   array.get_function_name(op) << "| "  ;
					}else{
						outfile << setw(10)  <<   " " << "| "  ;
					}
					
					outfile  << "Hilo " << thread << left << setw(7) << ":"  ;
					if(pData){
						 outfile << fixed << setprecision(9) << left << setw(20) << tmarkp;
					}
					outfile << fixed << setprecision(9) << left << setw(20) << tmarks << endl;
					increment++;
				}
				#pragma omp barrier
				
				#pragma omp single
				{
					// flushCache(cpus_vec);
					outfile << endl;
				}
				
			}
			op+=1;
			k+=2;
			increment = 0;
		}

		//Destruimos el objeto 
		array.delete_private();
	}
	cout << endl;
	Thread_array::free_shared(parser);
	outfile.close();

	return 0;
}
