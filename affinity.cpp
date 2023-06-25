//Standard libraries
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <math.h>
#include <unordered_map>
#include <thread>
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

//Constants
#define INTVALVEC 'A'
#define L1 32768
#define L2 262144
#define L3 26214400
#define dist 192 

//Avoid compiler optimization
template<typename T> 
	inline void doNotOptimizeAway(T&& datum) {
	asm volatile ("" : "+r" (datum));
}

//Auxiliar function 
// 1. getAdressNode: Get the node associated to a pointer
int getAdressNode(void* pointer){
	int node;
	if (get_mempolicy(&node, NULL, 0, pointer, MPOL_F_NODE | MPOL_F_ADDR) == -1) {
		perror("Error getting mempolicy");
		return -1;
	}
	printf("The address %p belong to the NUMA node %d.\n", (void *)pointer, node);
	return node;
}

class Thread_array{

	private:
		//Vector of operations
		static vectorType** shared_vec;
		vectorType* priv_shared_vec;

		//Extra arguments 
		unordered_map<string, string> arg;
		
		//Funcion pointer
		typedef int (Thread_array::*MemFuncPtr)(vectorSize);
		unordered_map<string, MemFuncPtr> functMap;
	
	public:
		//Constructor
		Thread_array(){};

		//Function control
		unordered_map<std::string, MemFuncPtr> setFunctionsPointers(){
			//Funciones sobre vectores	
			functMap["read"] = &Thread_array::read;
			functMap["write"] = &Thread_array::write;
			functMap["read_write"] = &Thread_array::read_write;
			functMap["write_read"] = &Thread_array::write_read;
			functMap["read_s"] = &Thread_array::read_s;
			functMap["write_s"] = &Thread_array::write_s;
			functMap["read_write_s"] = &Thread_array::read_write_s;
			functMap["read_h"] = &Thread_array::read_h;
			functMap["write_h"] = &Thread_array::write_h;
			functMap["read_write_h"] = &Thread_array::read_write_h;
			return functMap;
		}
		bool checkFunctions(vector <string> functions){
			setFunctionsPointers();
			for(int i = 0; i < functions.size(); i++){
				if(functMap.find(functions[i]) == functMap.end()){
					cout << "Error: Function " << functions[i] << " not found" << endl;
					return false;
				}
			}
			return true;
		}

		//Init private data
		void init_Private(const int threadN, Parser& parser){
			//Get pointer to shared vector
			priv_shared_vec = shared_vec[parser.get_s_vector_per_thread(threadN)];

			//Get extra arguments
			arg = parser.get_args_extra();
			
			//Init function pointers
			setFunctionsPointers();
		}		
		
		//Init vectors
		static void init_vectors(Parser& parser){

			shared_vec = (vectorType**) malloc(parser.get_num_s_vector()*sizeof(vectorType*));
			for(int i = 0; i < parser.get_num_s_vector(); i++){
				vectorSize size = parser.get_tam_s_vector(i);
				shared_vec[i] = (vectorType*) numa_alloc_onnode(size*sizeof(vectorType), parser.get_node_s_vector(i));
				
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
		int call_function(string func, vectorSize size) {
            return (this->*functMap[func])(size);
        }

		// ------------1. Standar access------------
		//1.1 Read
		int read(vectorSize size){
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
		// 1.2. Writes
		int write(const vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;
			
			while(j < dist){
				i = j;
				while(i < size){
					add = 'X';
			 		priv_shared_vec[i] = add;
					i+=dist;
				}
				j++;
			}
			return add;
		}
		// 1.3. Read-Write
		int read_write(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			while(j < dist){
				i = j;
				while(i < size){	
					add = priv_shared_vec[i];
					priv_shared_vec[i] = add + 1; 
					i+=dist;
				}
				j++;
			}
			return add;
		}
		// 1.4 Write-Read
		int write_read(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			while(j < dist){
				i = j;

				while(i < size){	
					priv_shared_vec[i] = 1; 
					add = priv_shared_vec[i];
					i+=dist;
				}
				j++;
			}
			return add;
		}


		// -------------2. half-access ----------
		//2.1 Read 
		int read_h(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			if(omp_get_thread_num() == 1){
				size = size/2;
			}
			while(j < dist){
				i = j;

				#pragma noprefetch priv_shared_vec
				while(i < size){
					add = priv_shared_vec[i]; 
					i+=dist;
				}
				j++;
			}
			return add;
		}
		//2.2. Writes
		int write_h(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;
			
			if(omp_get_thread_num() == 1){
				size = size/2;
			}
			while(j < dist){
				i = j;
				while(i < size){
					add = 'X';
			 		priv_shared_vec[i] = add;
					i+=dist;
				}
				j++;
			}
			return add;
		}
		//2.3. Read-Write
		int read_write_h(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			if(omp_get_thread_num() == 1){
				size = size/2;
			}
			while(j < dist){
				i = j;
				while(i < size){	
					add = priv_shared_vec[i];
					priv_shared_vec[i] = add + 1; 
					i+=dist;
				}
				j++;
			}
			return add;
		}

		// ----------3. Delay access (sleep)----------
		//3.1 Read 
		int read_s(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;
			
			int sleep = stoi(arg["sleep"]);
			if(omp_get_thread_num() == 1){
				this_thread::sleep_for(chrono::milliseconds(sleep));
			}
			while(j < dist){
				i = j;

				#pragma noprefetch priv_shared_vec
				while(i < size){
					add = priv_shared_vec[i]; 
					i+=dist;
				}
				j++;
			}
			return add;
		}
		//3.2. Writes
		int write_s(const vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;
			
			int sleep = stoi(arg["sleep"]);
			if(omp_get_thread_num() == 1){
				this_thread::sleep_for(chrono::milliseconds(sleep));
			}
			while(j < dist){
				i = j;
				while(i < size){
					add = 'X';
			 		priv_shared_vec[i] = add;
					i+=dist;
				}
				j++;
			}
			return add;
		}
		//3.3. Read-Write
		int read_write_s(vectorSize size){
			vectorSize i = 0; 
			volatile int add = 0;
			volatile vectorSize j = 0;

			int sleep = stoi(arg["sleep"]);
			if(omp_get_thread_num() == 1){
				this_thread::sleep_for(chrono::milliseconds(sleep));
			}
			while(j < dist){
				i = j;
				while(i < size){	
					add = priv_shared_vec[i];
					priv_shared_vec[i] = add + 1; 
					i+=dist;
				}
				j++;
			}
			return add;
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
	char *ptr = (char *)numa_alloc_onnode(3 * L3, node);
	
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
		return 1;
	}
	
	//Check if functions are correct
	Thread_array* array = new Thread_array();
	vector<string> opList = parser.get_op_list();
	if (!array->checkFunctions(opList)) {
		delete array;
		return 1;
	}
	delete array; 
	
	//Open output file
	ofstream outfile(outputFile+"Visual.txt", std::ios::app);
    if (!outfile.is_open()) {
       cout << "Error openning file: " << outputFile+"Visual.txt" << endl;
       return 1;
    }
	parser.print(outfile);
	
	//Openmp clauses
	omp_set_num_threads(parser.get_num_threads());
	omp_set_dynamic(false);

	//Vector initialization
	Thread_array::init_vectors(parser);

	//Shared variables
	//1. Impresion 
	int half = ceil(parser.get_num_threads()/2);
	int increment = 0;
	//2. Time marks
	double tmarkT=0;
	//3. Cache flush
	int num_nodes = numa_max_node() + 1;
    std::vector<int> primer_hilo_por_nodo(num_nodes, -1);
	//4. Hardware data
	vector<cpu_set_t> cpus_vec = getHardwareData();
	//5. Number of repetitions
	int num_iter = parser.get_num_iter();
	//6. Type of sumary 
	string summaryType = parser.get_summary_type();
	//7. Speed up operations 
	vector<string> speedUpCalcList = parser.get_speedup_calc();
	unordered_map<string, double> speedUpDataT;
	
	#pragma omp parallel
	{
		//Variables
		int i, result;

		//Speed up data per thread
		unordered_map<string, double> speedUpData;
		for (const string& op : speedUpCalcList) {
			auto pos = op.find("/");
			string op1 = op.substr(0, pos);
			string op2 = op.substr(pos + 1, op.size() - pos - 1);
			speedUpData[op1] = 0;
			speedUpData[op2] = 0;
			speedUpData[op] = 0;
		}

		//Time marks
		chrono::high_resolution_clock::time_point start, end;
		std::chrono::duration<double> diff;
		double tmarks;

		//Allocate thread on node
		int thread = omp_get_thread_num();
		int node = parser.get_node_thread(thread);
		#pragma omp critical
		{
			sched_setaffinity(0, sizeof(cpus_vec[node]), &cpus_vec[node]);
		}
        
		//Configure cache flush
        #pragma omp critical
        {
            if (primer_hilo_por_nodo[node] == -1) {
                primer_hilo_por_nodo[node] = thread;
            }
        }
        #pragma omp barrier

		//Allocate private data for each thread
		int vectorID = parser.get_s_vector_per_thread(thread);
		vectorSize tam = parser.get_tam_s_vector(vectorID);
		Thread_array array;
		array.init_Private(thread, parser);

		#pragma omp single
		{	
			outfile << setw(23) << " " << "-Results-" << endl;
			outfile << setw(31) << " " << "Time_Cost(s)" << endl;
		}

		//Call all test functions
		for (const string& op : opList) {

			//Time marks
			#pragma omp single
			{
				tmarkT=0;
			}
			tmarks = 0;
			
			i=0;
			while(i < num_iter){
				//Flush cache
				if (thread == primer_hilo_por_nodo[node]) {
					flushCache(node);
				}
				#pragma omp barrier

				//Function call
				start = std::chrono::high_resolution_clock::now();
				result = array.call_function(op, tam);
				end = std::chrono::high_resolution_clock::now();
				diff = end - start;

				tmarks += diff.count();
				i++;
			}
			tmarks /= num_iter;
			doNotOptimizeAway(result);

			//Print results
			#pragma omp critical
			{	
				if(increment == half){
					outfile << setw(15)  << op << "|";
				}else{
					outfile << setw(15)  << " " << "|";
				}
				outfile  << left << setw(15) << (" Thread " + to_string(thread)  + ":");
				outfile  << fixed << setprecision(9) << left << setw(20) << tmarks << endl;
				
				//Summary 
				if(summaryType == "max")
					tmarkT = max(tmarkT, tmarks);
				else if(summaryType == "min")
					tmarkT = min(tmarkT, tmarks);
				else if(summaryType == "sum")
					tmarkT += tmarks;
				increment++;
			}
			#pragma omp barrier
			
			#pragma omp single
			{
				outfile << setw(15)  <<   " " << "|";
				outfile << left << setw(15) << " Total:";
				outfile << fixed << setprecision(9) << left << setw(20) << tmarkT << endl;
				outfile << endl;

			}

			//Speed up data
			auto iter = speedUpData.find(op);
			//If op is an operation used to calculate speed up
			if (iter != speedUpData.end()) {
				speedUpData[op] = tmarks;
				#pragma omp single
				{
					speedUpDataT[op] = tmarks;
				} 
			}
			increment = 0;	
		}
		
		//Print speed up data
		if(!speedUpCalcList.empty()){
			#pragma omp barrier
			#pragma omp single
			{
				outfile << setw(23) << " " << "-Speedup-" << endl;
			}
			#pragma omp barrier

			//For each speed up operation
			for( const string& op : speedUpCalcList) {

				//Get the operands of the operation
				auto pos = op.find("/");
				string op1 = op.substr(0, pos);
				string op2 = op.substr(pos + 1, op.size() - pos - 1);

				//Each thread calculate its speedup
				#pragma omp critical
				{	
					if(increment == half){
						outfile << setw(15)  << op << "|";
					}else{
						outfile << setw(15)  << " " << "|";
					}
					increment++;
						
					outfile  << left << setw(15) << (" Thread " + to_string(thread)  + ":");
					double divisionResult = 0.0;
					if (speedUpData[op2] != 0) {
						divisionResult = speedUpData[op1] / speedUpData[op2];
					}
					outfile  << fixed << setprecision(9) << left << setw(20) << divisionResult << endl;
				}
				#pragma omp barrier

				//A single thread calculate the total speedup
				#pragma omp single
				{
					outfile << setw(15)  <<   " " << "|";
					outfile << left << setw(15) << " Total:";
					double divisionResult = 0.0;
					if (speedUpDataT[op2] != 0) {
						divisionResult = speedUpDataT[op1] / speedUpDataT[op2];
					}
					outfile << fixed << setprecision(9) << left << setw(20) << divisionResult << endl;
					outfile << endl;
				}
				#pragma omp barrier
				increment = 0;
			}
		}
	}
	Thread_array::free_shared(parser);
	outfile << endl;
	outfile.close();
	return 0;		
}