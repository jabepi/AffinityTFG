#include <iostream>
#include <chrono>
#include <vector>
using namespace std;



#include <array>
#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <omp.h>
#include "CLI11.hpp"
#include "numaalloc.hpp"

//Function declaration
//TODO add parser

//Global variables
size_t thread_num   = 2;
vector<size_t> list_thr_node {0, 0};
long psets_size = 2621440000; //100 veces el tamaño de la L3
long ssets_size = 2621440000;
short sset_node = 0;
vector<size_t> list_ssets_thread {0, 0};

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


int main(int argc, char* argv[]){
	int node = 0;  // Allocate memory on node 0
    void *ptr = numa_alloc_onnode(1024, node);  // Allocate 1KB of memory on node 0
    std::cout << "Allocated memory at address: " << ptr << std::endl;
    numa_free(ptr, 1024);  // Free the allocated memory
    return 0;
	
}
