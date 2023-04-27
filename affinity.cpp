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
#define L1 32768
#define L2 262144
#define L3 26214400

//#define cacheLineSize 64
#define dist 192

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
		std::vector<vectorType> priv_std_vec;
		vectorType* priv_shared_vec;
		
		//Funcion pointer
		typedef int (Thread_array::*MemFuncPtr)(vectorSize);
        MemFuncPtr function[2*nOP]; 
		string function_name[nOP];

		//Init vector
		static void initVector(vectorType* vec, const vectorSize size, const char init_val){
			for(vectorSize i = 0; i < size; i++){
				vec[i] = init_val;
			}
		}

	public:

		//Constructor
		Thread_array(){};
		void init_Private(const int threadN, Parser& parser){

			//Allocate private vector
			if(parser.get_p_data()){
				priv_vec = (vectorType*) numa_alloc_onnode(parser.get_tam_p_vector(threadN), parser.get_node_p_vector(threadN));
				priv_std_vec.resize(parser.get_tam_p_vector(threadN));
				initVector(priv_vec, parser.get_tam_p_vector(threadN), 'A');
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
				shared_vec[i] = (vectorType*) numa_alloc_onnode(parser.get_tam_s_vector(i), parser.get_node_s_vector(i));
				initVector(shared_vec[i], parser.get_tam_s_vector(i), 'A');
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
		int read_private(vectorSize size){ 
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;			
			
			long long int aux = 0;
			while(j < 333){
				i = j;
				while(i < size){
					add = priv_vec[i];
					i+=333;
					aux++;
				}
				j++;
			}
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
		int write_private(vectorSize size){
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
		int write_shared(vectorSize size){
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
int flushCache(vector<cpu_set_t> cpus_vec){

	int a;
	//Create the same number of threads as nodes
	omp_set_nested(1); 
	omp_set_num_threads(cpus_vec.size());
	omp_set_dynamic(false);
	
	#pragma omp parallel
	{
		//Set affinity to each thread to a specific node
		int node = omp_get_thread_num();
		sched_setaffinity(0, sizeof(cpus_vec[node]), &cpus_vec[node]);
		
		
		//Allocate memory in the node to flush cache
		int b;
		char *ptr = (char*)numa_alloc_onnode(2*L3, node);  
		for(long a = 0; a < 2*L3; a++){
			b += ptr[a];
		}
		numa_free(ptr, 2*L3);  

		//Add b to a to avoid compiler optimization
		#pragma omp critical
		{
			a+=b;
		}
	}
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

	//Get cpu and nodes information 
	vector<cpu_set_t> cpus_vec = getHardwareData();
	
	//Openmp clauses
	omp_set_num_threads(parser.get_num_threads());
	omp_set_dynamic(false);
	
	//1. Shared set creation
	Thread_array::init_shared(parser);

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
		int node;
		#pragma omp critical
		{
			 node = parser.get_node_thread(thread);
		}
		
		#pragma omp single
		{
			outfile << setw(50) << "-Datos-" << endl;
		}
		#pragma omp critical
        {
			sched_setaffinity(0, sizeof(cpus_vec[node]), &cpus_vec[node]);
			outfile << "-Hilo: " << thread << endl;
			outfile << " +CPU: " << sched_getcpu() << " - Node: " << parser.get_node_thread(thread) << endl;
			if(parser.get_p_data()){
				outfile << " +Tamaño del P_SET(" << parser.get_node_p_vector(thread) << "): " << parser.get_tam_p_vector(thread) << endl;
			}
			outfile << " +Datos recorridos del P_SHARED(" << parser.get_s_vector_per_thread(thread) << "): " << parser.get_num_s_elm_proc(thread) 
			<< "/" << parser.get_tam_s_vector(parser.get_s_vector_per_thread(thread))  << endl;
		}
		#pragma omp barrier	

		
		//Allocate private data for each thread
		Thread_array array;
		//Variables axiliares //TODO 
		bool condition;
		bool pData;
		#pragma omp critical
		{
			array.init_Private(thread, parser);
			pData = parser.get_p_data(); //TODO
		}

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
			flushCache(cpus_vec);
		}

		//Operations 
		while (op < nOP){

			#pragma omp critical
			{
				condition = parser.get_op_type(op);
			}
			
			//Check if the operation is active
			if(condition){

				//1.1. Private
				if(pData){
					//TODO
					i = 0;
					#pragma omp critical
					{
						tam = parser.get_tam_p_vector(thread);
					}
					//TODO revisar
					tmarkp = 0;
					while(i<ITER){
						#pragma omp single
						{	
							flushCache(cpus_vec);
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
					flushCache(cpus_vec);
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
