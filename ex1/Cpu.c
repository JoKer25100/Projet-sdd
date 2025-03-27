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

CPU *cpu_init(int memory_size) {
    CPU *res = (CPU*)malloc(sizeof(CPU));
    if (res == NULL) return NULL;

    res->memory_handler = memory_init(memory_size);
    if (res->memory_handler == NULL) {
        free(res);
        return NULL;
    }

    res->context = hashmap_create();
    if (res->context == NULL) {
        memory_destroy(res->memory_handler);
        free(res);
        return NULL;
    }

    res->constant_pool = hashmap_create();
    if (res->constant_pool == NULL) {
        hashmap_destroy(res->context);
        memory_destroy(res->memory_handler);
        free(res);
        return NULL;
    }

    // Initialisation des registres
    int *zero1 = malloc(sizeof(int));
    int *zero2 = malloc(sizeof(int));
    int *zero3 = malloc(sizeof(int));
    int *zero4 = malloc(sizeof(int));
    if (!zero1 || !zero2 || !zero3 || !zero4) {
        free(zero1); free(zero2); free(zero3); free(zero4);
        hashmap_destroy(res->constant_pool);
        hashmap_destroy(res->context);
        memory_destroy(res->memory_handler);
        free(res);
        return NULL;
    }

    *zero1 = 0; *zero2 = 0; *zero3 = 0; *zero4 = 0;
    hashmap_insert(res->context, "AX", zero1);
    hashmap_insert(res->context, "BX", zero2);
    hashmap_insert(res->context, "CX", zero3);
    hashmap_insert(res->context, "DX", zero4);

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
        char *operand2 = strdup(data_instructions[i]->operand2); // Copie pour strtok
        char *token = strtok(operand2, ",");
        while (token != NULL) {
            int *value = malloc(sizeof(int)); // Allocation permanente
            *value = atoi(token);
            store(cpu->memory_handler, "DS", current_pos, value);
            current_pos++;
            token = strtok(NULL, ",");
        }
        free(operand2);
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
        int *value = (int*)load(cpu->memory_handler, "DS", i);
        if (value != NULL && value != TOMBSTONE) {
            printf("DS[%d] = %d\n", i, *value);
        } else {
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

void *register_addressing(CPU *cpu, const char *operand){
    // Vérifie si l'opérande correspond à un registre valide (deux lettres majuscules, ex: AX, BX)
    if (matches("^[A-Z]{2}$", operand) == 0){
        printf("L'opérande %s n'est pas un registre valide.\n", operand);
        return NULL;
    }

    // Récupère la valeur associée au registre dans le contexte du CPU
    void *res = hashmap_get(cpu->context, operand);
    if (res == NULL){
        // Affiche un message si le registre n'est pas initialisé
        printf("Le registre %s n'est pas initialisé.\n", operand);
        return NULL;
    }

    // Retourne la valeur du registre
    return res; 
}

void *memory_direct_addressing(CPU *cpu, const char *operand){
    // Vérifie si l'opérande correspond à une adresse mémoire directe (ex: [42])
    if (matches("^\\[[0-9]+\\]$", operand) == 0) { // Échappement des crochets
        printf("Adresse invalide\n");
        return NULL;
    }

    // Convertit l'adresse en entier
    int pos = atoi(operand);

    // Charge la valeur à l'adresse spécifiée dans le segment de données (DS)
    void *res = load(cpu->memory_handler, "DS", pos);
    if (res == NULL){
        // Affiche un message si aucune valeur n'est trouvée à cette adresse
        printf("Aucune valeur trouvée à la position %d dans le segment de données.\n", pos);
        return NULL;
    }

    // Retourne la valeur trouvée à l'adresse
    return res;
}

void *register_indirect_addressing(CPU *cpu, const char *operand){
    // Vérifie si l'opérande correspond à un registre indirect valide (ex: [AX], [BX])
    if (matches("^\\[[A-Z]{2}\\]$", operand) == 0) {
        printf("L'opérande %s n'est pas un registre indirect valide.\n", operand);
        return NULL;
    }

    // Récupère la valeur du registre spécifié (ex: AX, BX) dans le contexte du CPU
    char register_name[3]; // Stocke le nom du registre (ex: AX)
    strncpy(register_name, operand + 1, 2); // Extrait le nom du registre sans les crochets
    register_name[2] = '\0'; // Ajoute une terminaison nulle

    void *res = hashmap_get(cpu->context, register_name);
    if (res == NULL){
        // Affiche un message si le registre n'est pas initialisé
        printf("Le registre %s n'est pas initialisé.\n", register_name);
        return NULL;
    }
    char res2[5]; // Assurez-vous que le tableau est suffisamment grand
    sprintf(res2, "[%d]", *(int*)res); // Si res est un pointeur vers un entier
    // Retourne la valeur du registre
    return memory_direct_addressing(cpu,res2);
}

void handle_MOV(CPU* cpu, void* src, void* dest){
    // Vérifie si les opérandes sont valides
    if (src == NULL || dest == NULL){
        printf("Les opérandes ne peuvent pas être NULL.\n");
        return;
    }

    // Copie la valeur de la source vers la destination
    *(int*)dest = *(int*)src;
}

CPU *setup_test_environment(){
    // Initialiser le CPU
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur d'initialisation du CPU\n");
        return NULL;
    }

    // Initialiser les registres avec des valeurs spécifiques
    int *ax = (int *)hashmap_get(cpu->context, "AX");
    int *bx = (int *)hashmap_get(cpu->context, "BX");
    int *cx = (int *)hashmap_get(cpu->context, "CX");
    int *dx = (int *)hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    // Créer et initialiser le segment de données
    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 20);

        // Initialiser le segment de données avec des valeurs de test
        for (int i = 0; i < 10; i++) {
            int *value = (int *)malloc(sizeof(int));
            *value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
            store(cpu->memory_handler, "DS", i, value);
        }
    }

    printf("Test environment initialized.\n");
    return cpu;
}