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
auto parse_args(int argc, char * const argv[]) -> int;

//Global variables
size_t thread_num   = 2;
std::vector<size_t> list_thr_node {0, 1};
long psets_size = 10;
size_t ssets_num = 1;
long ssets_size = 10;
short sset_node = 0;
std::vector<size_t> list_ssets_thread {0, 0};

//Types def
typedef NumaAlloc::NumaAlloc<char> na_t;
class Thread_array{

	private:
		static vector<char,na_t> shared_vec;
		vector<char,na_t > priv_vec;

	public: 
		Thread_array(int tam_priv, short node){
			//priv_vec = vector<char,na_t >(tam_priv,'2',na_t(node));
			
		}
		// static void init_shared(size_t tam_shared, short node){
		// 	shared_vec = vector<char,na_t>(tam_shared,'1',na_t(node));
		// }

		//TODO borrar
		 void prueba(){
			
			vector<char,na_t > priv_vec = vector<char,na_t >(10,'2',na_t(0));
		}

		//TODO función para ejecutar parte compartida

};	
vector<char,na_t> Thread_array::shared_vec;


int main(int argc, char* argv[]){
	
	//TODO get arguments 
	// if (!parse_args(argc, argv)) {
	// 	std::cerr << "Error parsing arguments. Exiting..." << '\n';
	// 	return EXIT_FAILURE;
	// }

	//1. Thread's arrays creation 
	// Thread_array::init_shared(ssets_size, sset_node);

	vector<char,na_t > priv_vec = vector<char,na_t >(10,'2',na_t(0));
//	Thread_array aux2(ssets_size, psets_size);

	//aux2.prueba();

	// vector<Thread_array> thread_list;
	// for(int i = 0; i < thread_num; i ++){
	//  	thread_list.push_back(Thread_array(psets_size, list_thr_node[i]));
	// }

	
	
	
}
auto parse_args(const int argc, char * const argv[]) -> int {
	CLI::App app("Thread affinity bechmark: This benchmark creates multiple threads with a private "
		"and a public memory set in a NUMA distribution, allowing you define:\n"
		"-Number of threads.\n"
		"-Phisical position of each thread on the distribution."
		"-Size of each private set.\n"
		"-The number of shared sets.\n"
		"-The phisical position of each set on the distribution.\n"
		"-The threads that share a set.\n");
	
	// app.add_option("-t,--num-threads", thread_num,
	//                "Number of threads on the executions. Integer >=1");
	// app.add_option("-n,--list-thr-node", list_thr_node,
	//                "List of nodes where each thread is going to be created. List of integers >= 0 (as long as the number of threads)");
	// app.add_option("-p,--psets-size", psets_size, 
	// 				"Size of the private part for each thread (Bytes). Integer >= 0."); 
	// app.add_option("-s,--ssets-num", ssets_num,
	//                "Number of shared sets.");
	// app.add_option("-s,--ssets-size", ssets_size,
	//                "Size of each shared set (Bytes). List of integer >= 0.");
	// app.add_option("-P,--list-ssets-node", list_ssets_node,
	//                "Node where each shared set is going to be created. List of integers >=0 (as long as the number of public sets)");
	// app.add_option("-l,--set-per-thread", list_ssets_thread,
	//     "Shared set for each thread (if only one public set this option may be omited). List of intergers >=0");
	// //app.add_flag("-v,--verbose", verbose, "Print verbose messages.");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError & e) {
		return app.exit(e);
	}

	return 1;
}



//Simp 

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

//Global variables
// size_t thread_num   = 2;
// std::vector<size_t> list_thr_node {0, 1};
// long psets_size = 10;
// size_t ssets_num = 1;
// long ssets_size = 10;
// short sset_node = 0;
// std::vector<size_t> list_ssets_thread {0, 0};

//Types def
//typedef NumaAlloc::NumaAlloc<char> na_t;

int main(int argc, char* argv[]){
	
	//1. Shared data
    //vector<char,na_t > shared_set = vector<char,na_t >(ss,'0',na_t(0)); 
	

	vector<int>* a;

    a = new vector<int>(10, 0);
	return 0;
}