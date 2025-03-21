#ifndef _CPU_H_
#define _CPU_H_
#include "Handler.h"
#include "hash.h"

typedef struct {
    MemoryHandler *memory_handler ; // Gestionnaire de memoire
    HashMap *context ; // Registres (AX, BX, CX, DX)
} CPU ;

CPU *cpu_init(int memory_size);// Initialise le CPU
void cpu_destroy(CPU *cpu);// Libere la memoire allouee dynamiquement
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);// Stocke une valeur dans la memoire
void* load(MemoryHandler *handler, const char *segment_name,int pos);// Charge une valeur depuis la memoire
void allocate_variables(CPU *cpu, Instruction** data_instructions,int data_count);// Alloue de la memoire pour les variables
void print_data_segment(CPU *cpu);// Affiche les valeurs des variables
#endif // _CPU_H_