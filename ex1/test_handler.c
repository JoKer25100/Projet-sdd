#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "Handler.h"

int main() {
    // Initialiser le gestionnaire de mémoire avec une taille de 100 unités
    MemoryHandler *handler = memory_init(100);
    if (handler == NULL) {
        fprintf(stderr, "Erreur d'initialisation du gestionnaire de mémoire\n");
        return 1;
    }

    // Créer des segments de mémoire
    if (create_segment(handler, "segment1", 0, 20) != 0) {
        fprintf(stderr, "Erreur de création du segment1\n");
    }
    if (create_segment(handler, "segment2", 20, 30) != 0) {
        fprintf(stderr, "Erreur de création du segment2\n");
    }
    if (create_segment(handler, "segment3", 50, 25) != 0) {
        fprintf(stderr, "Erreur de création du segment3\n");
    }

    // Afficher l'état des segments alloués
    Segment *seg1 = (Segment*)hashmap_get(handler->allocated, "segment1");
    Segment *seg2 = (Segment*)hashmap_get(handler->allocated, "segment2");
    Segment *seg3 = (Segment*)hashmap_get(handler->allocated, "segment3");

    printf("Segment1: start=%d, size=%d\n", seg1->start, seg1->size);
    printf("Segment2: start=%d, size=%d\n", seg2->start, seg2->size);
    printf("Segment3: start=%d, size=%d\n", seg3->start, seg3->size);

    // Supprimer un segment de mémoire
    if (remove_segment(handler, "segment2") != 0) {
        fprintf(stderr, "Erreur de suppression du segment2\n");
    }

    // Afficher l'état des segments alloués après suppression
    seg2 = (Segment*)hashmap_get(handler->allocated, "segment2");
    if (seg2 == NULL) {
        printf("Segment2 a été supprimé avec succès\n");
    } else {
        printf("Erreur: Segment2 n'a pas été supprimé\n");
    }

    // Afficher l'état de la liste des segments libres
    Segment *free_seg = handler->free_list;
    printf("Segments libres:\n");
    while (free_seg != NULL) {
        printf("start=%d, size=%d\n", free_seg->start, free_seg->size);
        free_seg = free_seg->next;
    }

    // Libérer la mémoire allouée dynamiquement
    memory_destroy(handler);
    return 0;
}