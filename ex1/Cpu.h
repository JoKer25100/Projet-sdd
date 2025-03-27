#ifndef _CPU_H_
#define _CPU_H_
#include "Handler.h"
#include "hash.h"

typedef struct {
    MemoryHandler *memory_handler ; // Gestionnaire de memoire
    HashMap *context ; // Registres (AX, BX, CX, DX)
    HashMap *constant_pool; // Table de hachage pour stocker les valeurs immediates
} CPU ;

int matches(const char *pattern, const char *string);// Verifie si une chaine de caracteres correspond a un motif
CPU *cpu_init(int memory_size);// Initialise le CPU
void cpu_destroy(CPU *cpu);// Libere la memoire allouee dynamiquement
void *store(MemoryHandler *handler, const char *segment_name, int pos, void *data);// Stocke une valeur dans la memoire
void *load(MemoryHandler *handler, const char *segment_name,int pos);// Charge une valeur depuis la memoire
void allocate_variables(CPU *cpu, Instruction** data_instructions,int data_count);// Alloue de la memoire pour les variables

void print_data_segment(CPU *cpu);// Affiche les valeurs des variables

void *immediate_addressing(CPU *cpu, const char *operand);// Traite l'addressage immédiat
void *register_addressing(CPU *cpu, const char *operand);// Traite l'addressage par registre
void *memory_direct_addressing(CPU *cpu, const char *operand); // Traite l'addressage direct en memoire
void *register_indirect_addressing(CPU *cpu, const char *operand);// Traite l'addressage indirect par registre
void handle_MOV(CPU* cpu, void* src, void* dest);// Execute l'instruction MOV
void *resolve_addressing(CPU *cpu, const char *operand);// Resout l'addressage de l'operande

CPU *setup_test_environment (); //fonction de test
#endif // _CPU_H_