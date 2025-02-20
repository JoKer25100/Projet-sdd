#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hash.h"

unsigned long simple_hash(const char* str){
    unsigned long cle =0;
    for (unsigned long i = 0; i<strlen(str); i++){
        cle += str[i];
    }
    unsigned long A = (sqrt(5)-1)/2;
    unsigned long arr= (unsigned long)(cle*A);
    A = TABLE_SIZE*(cle*A-arr);
    unsigned long res = (unsigned long) A;
    return res;
}


HashMap* hashmap_create(){
    HashMap* res = (HashMap*)malloc(sizeof(HashMap));
    res->size = TABLE_SIZE;
    res-> table = calloc(res->size, sizeof(HashEntry));
    return res;
}

int hashmap_insert(HashMap *map, const char *key, void *value){
    unsigned long cle = simple_hash(key);
    if ((map->table[cle].value == TOMBSTONE)||(map->table[cle].key == TOMBSTONE)||(map->table[cle].value == TOMBSTONE)||(map->table[cle].key == TOMBSTONE)) return -1;
    map->table[cle].key = strdup(key);
    map->table[cle].value = value;
    return 0;
}

void *hashmap_get(HashMap *map, const char *key){
    return map->table[simple_hash(key)].value;
}

int hashmap_remove(HashMap *map, const char *key){
    if ((map->table[simple_hash(key)].value == TOMBSTONE)||(map->table[simple_hash(key)].key == TOMBSTONE)) return -1;
    map->table[simple_hash(key)].value = TOMBSTONE;
    map->table[simple_hash(key)].key = TOMBSTONE;
    return 0;
}

void hashmap_destroy(HashMap *map){
    for (int i=0; i<TABLE_SIZE;i++){
        free(map->table[i].key);
        free(map->table[i].value);
    }
    free(map->table);
    free(map);
}