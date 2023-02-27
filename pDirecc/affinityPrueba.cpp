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

	// vector<size_t> list_thr_node;
	// size_t thread_num;
	// long psets_size;
	// long ssets_size;
	// short sset_node;

	
    
    


	const size_t size = 1024*1024*1024;  // 1 GB

    // Allocate memory on NUMA node 0
    int* arr = static_cast<int*>(numa_alloc_onnode(size, 0));
    if (arr == nullptr) {
        std::cerr << "Error allocating memory on NUMA node 0" << std::endl;
        return 1;
    }

    // Get physical address of memory block
    const std::string command = "numactl --physcpubind=0 --membind=0 addr2line -e /proc/self/exe " + std::to_string(reinterpret_cast<std::uintptr_t>(arr));
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        std::cerr << "Error executing numactl command" << std::endl;
        numa_free(arr, size);
        return 1;
    }
    char buf[256];
    std::fgets(buf, sizeof(buf), pipe);
    pclose(pipe);
    const std::uintptr_t physical_addr = std::strtoull(buf, nullptr, 16);

    std::cout << "Virtual address: " << arr << std::endl;
    std::cout << "Physical address: " << physical_addr << std::endl;

    // Use the allocated memory here

    // Free the allocated memory
    numa_free(arr, size);

    return 0;
}
