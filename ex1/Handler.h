#ifndef _HANDLER_H_
#define _HANDLER_H_
#include "hash.h"

typedef struct segment {
    int start ; // Position de debut (adresse) du segment dans la memoire
    int size ; // Taille du segment en unites de memoire
    struct Segment *next ; // Pointeur vers le segment suivant dans la liste chainee
} Segment;

typedef struct memoryHandler {
    void **memory ; // Tableau de pointeurs vers la memoire allouee
    int total_size ; // Taille totale de la memoire geree
    Segment *free_list ; // Liste chainee des segments de memoire libres
    HashMap *allocated ; // Table de hachage (nom, segment)
} MemoryHandler ;
 
MemoryHandler *memory_init(int size); //Initialise le gestionnaire de memoire
Segment *find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev); //Retourne un segment libre s’il existe
void liberer_segment(MemoryHandler *handler, Segment *segment); //Libere un segment de memoire
int create_segment(MemoryHandler *handler, const char *name,int start, int size);//alloue dynamiquement un segment de memoire de taille size a l’adresse memoire start; retourne 0 si l’operation a reussi, -1 sinon
int remove_segment(MemoryHandler *handler, const char *name); //Libere le segment de memoire nomme name; retourne 0 si l’operation a reussi, -1 sinon

#endif