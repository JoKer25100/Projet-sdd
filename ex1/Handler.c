#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hash.h"
#include "Handler.h"

MemoryHandler *memory_init(int size){
    MemoryHandler *res = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    res->total_size = size;
    res->memory = (void**)malloc(size * sizeof(void*));
    res->free_list = (Segment*)malloc(sizeof(Segment));
    res->free_list->start = 0;
    res->free_list->size = size;
    res->free_list->next = NULL;
    res->allocated = hashmap_create();
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
            new_segment->next = current->next;
            current->next = new_segment;
            current->size = start - current->start;
        }
    }
 
    Segment *allocated_segment = creer_segment(start, size);
    hashmap_insert(handler->allocated, name, allocated_segment);
    return 0;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    Segment *segment = (Segment*)hashmap_get(handler->allocated, name);
    if (segment == NULL) {
        return -1;
    }

    Segment *libre = handler->free_list;
    Segment *prev = NULL;

    // Recherche du segment libre juste avant
    while (libre != NULL && libre->start + libre->size < segment->start) {
        prev = libre;
        libre = libre->next;
    }

    // Fusionner avec le segment libre précédent si adjacent
    if (prev != NULL && prev->start + prev->size == segment->start) {
        prev->size += segment->size;
        segment->start = prev->start;
        segment->size = prev->size;
        segment->next = prev->next;
        free(prev);
        prev = NULL;
    }

    // Fusionner avec le segment libre suivant si adjacent
    if (libre != NULL && segment->start + segment->size == libre->start) {
        segment->size += libre->size;
        segment->next = libre->next;
        free(libre);
    } else {
        segment->next = libre;
    }

    // Ajouter le segment libéré à la liste des segments libres
    if (prev == NULL) {
        handler->free_list = segment;
    } else {
        prev->next = segment;
    }
    hashmap_remove(handler->allocated, name);
    return 0;
}

void memory_destroy(MemoryHandler *handler) {
    // Libérer les segments alloués dynamiquement dans le gestionnaire de mémoire
    for (int i = 1; i <= 3; i++) {
        char key[20];
        snprintf(key, sizeof(key), "segment%d", i);
        Segment *seg = (Segment *)hashmap_get(handler->allocated, key);
        if (seg != NULL) {
            free(seg);
        }
    }

    // Libérer la mémoire allouée pour le gestionnaire de mémoire
    while (handler->free_list != NULL) {
        Segment *next = handler->free_list->next;
        free(handler->free_list);
        handler->free_list = next;
    }
    hashmap_destroy(handler->allocated);
    free(handler->memory);
    free(handler);
}