#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "Parser.h"
#include "hash.h"
#include "Handler.h"
#include "Cpu.h"


CPU *cpu_init(int memory_size){
    CPU *res = (CPU*)malloc(sizeof(CPU));
    res->memory_handler = memory_init(memory_size);
    res->context = hashmap_create();
    hashmap_insert(res->context, "AX", (void*)0);
    hashmap_insert(res->context, "BX", (void*)0);
    hashmap_insert(res->context, "CX", (void*)0);
    hashmap_insert(res->context, "DX", (void*)0);
    return res;
}

void cpu_destroy(CPU *cpu){
    hashmap_destroy(cpu->context);
    memory_destroy(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data){
    //On verifie que les parametres sont valides
    if (handler == NULL || segment_name == NULL){
        return NULL;
    }

    //On verifie que les conditions sont vérifiées
    Segment *segment = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (segment == NULL || pos < 0 || pos >= segment->size){
        return NULL;
    }

    //On stocke la valeur dans la mémoire
    handler->memory[segment->start + pos] = data;
    return handler->memory[segment->start + pos];
}
    
void* load(MemoryHandler *handler, const char *segment_name, int pos){
    //On verifie que les parametres sont valides
    if (handler == NULL || segment_name == NULL){
        return NULL;
    }

    //On verifie que les conditions sont vérifiées
    Segment *segment = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (segment == NULL || pos < 0 || pos >= segment->size){
        return NULL;
    }

    //On charge la valeur depuis la mémoire
    return handler->memory[segment->start + pos];
}

void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count) {
    if (cpu == NULL || data_instructions == NULL) {
        return;
    }

    // Calculer l'espace nécessaire au stockage des variables
    int taille_segment = 0;
    for (int i = 0; i < data_count; i++) {
        // Compter le nombre d'éléments dans operand2 (valeurs séparées par des virgules)
        char *operand2 = data_instructions[i]->operand2;
        taille_segment += count_elements(operand2);
    }

    // Allouer un segment de mémoire pour les variables
    if (create_segment(cpu->memory_handler, "DS", 0, taille_segment) != 0) {
        fprintf(stderr, "Erreur lors de l'allocation du segment de données.\n");
        return;
    }

    // Stocker les variables dans la mémoire
    int current_pos = 0;
    for (int i = 0; i < data_count; i++) {
        char *operand2 = data_instructions[i]->operand2;
        char *token = strtok(operand2, ",");
        while (token != NULL) {
            // Convertir la valeur en entier (supposons que les données sont des entiers)
            int value = atoi(token);
            store(cpu->memory_handler, "DS", current_pos, &value);
            current_pos++;
            token = strtok(NULL, ",");
        }
    }
}

void print_data_segment(CPU *cpu){
    Segment *segment = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (segment == NULL) {
        fprintf(stderr, "Segment de données non trouvé.\n");
        return;
    }
    
    int taille_segment = segment->size;
    for (int i = 0; i < taille_segment; i++) {
        int *value = (int*)load(cpu->memory_handler, "DS", i);
        if (value != NULL) {
            printf("DS[%d] = %d\n", i, *value);
        } else {
            printf("DS[%d] = NULL\n", i);
        }
    }
}