#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "hash.h"

int main() {
    // 1. Test de simple_hash
    unsigned long hash1 = simple_hash("test");
    unsigned long hash2 = simple_hash("test");
    unsigned long hash3 = simple_hash("autre");
    assert(hash1 == hash2);  // Même entrée = même hash
    assert(hash1 != hash3);  // Entrées différentes = hash différents (généralement)
    printf("simple_hash: OK\n");

    // 2. Création et destruction
    HashMap *map = hashmap_create();
    assert(map != NULL && map->size == TABLE_SIZE);
    printf("hashmap_create: OK\n");

    // 3. Insertion et récupération
    int *value1 = malloc(sizeof(int));
    int *value2 = malloc(sizeof(int));
    *value1 = 42;
    *value2 = 100;

    assert(hashmap_insert(map, "clé1", value1) == 0);
    assert(hashmap_get(map, "clé1") == value1);
    assert(hashmap_insert(map, "clé1", value2) == 0);  // Écrasement
    assert(hashmap_get(map, "clé1") == value2);
    printf("hashmap_insert/get: OK\n");

    // 4. Suppression
    assert(hashmap_remove(map, "clé1") == 0);
    assert(hashmap_get(map, "clé1") == NULL);
    assert(hashmap_remove(map, "inexistante") == -1);
    printf("hashmap_remove: OK\n");

    // 5. Taille de la table
    assert(hashmap_size(map) == 0);
    hashmap_insert(map, "clé2", value1);
    assert(hashmap_size(map) == 1);
    printf("hashmap_size: OK\n");

    // 6. Nettoyage final
    //value2 n'est pas dans la table de hachage on la free nous-meme
    free(value2);
    hashmap_destroy(map); // hashmap_destroy doit libérer les clés et les valeurs dynamiques
    printf("hashmap_destroy: OK\n\n");

    printf("Tous les tests passés avec succès !\n");
    return 0;
}