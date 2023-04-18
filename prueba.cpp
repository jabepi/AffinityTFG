#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
 
int main()
{
    std::random_device rd{};
    std::mt19937 gen{rd()};
 
    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> d{5, 2};
 
    std::map<int, int> hist{};
    for (int n = 0; n != 10000; ++n)
        ++hist[std::round(d(gen))];
 
    for (auto [x, y] : hist)
        std::cout << std::setw(2) << x << ' ' << std::string(y / 200, '*') << '\n';
}


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

	//Archivo de salida 
	string salida = argv[i+3];
	ofstream outfile(salida+"Visual.txt", std::ios::app);
    if (!outfile.is_open()) {
       cout << "No se ha podido abrir el archivo " << salida+"Visual.txt" << endl;
       return 1;
    }
	//---------------------------------------------------------------------------------------------------


	// //2. Memory reads
		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	r1 = array.read_private(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
        // diff = end - start;
		// r_tmarkp[thread] = diff.count()/ITER;
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas privadas" << endl;
		// }
		
		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	r2 = array.read_shared(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
        // diff = end - start;
		// r_tmarks[thread] = diff.count()/ITER;
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas compartidas" << endl;
		// }
		
		// flushCache(list_thr_node[thread]);
		// doNotOptimizeAway(r1);
		// doNotOptimizeAway(r2);

		// //3. Memory reads and writes
		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	rw1 = array.rw_private(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
		// diff = end - start;
		// rw_tmarkp[thread] = diff.count()/ITER;
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas y escrituras privadas" << endl;
		// }

		// i=0;
		// #pragma omp barrier
		// start = std::chrono::high_resolution_clock::now();
		// while(i < ITER){
		// 	rw2 = array.rw_shared(ssets_size);
		// 	i++;
		// }
		// end = std::chrono::high_resolution_clock::now();
		// diff = end - start;
		// rw_tmarks[thread] = diff.count()/ITER;
		// doNotOptimizeAway(rw1);
		// doNotOptimizeAway(rw2);
		// #pragma omp single
		// {
		// 	cout << "Terminadas las lecturas y escrituras compartidas" << endl;
		// }
