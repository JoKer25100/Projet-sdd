#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>
#include "Parser.h"
#include "hash.h"
#include "Handler.h"
#include "Cpu.h"

int matches(const char *pattern, const char *string) {
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) {
        fprintf(stderr, "Regex compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

CPU *cpu_init(int memory_size){
    CPU *res = (CPU*)malloc(sizeof(CPU));
    res->memory_handler = memory_init(memory_size);
    res->context = hashmap_create();
    res->constant_pool = hashmap_create();
    hashmap_insert(res->context, "AX", (void*)0);
    hashmap_insert(res->context, "BX", (void*)0);
    hashmap_insert(res->context, "CX", (void*)0);
    hashmap_insert(res->context, "DX", (void*)0);
    return res;
}

void cpu_destroy(CPU *cpu){
    hashmap_destroy(cpu->context);
    hashmap_destroy(cpu->constant_pool);
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
    // Récupère le segment de données (DS) depuis le gestionnaire de mémoire
    Segment *segment = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (segment == NULL) {
        // Affiche un message d'erreur si le segment de données n'est pas trouvé
        fprintf(stderr, "Segment de données non trouvé.\n");
        return;
    }
    
    // Récupère la taille du segment de données
    int taille_segment = segment->size;
    // Parcourt chaque position dans le segment de données
    for (int i = 0; i < taille_segment; i++) {
        // Charge la valeur à la position i dans le segment de données
        int *value = (int*)load(cpu->memory_handler, "DS", i);
        if (value != NULL) {
            // Affiche la valeur si elle n'est pas NULL
            printf("DS[%d] = %d\n", i, *value);
        } else {
            // Affiche NULL si aucune valeur n'est trouvée à cette position
            printf("DS[%d] = NULL\n", i);
        }
    }
}

void *immediate_addressing(CPU *cpu, const char *operand){
    // Vérifie si l'opérande est une valeur immédiate valide (un nombre entier)
    if (matches("^[0-9]+$", operand) == 0){
        printf("L'opérande %s n'est pas une valeur immédiate valide.\n", operand);
        return NULL;
    }

    // Vérifie si la valeur immédiate est déjà présente dans le pool de constantes
    if (hashmap_get(cpu->constant_pool, operand) != NULL){
        return hashmap_get(cpu->constant_pool, operand);
    }

    // Alloue de la mémoire pour stocker la valeur immédiate
    int *res = (int*)malloc(sizeof(int));
    *res = atoi(operand);

    // Insère la valeur immédiate dans le pool de constantes
    if (hashmap_insert(cpu->constant_pool, operand, res) != 0){
        free(res);
        printf("Erreur lors de l'insertion de la valeur immédiate %s dans le pool de constantes.\n", operand);
        return NULL;
    }

    // Retourne un pointeur vers la valeur immédiate
    return (void*)res;
}