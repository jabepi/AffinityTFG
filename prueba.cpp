#include <stdio.h>
#include <stdlib.h>
#include <numa.h>
#include <numaif.h>

int main() {
    if (numa_available() == -1) {
        printf("NUMA no está disponible en este sistema.\n");
        return 1;
    }

    int variable = 42;
    int *pointer = &variable;
    
	int node;
    if (get_mempolicy(&node, NULL, 0, pointer, MPOL_F_NODE | MPOL_F_ADDR) == -1) {
        perror("Error al obtener la política de memoria");
        return 1;
    }

    printf("La dirección de memoria %p pertenece al nodo NUMA %d.\n", (void *)pointer, node);
    return 0;
}