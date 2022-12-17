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
		//TODO cuando generalizar esto es un vector de vectores
		static vector<char,na_t> *shared_vec;

		//La zona prinvada siempre es única
		vector<char,na_t > *priv_vec;

	public: 
		Thread_array(){
			priv_vec = NULL;
		}
		void init_private(long tam_priv, short node){
			priv_vec = new vector<char,na_t>(tam_priv,'0',na_t(node));
		}
		void end_private(){
			delete priv_vec;
		}
		static void init_shared(long tam_shared, short node){
			shared_vec = new vector<char,na_t>(tam_shared,'1',na_t(node));
		}
		static void end_shared(){
			delete shared_vec;
		}

		//Fuctions over vectors data
		int read_private(){
			int size = (*priv_vec).size();
			int add = 0; 
			for(int i = 0; i< size; i ++){
				add += (*priv_vec)[i];
			}
			return add;
		}
		int read_shared(){
			int size = (*shared_vec).size();
			int add = 0; 
			for(int i = 0; i< size; i ++){
				add += (*shared_vec)[i];
			}
			return add;
		}

		void write_private(){
			int size = (*priv_vec).size();
			for(int i = 0; i< size; i ++){
				(*priv_vec)[i] = 'X';
			}
		}
		void write_shared(){
			int size = (*priv_vec).size();
			for(int i = 0; i< size; i ++){
				(*priv_vec)[i] = 'X';
			}
		}

		void rw_private(){
			int size = (*priv_vec).size();
			for(int i = 0; i< size; i ++){
				(*priv_vec)[i] = (*priv_vec)[i] + 1;
			}
		}
		void rw_shared(){
			int size = (*priv_vec).size();
			for(int i = 0; i< size; i ++){
				(*priv_vec)[i] = (*priv_vec)[i] + 1;
			}
		}
};	
vector<char,na_t>* Thread_array::shared_vec;


int main(int argc, char* argv[]){

	//TODO obtener argumentos con el parser bien
	// if (!parse_args(argc, argv)) {
	// 	std::cerr << "Error parsing arguments. Exiting..." << '\n';
	// 	return EXIT_FAILURE;
	// }
	if(argc != 4){
		cout << "Introducir nodo de vector compartido." << endl;	
		return 0;
	}
	list_thr_node[0] = atoi(argv[1]);
	list_thr_node[1] = atoi(argv[2]);
	sset_node = atoi(argv[2]);

	//Openmp clauses
	omp_set_num_threads(thread_num);
	omp_set_dynamic(false);	
	//NO funciona, hecho desde el script --> bucar solución (los entornos son distintos)
	// unsetenv("GOMP_CPU_AFFINITY");
	// setenv("GOMP_CPU_AFFINITY", "0 17", 1);
	// cout<<getenv("GOMP_CPU_AFFINITY");  
	
	//1. Shared set creation 
	// TODO generalizar para n sets compartidos
	Thread_array::init_shared(ssets_size, sset_node);
	
	//2. Time marks vector
	vector<double> r_tmarks(thread_num);
	vector<double> w_tmarks(thread_num);
	vector<double> rw_tmarks(thread_num);

	//3. Private sets creation & access
	#pragma omp parallel
	{
		//Allocated thread and the vector at the same node 
		int thread = omp_get_thread_num();
		Thread_array array = Thread_array();
		array.init_private(psets_size, list_thr_node[thread]);
		#pragma omp barrier

		//Vector access (without cache flush)
		
		//1. Memory reads
		auto start = std::chrono::high_resolution_clock::now();
		array.read_private();
		array.read_shared();

		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
		r_tmarks[thread] = diff.count();

		//2. Memory writes
		start = std::chrono::high_resolution_clock::now();
		array.write_private();
		array.write_shared();

		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		w_tmarks[thread] = diff.count();

		//3. Memory reads and writes
		start = std::chrono::high_resolution_clock::now();
		array.rw_private();
		array.rw_shared();

		end = std::chrono::high_resolution_clock::now();
        diff = end - start;
		rw_tmarks[thread] = diff.count();

		//Deallocate memory 
		array.end_private();
	}
	
	Thread_array::end_shared();

	cout << "Resultado hilo 0:" << endl;
	cout << "T_lectura: " << r_tmarks[0] << endl;
	cout << "T_escritura: " << w_tmarks[0] << endl;
	cout << "T_lectura/escritura: " << rw_tmarks[0] << "\n\n";

	cout << "Resultado hilo 1:" << endl;
	cout << "T_lectura: " << r_tmarks[1] << endl;
	cout << "T_escritura: " << w_tmarks[1] << endl;
	cout << "T_lectura/escritura: " << rw_tmarks[1] << "\n\n";
	
	return 0;
}
