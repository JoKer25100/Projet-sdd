#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "Parser.h"
#include "hash.h"
#include "Handler.h"


int count_elements(const char *valeur) {
    if (valeur == NULL || strlen(valeur) == 0) {
        return 0;
    }

    // Copier la valeur pour éviter de modifier l'original
    char *valeur_copy = strdup(valeur);
    if (valeur_copy == NULL) {
        return 0; 
    }

    int cpt = 0;
    char *token = strtok(valeur_copy, ","); // Diviser la valeur en fonction des virgules

    while (token != NULL) {
        cpt++;
        token = strtok(NULL, ","); // Passer à l'élément suivant
    }

    free(valeur_copy); // Libérer la copie de la valeur
    return cpt;
}

Instruction *parse_data_instruction(const char *line, HashMap *memory_locations){
    char mnemonic[32];
    char operand1[32];
    char operand2[32];
    sscanf(line, "%s %s %s", mnemonic, operand1, operand2);
    Instruction *res = (Instruction*)malloc(sizeof(Instruction));
    res->mnemonic = strdup(mnemonic);
    res->operand1 = strdup(operand1);
    res->operand2 = strdup(operand2);


    int NbElemValue = count_elements(operand2);
    int NbElem = hashmap_size(memory_locations);

    hashmap_insert(memory_locations, res->mnemonic, (void*)(intptr_t)NbElem);
    
    for (int i = 1; i < NbElemValue; i++) {
        char key[64];
        sprintf(key, "%s_%d", mnemonic, i); // Créer une clé unique pour chaque élément
        hashmap_insert(memory_locations, key, (void *)(intptr_t)(NbElem + i));
    }

    return res;
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count){
    Instruction *instr = (Instruction *)malloc(sizeof(Instruction));
    if (instr == NULL) {
        return NULL;
    }
    char label[32];
    char mnemonic[32];
    char operand1[32];
    char operand2[32];

    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " :,\t\n");
    
    if (token == NULL) {
        free(instr);
        free(line_copy);
        return NULL;
    }

    //On vérifie si le token est un label
    if (token != NULL && token[strlen(token) - 1] == ':') {
        strncpy(label, token, strlen(token) - 1);
        hashmap_insert(labels, label, (void *)(intptr_t)code_count);
        token = strtok(NULL, " :,\t\n");
    }
    
    //On récupère toutes les informations de l'instruction
    if (token != NULL){
        strncpy(mnemonic, token, 32);
        token = strtok(NULL, " :,\t\n");
    }

    if (token != NULL){
        strncpy(operand1, token, 32);
        token = strtok(NULL, " :,\t\n");
    }

    if (token != NULL){
        strncpy(operand2, token, 32);
        token = strtok(NULL, " :,\t\n");
    }

    strcpy(instr->mnemonic, mnemonic);
    strcpy(instr->operand1, operand1);
    strcpy(instr->operand2, operand2);

    free(line_copy);
    return instr;
}