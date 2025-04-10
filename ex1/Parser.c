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
    int *NbElem = (int*)malloc(sizeof(int));
    *NbElem = hashmap_size(memory_locations);


    hashmap_insert(memory_locations, res->mnemonic, (void*)NbElem);
    
    for (int i = 1; i < NbElemValue; i++) {
        char key[64];
        sprintf(key, "%s_%d", mnemonic, i); // Créer une clé unique pour chaque élément
        int *NbElem2 = (int *)malloc(sizeof(int));
        *NbElem2 = *NbElem + i;
        if (hashmap_insert(memory_locations, key, (void *)NbElem2) != 0) {
            fprintf(stderr, "Erreur d'insertion dans la table de hachage\n");
            free(res->mnemonic);
            free(res->operand1);
            free(res->operand2);
            free(res);
            return NULL;
        }
    }

    return res;
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count){
    Instruction *instr = (Instruction *)malloc(sizeof(Instruction));
    if (instr == NULL) {
        return NULL;
    }
    char label[32] = "";
    char mnemonic[32] = "";
    char operand1[32] = "";
    char operand2[32] = "";

    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " ,\t\n");
    
    if (token == NULL) {
        free(instr);
        free(line_copy);
        return NULL;
    }

    // On vérifie si le token est un label
    if (token != NULL && token[strlen(token) - 1] == ':') {
        strcpy(label, token);
        label[strlen(label)-1] = '\0'; // Ensure null-termination
        int *code_count_ptr = (int *)malloc(sizeof(int));
        *code_count_ptr = code_count;
        hashmap_insert(labels, label, (void *)code_count_ptr);
        token = strtok(NULL, " :,\t\n");
    }
    
    // On récupère toutes les informations de l'instruction
    if (token != NULL){
        strcpy(mnemonic, token);
        token = strtok(NULL, " :,\t\n");
    }

    if (token != NULL){
        strcpy(operand1, token);
        token = strtok(NULL, " :,\t\n");
    }

    if (token != NULL){
        strcpy(operand2, token);
        token = strtok(NULL, " :,\t\n");
    }

    instr->mnemonic = strdup(mnemonic);
    instr->operand1 = strdup(operand1);
    instr->operand2 = strdup(operand2);

    free(line_copy);
    return instr;
}

ParserResult *parse(const char *filename){
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return NULL;
    }

    ParserResult *res = (ParserResult *)malloc(sizeof(ParserResult));
    if (res == NULL) {
        fclose(f);
        return NULL;
    } 

    //Allocation de la mémoire pour tous les élements 
    char buffer [128];
    int code_count = 0;
    int data_count = 0;
    res->data_instructions = NULL;
    res->code_instructions = NULL;
    res->labels = hashmap_create();
    res->memory_locations = hashmap_create();

    if (res->labels == NULL || res->memory_locations == NULL) {
        free(res);
        fclose(f);
        return NULL;
    }

    //On lit les instructions de .DATA
    if (strcmp(fgets(buffer, 128, f), ".DATA\n") == 0) {
        while (fgets(buffer, 128, f) != NULL && strcmp(buffer, ".CODE\n") != 0) {
            Instruction *instr = parse_data_instruction(buffer, res->memory_locations);
            if (instr != NULL) {
                res->data_instructions = (Instruction **)realloc(res->data_instructions, (data_count + 1) * sizeof(Instruction *));
                if (res->data_instructions == NULL) {
                    free(res);
                    fclose(f);
                    return NULL;
                }
                res->data_instructions[data_count] = instr;
                data_count++;
            }
        }
    }

    //On lit les instructions de .CODE
    if (strcmp(buffer, ".CODE\n")== 0){
        while (fgets(buffer, 128, f) != NULL) {
            Instruction *instr = parse_code_instruction(buffer, res->labels, code_count);
            if (instr != NULL) {
                res->code_instructions = (Instruction **)realloc(res->code_instructions, (code_count + 1) * sizeof(Instruction *));
                if (res->code_instructions == NULL) {
                    free(res);
                    fclose(f);
                    return NULL;
                }
                res->code_instructions[code_count] = instr;
                code_count++;
            }
        }
    }
    res->data_count = data_count;
    res->code_count = code_count;
    fclose(f);
    return res;
}

void free_parser_result(ParserResult *result){
    if (result == NULL) {
        return;
    }

    if (result->data_instructions != NULL) {
        for (int i = 0; i < result->data_count; i++) {
            free(result->data_instructions[i]->mnemonic);
            free(result->data_instructions[i]->operand1);
            free(result->data_instructions[i]->operand2);
            free(result->data_instructions[i]);
        }   
        free(result->data_instructions);
    }

    if (result->code_instructions != NULL) {
        for (int i = 0; i < result->code_count; i++) {
            free(result->code_instructions[i]->mnemonic);
            free(result->code_instructions[i]->operand1);
            free(result->code_instructions[i]->operand2);
            free(result->code_instructions[i]);
        }
        free(result->code_instructions);
    }

    hashmap_destroy(result->labels);
    hashmap_destroy(result->memory_locations);
    free(result);
}

char* trim(char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;

    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}

int search_and_replace(char** str, HashMap* values) {
    if (!str || !*str || !values) return 0;

    int replaced = 0;
    char* input = *str;

    // Iterate through all keys in the hashmap
    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].key != (void*)-1) {
            char* key = values->table[i].key;
            int value = (int)(long)values->table[i].value;

            // Find potential substring match
            char* substr = strstr(input, key);
            if (substr) {
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                // Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                int remain_len = strlen(substr + key_len);

                // Create new string
                char* new_str = (char*)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                // Free and update original string
                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    // Trim the final string
    if (replaced) {
        char* trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}

int resolve_constants(ParserResult *result){
    if (result == NULL || result->code_instructions == NULL || result->labels == NULL || result->memory_locations == NULL) {
        return -1; // Erreur
    }
    for (int i=0; i<result->code_count; i++){
        Instruction *instruction = result->code_instructions[i];
        void *lb = hashmap_get(result->labels, instruction->operand1);
        if (lb){
            // Si l'instruction est une étiquette, on remplace le nom de l'étiquette par son adresse
            char new_val[32];
            snprintf(new_val, sizeof(new_val), "%ld", (intptr_t)lb);
            free(instruction->operand1);
            instruction->operand1 = strdup(new_val);
        }
        int replaced = search_and_replace(&instruction->operand2, result->memory_locations);
    }
    return 0; // Succès
}
