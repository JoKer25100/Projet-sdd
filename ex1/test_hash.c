#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_elements>\n", argv[0]);
        return 1;
    }

    int num_elements = atoi(argv[1]);
    if (num_elements <= 0) {
        fprintf(stderr, "Le nombre d'element doit etre positif\n");
        return 1;
    }

    // Créer une nouvelle table de hachage
    HashMap *map = hashmap_create();

    // Insérer des éléments dans la table de hachage 
    for (int i = 1; i <= num_elements; i++) {
        char key[20];
        char *value = malloc(20 * sizeof(char));
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, 20, "value%d", i);
        hashmap_insert(map, key, value);
    }

    // Récupérer des éléments de la table de hachage
    for (int i = 1; i <= num_elements; i++) {
        char key[20];
        snprintf(key, sizeof(key), "key%d", i);
        printf("%s: %s\n", key, (char *)hashmap_get(map, key));
    }

    // Supprimer un élément de la table de hachage
    char key_to_remove[20];
    snprintf(key_to_remove, sizeof(key_to_remove), "key%d", num_elements);
    char *value_to_remove = (char *)hashmap_get(map, key_to_remove); // Récupérer la valeur avant suppression
    hashmap_remove(map, key_to_remove);
    if (value_to_remove != NULL) {
        free(value_to_remove); // Libérer la mémoire de la valeur supprimée
    }

    // Essayer de récupérer l'élément supprimé
    printf("%s: %s\n", key_to_remove, (char *)hashmap_get(map, key_to_remove));

    // Libérer les valeurs allouées dynamiquement dans la table de hachage
    for (int i = 1; i <= num_elements; i++) {
        char key[20];
        snprintf(key, sizeof(key), "key%d", i);
        char *value = (char *)hashmap_get(map, key);
        if (value != NULL|| value != TOMBSTONE) {
            free(value);
        }
    }

    // Détruire la table de hachage
    hashmap_destroy(map);

    return 0;
}