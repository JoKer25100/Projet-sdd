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

    if (memory_size < 128) {
        printf("Erreur: La taille de la mémoire doit être supérieure à 128.\n");
        free(res);
        return NULL;
    }

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

    // Initialisation de la pile "stack segment"
    if (create_segment(res->memory_handler, "SS", 0, 128) != 0) {
        hashmap_destroy(res->constant_pool);
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
    int *zero5 = malloc(sizeof(int));
    int *zero6 = malloc(sizeof(int));
    int *zero7 = malloc(sizeof(int));
    int *sp = malloc(sizeof(int)); // Registre SP
    int *bp = malloc(sizeof(int)); // Registre BP
    int *es = malloc(sizeof(int));
    if (!zero1 || !zero2 || !zero3 || !zero4 || !zero5 || !zero6 || !zero7 || !sp || !bp || !es) {
        free(zero1); free(zero2); free(zero3); free(zero4); free(zero5); free(zero6); free(zero7); free(sp); free(bp); free(es);
        hashmap_destroy(res->constant_pool);
        hashmap_destroy(res->context);
        memory_destroy(res->memory_handler);
        free(res);
        return NULL;
    }

    *zero1 = 0; *zero2 = 0; *zero3 = 0; *zero4 = 0; *zero5 = 0; *zero6 = 0; *zero7 = 0; *sp = 128; *bp = 0; *es = -1;
    hashmap_insert(res->context, "AX", zero1);
    hashmap_insert(res->context, "BX", zero2);
    hashmap_insert(res->context, "CX", zero3);
    hashmap_insert(res->context, "DX", zero4);
    hashmap_insert(res->context, "IP", zero5);
    hashmap_insert(res->context, "ZF", zero6);
    hashmap_insert(res->context, "SF", zero7);
    hashmap_insert(res->context, "SP", sp);
    hashmap_insert(res->context, "BP", bp);
    hashmap_insert(res->context, "ES", es);
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
    if (create_segment(cpu->memory_handler, "DS", cpu->memory_handler->total_size, taille_segment) != 0) {
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
    char num[3];
    strncpy(num, operand + 1, 2); // Extrait le nom du registre sans les crochets
    num[2] = '\0'; // Ajoute une terminaison nulle
    int pos = atoi(num);


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
        create_segment(cpu->memory_handler, "DS", 128, 20);

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

void *resolve_addressing(CPU *cpu, const char *operand){
    if (immediate_addressing(cpu, operand)){
        return immediate_addressing(cpu, operand);
    }
    if (register_addressing(cpu, operand)){
        return register_addressing(cpu, operand);
    }
    if (memory_direct_addressing(cpu, operand)){
        return memory_direct_addressing(cpu, operand);
    }
    if (register_indirect_addressing(cpu, operand)){
        return register_indirect_addressing(cpu, operand);
    }
    if (segment_override_addressing(cpu, operand)){
        return segment_override_addressing(cpu, operand);
    }
    return NULL;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count){
    if (cpu == NULL || code_instructions == NULL) {
        return;
    }

    // Mettre à jour le registre IP (Instruction Pointer) pour pointer vers le début du segment de code
    int *ip = (int *)hashmap_get(cpu->context, "IP");
    if (!ip) {
        fprintf(stderr, "Registre IP non trouvé\n");
        return;
    }
    *ip = 0;

    // Création de l'espace nécessaire pour le segment de code
    if (create_segment(cpu->memory_handler, "DS", cpu->memory_handler->total_size, code_count) != 0) {
        fprintf(stderr, "Erreur lors de l'allocation du segment de données.\n");
        return;
    }

    // Stocker les instructions dans la mémoire
    for (int i = 0; i < code_count; i++) {
        Instruction *instruction = (Instruction*)malloc(sizeof(Instruction));
        if (instruction == NULL) {
            fprintf(stderr, "Erreur lors de l'allocation de l'instruction.\n");
            return;
        }

        instruction->mnemonic = strdup(code_instructions[i]->mnemonic);
        if (code_instructions[i]->operand1) instruction->operand1 = strdup(code_instructions[i]->operand1);
        if (code_instructions[i]->operand2) instruction->operand2 = strdup(code_instructions[i]->operand2);

        if (store(cpu->memory_handler, "CS", i, instruction) == NULL) {
            fprintf(stderr, "Échec de stockage de l'instruction %d\n", i);
            free(instruction->mnemonic);
            free(instruction->operand1);
            free(instruction->operand2);
            free(instruction);
        }
    }
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest){
    if (cpu == NULL || instr == NULL) {
        return -1; // Erreur
    }

    // MOV
    if (strcmp(instr->mnemonic, "MOV") == 0) {
        handle_MOV(cpu, src, dest);
        return 0; // Succès
    }
    
    // ADD
    if (strcmp(instr->mnemonic, "ADD") == 0) {
        *(int*)dest += *(int*)src;  
        return 0; // Succès
    }

    // CMP
    if (strcmp(instr->mnemonic, "CMP") == 0) {
        int result = *(int*)dest - *(int*)src;
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        int *sf = (int *)hashmap_get(cpu->context, "SF");
        if (result == 0) {
            *zf = 1; // ZF = 1 si égal
            *sf = 0; // SF = 0
        } else if (result < 0) {
            *zf = 0; // ZF = 0
            *sf = 1; // SF = 1 si négatif
        } else {
            *zf = 0; // ZF = 0
            *sf = 0; // SF = 0
        }
        return 0; // Succès
    }

    // JMP address
    if (strcmp(instr->mnemonic, "JMP") == 0) {
        void *ip =hashmap_get(cpu->context, "IP");
        ip = instr->operand1;
        return 0; // Succès
    }

    // JZ address
    if (strcmp(instr->mnemonic, "JZ") == 0) {
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 1) {
            void *ip = hashmap_get(cpu->context, "IP");
            ip = instr->operand1;
        }
        return 0; // Succès
    }

    // JNZ address
    if (strcmp(instr->mnemonic, "JNZ") == 0) {
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 0) {
            void *ip = hashmap_get(cpu->context, "IP");
            ip = instr->operand1;
        }
        return 0; // Succès
    }

    // HALT
    if (strcmp(instr->mnemonic, "HALT") == 0) {
        void *ip =hashmap_get(cpu->context, "IP");
        Segment* seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
        int size = seg->size;
        void *data = load(cpu->memory_handler, "CS", size);
        data = ip;
        return 0; // Succès
    }

    // PUSH
    if (strcmp(instr->mnemonic, "PUSH") == 0) {

        int value = -1;
        if (src == NULL) {
            value = *(int*)hashmap_get(cpu->context, "AX");
        } else{
            value = *(int*)hashmap_get(cpu->context, instr->operand1);
        }
        
        if (value == -1) {
            fprintf(stderr, "Erreur: Registre %s non trouvé\n", instr->operand1);
            return -1; // Erreur
        }

        return push_value(cpu, value); // Succès
    }

    // POP
    if (strcmp(instr->mnemonic, "POP") == 0) {
        int res; 
        if (dest == NULL) {
            res = pop_value(cpu, dest);
        } else{
            res = pop_value(cpu, hashmap_get(cpu->context, "AX"));
        }

        return res; // Succès
    }

    // ALLOC
    if (strcmp(instr->mnemonic, "ALLOC") == 0) {
        return alloc_es_segment(cpu); // Succès
    }

    // FREE
    if (strcmp(instr->mnemonic, "FREE") == 0) {
        return free_es_segment(cpu); // Succès
    }

    // Si l'instruction n'est pas reconnue
    fprintf(stderr, "Instruction non reconnue: %s\n", instr->mnemonic);
    return -1; // Erreur
}

int execute_instruction(CPU *cpu, Instruction *instr){
    if (cpu == NULL || instr == NULL) {
        return -1; // Erreur
    }
    
    if (strcmp(instr->mnemonic,"MOV") == 0 || strcmp(instr->mnemonic,"ADD") == 0 || strcmp(instr->mnemonic,"CMP") == 0){
        void *src = resolve_addressing(cpu, instr->operand1);
        void *dest = resolve_addressing(cpu, instr->operand2);
        if (src == NULL || dest == NULL) {
            fprintf(stderr, "Erreur de résolution d'adressage pour MOV ou ADD\n");
            return -1; // Erreur
        }
        return handle_instruction(cpu, instr, src, dest);
    }
    return handle_instruction(cpu, instr, NULL, NULL);
}

Instruction* fetch_next_instruction(CPU *cpu){
    if (cpu == NULL) {
        return NULL; // Erreur
    }

    // Récupérer le registre IP (Instruction Pointer)
    int *ip = (int *)hashmap_get(cpu->context, "IP");
    if (ip == NULL) {
        fprintf(stderr, "Erreur: Registre IP non trouvé\n");
        return NULL; // Erreur
    }

    // Récupérer le segment de code (CS)
    Segment *segment = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
    if (segment == NULL) {
        fprintf(stderr, "Erreur: Segment de code non trouvé\n");
        return NULL; // Erreur
    }

    // Vérifier si l'IP est dans les limites du segment de code
    if (*ip < 0 || *ip >= segment->size) {
        fprintf(stderr, "Erreur: IP hors limites du segment de code\n");
        return NULL; // Erreur
    }
    // Charger l'instruction à l'adresse spécifiée par l'IP
    Instruction *instr = (Instruction *)load(cpu->memory_handler, "CS", *ip);
    if (instr == NULL) {
        fprintf(stderr, "Erreur: Instruction non trouvée à l'adresse %d\n", *ip);
        return NULL; // Erreur
    }
    // Incrémenter l'IP pour la prochaine instruction
    (*ip)++;
    return instr; // Retourner l'instruction chargée
}

void afficher_etat(CPU *cpu){
    printf("Segment de code :\n");
    print_data_segment(cpu);
    printf("====================================\n");
    printf("Registres :\n");
    printf("AX: %d\n", *(int*)hashmap_get(cpu->context, "AX"));
    printf("BX: %d\n", *(int*)hashmap_get(cpu->context, "BX"));
    printf("CX: %d\n", *(int*)hashmap_get(cpu->context, "CX"));
    printf("DX: %d\n", *(int*)hashmap_get(cpu->context, "DX"));
    printf("IP: %d\n", *(int*)hashmap_get(cpu->context, "IP"));
    printf("ZF: %d\n", *(int*)hashmap_get(cpu->context, "ZF"));
    printf("SF: %d\n", *(int*)hashmap_get(cpu->context, "SF"));
    printf("====================================\n");
}
int run_program(CPU *cpu){
    printf("Execution du programme :\n");
    printf("====================================\n");
    afficher_etat(cpu);

    // Boucle d'exécution du programme
    char input[64];
    while (1) {
        printf("Appuyez sur Entrée pour executer la prochaine instruction (q pour quitter): ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Erreur lors de la lecture de l'entrée\n");
            break;
        }
        
        // Execution prochaine commande
        if (strcmp(input, "\n") == 0) {
            printf("Execution de la prochaine instruction\n");
            Instruction *instr = fetch_next_instruction(cpu);
            if (instr == NULL) {
                printf("Erreur lors de la récupération de l'instruction\n");
                break;
            }
            execute_instruction(cpu, instr);
            afficher_etat(cpu);
        }

        // Quitter le programme
        else if (strcmp(input, "q\n") == 0) {
            printf("Execution interrompue par l'utilisateur.\n");
            break;
        }

        // Commande non reconnue
        else {
            printf("Commande non reconnue. Appuyez sur Entrée pour executer la prochaine instruction ou 'q' pour quitter.\n");
            continue;
        }
    }
    afficher_etat(cpu);
    printf("Fin de l'exécution du programme.\n");
    return 0; // Succès
}

int push_value(CPU *cpu, int value){
    if (cpu == NULL) {
        return -1; // Erreur
    }

    // Récupérer le registre SP (Stack Pointer)
    int *sp = (int *)hashmap_get(cpu->context, "SP");
    if (sp == NULL) {
        fprintf(stderr, "Erreur: Registre SP non trouvé\n");
        return -1; // Erreur
    }
    if (*sp <= 0){
        fprintf(stderr, "Erreur: Stack Overflow\n");
        return -1; // Erreur
    }
    (*sp)--;
    // Allouer de la mémoire pour la valeur à empiler
    int *value_ptr = (int *)malloc(sizeof(int));
    if (value_ptr == NULL) {
        fprintf(stderr, "Erreur: Impossible d'allouer de la mémoire pour la valeur\n");
        return -1; // Erreur
    }
    *value_ptr = value;
    // Stocker la valeur dans le segment de pile (SS)
    store(cpu->memory_handler, "SS", *sp, value_ptr);
    return 0; // Succès
}

int pop_value(CPU *cpu, int *dest){
    if (cpu == NULL) {
        return -1; // Erreur
    }

    int *sp = (int *)hashmap_get(cpu->context, "SP");
    if (sp == NULL) {
        fprintf(stderr, "Erreur: Registre SP non trouvé\n");
        return -1; // Erreur
    }
    
    if (*sp >= 128){
        fprintf(stderr, "Erreur: Stack Underflow\n");
        return -1; // Erreur
    }
    void *value = load(cpu->memory_handler, "SS", *sp);
    if (value == NULL){
        fprintf(stderr, "Erreur: Impossible de charger la valeur de la pile\n");
        return -1; // Erreur
    }
    *dest = *(int*)value;
    // Libérer la mémoire de la valeur chargée
    free(value);
    (*sp)++;
    return 0; // Succès
}

void* segment_override_addressing(CPU* cpu, const char* operand) {
    if (!cpu || !operand) return NULL;
    
    // Expression régulière pour valider le format
    if (!matches("^\\[A-Z]{2}:[A-Z]{2}\\]$", operand)) {
        printf("L'opérande %s n'est pas au format [segment:reg]\n", operand);
        return NULL;
    }
    // Extraction du segment et du registre
    char segment[3];
    char reg[3];
    strncpy(segment, operand + 1, 2); // Extrait le nom du registre sans les crochets
    strncpy(reg, operand + 4, 2);
    segment[2] = '\0'; // Ajoute une terminaison nulle
    reg[2] = '\0'; 
    
    // Vérification que le segment existe
    Segment* seg = hashmap_get(cpu->memory_handler->allocated, segment);
    if (!seg) return NULL;
    
    // Récupération de la valeur du registre
    int* reg_value = hashmap_get(cpu->context, reg);
    if (!reg_value) return NULL;
    
    // Chargement de la valeur
    return load(cpu->memory_handler, segment, *reg_value);
}
Segment *best_fit(MemoryHandler *handler, int size){
    Segment *best = NULL;
    Segment *current = handler->free_list;
    while (current != NULL) {
        if (current->size >= size) {
            if (best == NULL || current->size < best->size) {
                best = current;
            }
        }
        current = current->next;
    }
    return best;
}

Segment *worst_fit(MemoryHandler *handler, int size){
    Segment *worst = NULL;
    Segment *current = handler->free_list;
    while (current != NULL) {
        if (current->size >= size) {
            if (worst == NULL || current->size > worst->size) {
                worst = current;
            }
        }
        current = current->next;
    }
    return worst;
}

int find_free_address_strategy(MemoryHandler *handler, int size, int strategy){
    if (strategy == 0){
        Segment *first = find_free_segment(handler, handler->total_size, size, NULL);
        if (first == NULL) {
            return -1; // Pas de segment libre trouvé
        }
        return first->start;
    }

    if (strategy == 1){
        Segment *best = best_fit(handler, size);
        if (best == NULL) {
            return -1; // Pas de segment libre trouvé
        }
        return best->start;
    }

    if (strategy == 2){
        Segment *worst = worst_fit(handler, size);
        if (worst == NULL) {
            return -1; // Pas de segment libre trouvé
        }
        return worst->start;
    }
    return -1; // Stratégie non reconnue
}

int alloc_es_segment(CPU *cpu){
    if (cpu == NULL) {
        return -1; // Erreur
    }
    
    // Récupérer taille (AX) et stratégie (BX)
    int *ax = hashmap_get(cpu->context, "AX");
    int *bx = hashmap_get(cpu->context, "BX");
    int *es = hashmap_get(cpu->context, "ES");
    int *zf = hashmap_get(cpu->context, "ZF");
    
    if (!ax || !bx || !es || !zf) return -1; // Erreur si registres manquants
    if (*es != -1) {
        *zf = 1; // ZF = 1 si segment ES déjà alloué
        fprintf(stderr, "Erreur: Segment ES déjà alloué\n");
        return -1; // Erreur
    }
    if (*ax <= 0) {
        *zf = 1; // ZF = 1 si taille invalide
        fprintf(stderr, "Erreur: Taille de segment ES invalide\n");
        return -1; // Erreur
    }
    if (*bx < 0 || *bx > 2) {
        *zf = 1; // ZF = 1 si stratégie invalide
        fprintf(stderr, "Erreur: Stratégie d'allocation invalide\n");
        return -1; // Erreur
    }

    int start = find_free_address_strategy(cpu->memory_handler, *ax, *bx);
    if (start == -1) {
        *zf = 1; // ZF = 1 si pas de segment libre
        fprintf(stderr, "Erreur: Pas de segment libre trouvé\n");
        return -1; // Erreur
    }

    if (create_segment(cpu->memory_handler, "ES", start, *ax) != 0) {
        *zf = 1; // Échec
        return -1;
    }

    // Initialiser à 0 et mettre à jour ES
    for (int i = 0; i < *ax; i++) {
        int *zero = malloc(sizeof(int));
        *zero = 0;
        store(cpu->memory_handler, "ES", i, zero);
    }

    *es = start; // Adresse de base de ES
    *zf = 0;     // Succès
    return 0;
}

int free_es_segment(CPU *cpu){
    int *es = (int *)hashmap_get(cpu->context, "ES");
    if (es == NULL) {
        fprintf(stderr, "Erreur: Registre ES non trouvé\n");
        return -1; // Erreur
    }
    Segment *segment = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
    if (segment == NULL) {
        fprintf(stderr, "Erreur: Segment ES non trouvé\n");
        return -1; // Erreur
    }
    
    for (int i = 0; i < segment->size; i++) {
        int *value = (int *)load(cpu->memory_handler, "ES", i);
        if (value != NULL) {
            free(value); // Libérer la mémoire allouée
        }
    }
    remove_segment(cpu->memory_handler, "ES");
    *es = -1; // Réinitialiser ES
    return 0; // Succès
}