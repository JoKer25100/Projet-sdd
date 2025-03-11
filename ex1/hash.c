#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hash.h"

#define A ((sqrt(5) - 1) / 2)

unsigned long simple_hash(const char* str){
    /*convertis une chaıne de caracteres en un indice dans la table de hachage*/
    unsigned long cle = 0;
    for (unsigned long i = 0; i < strlen(str); i++){
        cle += str[i];
    }
    double frac_part = cle * A - floor(cle * A);
    unsigned long res = (unsigned long)(TABLE_SIZE * frac_part);
    return res;
}

HashMap* hashmap_create(){
    /*alloue dynamiquement une table de hachage et initialise ses cases a NULL*/
    HashMap* res = (HashMap*)malloc(sizeof(HashMap));
    res->size = TABLE_SIZE;
    res->table = calloc(res->size, sizeof(HashEntry));
    return res;
}

int hashmap_insert(HashMap *map, const char *key, void *value){
    /*insere un element dans la table de hachage*/
    unsigned long cle = simple_hash(key);
    while (map->table[cle].key != NULL && map->table[cle].key != TOMBSTONE) {
        if (strcmp(map->table[cle].key, key) == 0) {
            map->table[cle].value = value;
            return 0;
        }
        cle = (cle + 1) % TABLE_SIZE;
    }
    map->table[cle].key = strdup(key);
    map->table[cle].value = value;
    return 0;
}

void *hashmap_get(HashMap *map, const char *key){
    /*recupere un element a partir de sa cle*/
    unsigned long cle = simple_hash(key);
    while (map->table[cle].key != NULL) {
        if (map->table[cle].key != TOMBSTONE && strcmp(map->table[cle].key, key) == 0) {
            return map->table[cle].value;
        }
        cle = (cle + 1) % TABLE_SIZE;
    }
    return NULL;
}

int hashmap_remove(HashMap *map, const char *key){
    /*supprime un element de la table de hachage tout en assurant la continuite du sondage lineaire*/
    unsigned long cle = simple_hash(key);
    while (map->table[cle].key != NULL) {
        if (map->table[cle].key != TOMBSTONE && strcmp(map->table[cle].key, key) == 0) {
            free(map->table[cle].key);
            map->table[cle].key = TOMBSTONE;
            map->table[cle].value = TOMBSTONE; 
            return 0;
        }
        cle = (cle + 1) % TABLE_SIZE;
    }
    return -1;
}

void hashmap_destroy(HashMap *map){
    /*libere toute la memoire allouee a la table de hachage*/
    for (int i = 0; i < TABLE_SIZE; i++){
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);
        }
    }
    free(map->table);
    free(map);
}

int hashmap_size(HashMap *map){
    /*retourne le nombre d'elements dans la table de hachage*/
    int res = 0;
    for (int i = 0; i < TABLE_SIZE; i++){
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            res++;
        }
    }
    return res;
}