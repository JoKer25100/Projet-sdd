#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Handler.h"

void run_tests() {
    printf("=== Début des tests ===\n\n");
    
    // Test 1: Initialisation
    printf("Test 1: Initialisation... ");
    MemoryHandler *handler = memory_init(1024);
    assert(handler != NULL);
    assert(handler->free_list != NULL);
    assert(handler->free_list->start == 0);
    assert(handler->free_list->size == 1024);
    assert(handler->free_list->next == NULL);
    assert(handler->allocated != NULL);
    printf("OK\n");
    
    // Test 2: Création de segments
    printf("\nTest 2: Création de segments...\n");
    printf("- Création segment1 (0-256)... ");
    assert(create_segment(handler, "segment1", 0, 256) == 0);
    assert(handler->free_list->start == 256);
    assert(handler->free_list->size == 768);
    printf("OK\n");
    
    printf("- Création segment2 (256-512)... ");
    assert(create_segment(handler, "segment2", 256, 256) == 0);
    assert(handler->free_list->start == 512);
    assert(handler->free_list->size == 512);
    printf("OK\n");
    
    printf("- Tentative création trop grande (512-1025)... ");
    assert(create_segment(handler, "segment3", 512, 513) == -1);
    printf("OK (échec attendu)\n");
    
    printf("- Création segment3 (512-768)... ");
    assert(create_segment(handler, "segment3", 512, 256) == 0);
    assert(handler->free_list->start == 768);
    assert(handler->free_list->size == 256);
    printf("OK\n");
    
    // Test 3: Suppression de segments
    printf("\nTest 3: Suppression de segments...\n");
    printf("- Suppression segment2... ");
    assert(remove_segment(handler, "segment2") == 0);
    assert(handler->free_list->start == 256);
    assert(handler->free_list->size == 256);
    printf("OK\n");
    
    printf("- Suppression segment inexistant... ");
    assert(remove_segment(handler, "segment4") == -1);
    printf("OK (échec attendu)\n");
    
    printf("- Suppression segment1 (fusion avec libre précédent)... ");
    assert(remove_segment(handler, "segment1") == 0);
    assert(handler->free_list->start == 0);
    assert(handler->free_list->size == 512);
    printf("OK\n");
    
    // Test 4: Scénario complexe
    printf("\nTest 4: Scénario complexe...\n");
    printf("- Création segment4 (100-150)... ");
    assert(create_segment(handler, "segment4", 100, 50) == 0);
    assert(handler->free_list->start == 0);
    assert(handler->free_list->size == 100);
    assert(handler->free_list->next->start == 150);
    assert(handler->free_list->next->size == 362);
    printf("OK\n");
    
    printf("- Création segment5 (150-200)... ");
    assert(create_segment(handler, "segment5", 150, 50) == 0);
    assert(handler->free_list->start == 0);
    assert(handler->free_list->size == 100);
    assert(handler->free_list->next->start == 200);
    assert(handler->free_list->next->size == 312);
    printf("OK\n");
    
    printf("- Suppression segment4 et segment5 (fusion complexe)... ");
    assert(remove_segment(handler, "segment4") == 0);
    assert(remove_segment(handler, "segment5") == 0);
    assert(handler->free_list->start == 0);
    assert(handler->free_list->size == 512);
    printf("OK\n");
    
    
    // Nettoyage
    memory_destroy(handler);    
    
    printf("\n=== Tous les tests ont réussi ===\n");
}

int main() {
    run_tests();
    return 0;
}