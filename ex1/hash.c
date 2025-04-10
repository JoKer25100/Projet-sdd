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

int hashmap_insert(HashMap *map, const char *key, void *value) {
    // Vérification des paramètres
    if (map == NULL || key == NULL) return -1;

    // Calcul de l'index
    unsigned long index = simple_hash(key) % TABLE_SIZE;
    int start_index = index;

    // Probing linéaire
    do {
        HashEntry* entry = &map->table[index];
        if (entry->key == NULL || entry->key == TOMBSTONE) {
            free(entry->key); // Libère l'ancienne clé si TOMBSTONE
            entry->key = strdup(key);
            entry->value = value;
            printf("Inserting key: %s, value: %d\n", key, *(int*)value);
            return 0;
        }
        
        // Clé existante
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            printf("Key already exists, updating value\n");
            return 0;
        }

        // Collision
        index = (index + 1) % TABLE_SIZE;
    } while (index != start_index);

    return -3; // Table pleine
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
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);
            if ((map->table[i].value != NULL) && (map->table[i].value != TOMBSTONE)) {
                free(map->table[i].value);
            }
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