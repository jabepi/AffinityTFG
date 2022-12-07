#include <iostream>
#include <vector>
using namespace std;

#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <omp.h>
#include "CLI11.hpp"
#include "numaalloc.hpp"

//Function declaration
//TODO add parser

//Global variables
size_t thread_num   = 3;
std::vector<size_t> list_thr_node {0, 1};
long psets_size = 10;
long ssets_size = 10;
short sset_node = 0;
std::vector<size_t> list_ssets_thread {0, 0};

//Types def
typedef NumaAlloc::NumaAlloc<char> na_t;

//Class def
class Thread_array{

	private:
		static vector<char,na_t> *shared_vec;
		vector<char,na_t > *priv_vec;

	public: 
		Thread_array(int tam_priv, short node){
			priv_vec = new vector<char,na_t>(tam_priv,'0',na_t(node));
		}

		static void initShared(int tam_shared, short node){
			shared_vec = new vector<char,na_t>(tam_shared,'0',na_t(node));
		}
		void static prueba(){
			cout << (*shared_vec)[0]; 
		}

		//TODO función para ejecutar parte compartida

};	
vector<char,na_t>* Thread_array::shared_vec;

export OMP_PLACES="{0,4,8,12,16,20,24,28,32,36},{0,4,8,12,16,20,24,28,32,36}"

int main(int argc, char* argv[]){

	//TODO get arguments 
	// if (!parse_args(argc, argv)) {
	// 	std::cerr << "Error parsing arguments. Exiting..." << '\n';
	// 	return EXIT_FAILURE;
	// }

	//Openmp clauses
	// 
	omp_set_num_threads(thread_num);
	omp_set_dynamic(false);
	
	//1. Shared set creation 
	//TODO generalizar para n sets compartidos
	/*Thread_array::init_shared(ssets_size, )----------------*/
	
	//2. Private sets creation
	// vector<Thread_array> thread_list;
	// for()
	
	cout << omp_get_num_places() << "\n";



	//zona paralela
	// #pragma omp parallel
	// {
	// 	cout << "hilo: " << omp_get_thread_num() << endl; 

	// }

	
	
}
