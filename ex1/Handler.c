#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hash.h"
#include "Handler.h"

MemoryHandler *memory_init(int size){
    MemoryHandler *res = malloc(sizeof(MemoryHandler));
    if (!res) return NULL;
    res->total_size = size;
    res->memory = calloc(size, sizeof(void*)); // Initialisé à NULL
    if (!res->memory) { free(res); return NULL; }
    
    res->free_list = malloc(sizeof(Segment));
    if (!res->free_list) { free(res->memory); free(res); return NULL; }
    res->free_list->start = 0;
    res->free_list->size = size;
    res->free_list->next = NULL;
    
    res->allocated = hashmap_create();
    if (!res->allocated) { free(res->free_list); free(res->memory); free(res); return NULL; }
    
    return res;
}

Segment *find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
    Segment *current = handler->free_list;
    *prev = NULL;
    while (current != NULL){
        if (current->start <= start && (current->start + current->size) >= (start + size)){
            return current;
        }
        if (current->start > start){
            return NULL;
        }
        *prev = current;
        current = current->next;
    }
    return NULL;
}

Segment *creer_segment(int start, int size){
    Segment *new_segment = (Segment*)malloc(sizeof(Segment));
    new_segment->start = start;
    new_segment->size = size;
    new_segment->next = NULL;
    return new_segment;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    Segment *prev = NULL;
    Segment *current = find_free_segment(handler, start, size, &prev);
    if (current == NULL) {
        return -1;
    }

    // Si le segment trouvé correspond exactement à la taille demandée
    if (current->start == start && current->size == size) {
        if (prev == NULL) {
            handler->free_list = current->next;
        } else {
            prev->next = current->next;
        }
        free(current);
    } else {
        // Si le segment trouvé est plus grand que la taille demandée
        if (current->start == start) {
            current->start += size;
            current->size -= size;
        } else if (current->start + current->size == start + size) {
            current->size -= size;
        } else {
            Segment *new_segment = creer_segment(start + size, current->start + current->size - (start + size));
            if (new_segment == NULL) {
                return -1; // Vérification d'erreur
            }
            new_segment->next = current->next;
            current->next = new_segment;
            current->size = start - current->start;
        }
    }

    Segment *allocated_segment = creer_segment(start, size);
    if (allocated_segment == NULL) {
        return -1; // Vérification d'erreur
    }

    if (hashmap_insert(handler->allocated, name, allocated_segment) != 0) {
        free(allocated_segment); // Libérer si l'insertion échoue
        return -1;
    }

    return 0;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    // 1. Vérifier que le segment existe
    Segment *seg = (Segment*)hashmap_get(handler->allocated, name);
    if (seg == NULL) return -1;

    // 2. Préparer les informations du segment à libérer
    int start = seg->start;
    int size = seg->size;
    int end = start + size;

    // 3. Supprimer de la table allouée
    hashmap_remove(handler->allocated, name);
    free(seg);

    // 4. Trouver la position d'insertion dans la free_list
    Segment *current = handler->free_list;
    Segment *prev = NULL;
    Segment *to_free = NULL;

    while (current != NULL && current->start < start) {
        prev = current;
        current = current->next;
    }

    // 5. Vérifier la fusion avec le segment précédent
    if (prev != NULL && prev->start + prev->size == start) {
        prev->size += size;
        to_free = prev;
    } else {
        // Créer un nouveau segment libre
        to_free = creer_segment(start, size);
        to_free->next = current;
        if (prev) prev->next = to_free;
        else handler->free_list = to_free;
    }

    // 6. Vérifier la fusion avec le segment suivant
    if (to_free->next != NULL && end == to_free->next->start) {
        to_free->size += to_free->next->size;
        Segment *temp = to_free->next;
        to_free->next = temp->next;
        free(temp);
    }

    return 0;
}

void memory_destroy(MemoryHandler *handler) {
    if (handler == NULL) return;

    // 1. Libérer la mémoire allouée dans le tableau memory
    if (handler->memory != NULL) {
        for (int i = 0; i < handler->total_size; i++) {
            if (handler->memory[i] != NULL) {
                free(handler->memory[i]);
            }
        }
        free(handler->memory);
    }

    // 2. Libérer la liste des segments libres
    Segment *current = handler->free_list;
    while (current != NULL) {
        Segment *next = current->next;
        free(current);
        current = next;
    }

    // 3. Détruire la hashmap (hashmap_destroy gère la libération des clés et des valeurs)
    if (handler->allocated != NULL) {
        hashmap_destroy(handler->allocated);
    }

    // 4. Libérer le handler lui-même
    free(handler);
}
