#ifndef _PARSER_H_
#define _PARSER_H_
#include "hash.h"

typedef struct {
    char *mnemonic ; // Instruction mnemonic (ou nom de variable pour .DATA)
    char *operand1 ; // Premier operande (ou type pour .DATA)
    char *operand2 ; // Second operande (ou initialisation pour .DATA)
} Instruction ;

typedef struct {
    Instruction **data_instructions ; // Tableau d’instructions .DATA
    int data_count ; // Nombre d’instructions .DATA
    Instruction **code_instructions ; // Tableau d’instructions .CODE
    int code_count ; // Nombre d’instructions .CODE
    HashMap *labels ; // labels -> indices dans code_instructions
    HashMap *memory_locations ; // noms de variables -> adresse memoire
} ParserResult ;

int count_elements(const char *valeur);//compte le nombre d'elements dans une chaine de caractere
Instruction *parse_data_instruction(const char *line, HashMap *memory_locations);//parse une instruction de type .DATA
Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count);//analyser et stocker une ligne de la section .CODE
ParserResult *parse(const char *filename);//analyser un fichier assembleur
void free_parser_result(ParserResult *result);//liberer la memoire allouee pour le resultat de l'analyse


#endif
