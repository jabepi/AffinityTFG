

void getNode1(){
    int numa_node = -1;
    get_mempolicy(&numa_node, NULL, 0, (void*)a, MPOL_F_NODE | MPOL_F_ADDR);
    cout << a << endl;
}

void getNode2(){
    // Example pointer to check
    void* ptr_to_check = reinterpret_cast<void*>(0x2C0000000);

    //Determine the page size
    long page_size = sysconf(_SC_PAGESIZE);

    // Calculaos el puntero alineado a la página
    void* aligned_ptr = nullptr;
    if (posix_memalign(&aligned_ptr, page_size, sizeof(void*)) != 0) {
        std::cerr << "Failed to align pointer!" << std::endl;
        return 1;
    }

    // Imprimirmos los punteros
    std::cout << "Original pointer: " << ptr_to_check << std::endl;
    std::cout << "Aligned pointer:   " << aligned_ptr << std::endl;

	int status[1] = {-1};
    int ret_code;
	ret_code=numa_move_pages(0, 1, &aligned_ptr,NULL, status, 0);
	printf("Memory at %p is at %d node (retcode %d)\n", ptr_to_check, status[0], ret_code);

    free(aligned_ptr);
}

void comprobarNumalloc(){
    double *a = (double*) numa_alloc_onnode( 10 * sizeof(double), 3);
    cout << getpid() << endl;
    while(1);

    //Para comprobar que se ha asignado a un nodo
    //cat /proc/4152649/numa_maps | grep 7f6d05df2000
}




