#ifndef _HASH_H_
#define _HASH_H_

#define TABLE_SIZE 128
#define TOMBSTONE ((void*) - 1)


typedef struct hashentry {
    char* key;
    void* value;
} HashEntry;

typedef struct hashmap {
    int size;
    HashEntry* table;
} HashMap;

unsigned long simple_hash(const char *str); // Fonction de hachage simple
HashMap* hashmap_create(); // Cree une table de hachage
int hashmap_insert(HashMap *map, const char *key, void *value); // Insere un element dans la table de hachage
void *hashmap_get(HashMap *map, const char *key); // Recupere un element a partir de sa cle
int hashmap_remove(HashMap *map, const char *key); // Supprime un element de la table de hachage
void hashmap_destroy(HashMap *map); // Libere la memoire allouee a la table de hachage
int hashmap_size(HashMap *map); // Retourne le nombre d'elements dans la table de hachage

#endif