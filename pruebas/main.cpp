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
		//TODO borrar
		void initPrivate(){
			vector<char,na_t > priv_vec = vector<char,na_t >(10,'2',na_t(0));
		}
		void prueba(){
			cout << priv_vec[0] << endl;
		}
		
		// static void init_shared(size_t tam_shared, short node){
		// 	shared_vec = vector<char,na_t>(tam_shared,'1',na_t(node));
		// }


		//TODO función para ejecutar parte compartida

};	
vector<char,na_t> Thread_array::shared_vec;


int main(int argc, char* argv[]){
	
	//1. Thread's arrays creation 
	// Thread_array::init_shared(ssets_size, sset_node);

	
	Thread_array aux2;
	aux2.initPrivate();
	aux2.prueba();

	//aux2.prueba();

	// vector<Thread_array> thread_list;
	// for(int i = 0; i < thread_num; i ++){
	//  	thread_list.push_back(Thread_array(psets_size, list_thr_node[i]));
	// }

	
	
	
}