#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hash.h"
#include "Handler.h"
#include "Parser.h"
#include "Cpu.h"

void test_cpu_init_destroy() {
    CPU *cpu = cpu_init(1024);
    assert(cpu != NULL);
    assert(cpu->memory_handler != NULL);
    assert(cpu->context != NULL);
    cpu_destroy(cpu);
    printf("cpu_init/destroy OK\n");
}

void test_register_operations() {
    CPU *cpu = cpu_init(1024);
    int *ax = (int*)hashmap_get(cpu->context, "AX");
    
    *ax = 42; // Test écriture registre
    assert(*ax == 42);
    
    void *reg = register_addressing(cpu, "AX");
    assert(reg != NULL && *(int*)reg == 42);
    
    cpu_destroy(cpu);
    printf("register operations OK\n");
}

void test_memory_operations() {
    CPU *cpu = cpu_init(1024);
    create_segment(cpu->memory_handler, "DS", 0, 10);
    
    int *val= malloc(sizeof(int));
    *val = 123;
    store(cpu->memory_handler, "DS", 0, val);
    int *loaded = (int*)load(cpu->memory_handler, "DS", 0);
    
    assert(*loaded == 123);
    cpu_destroy(cpu);
    printf("memory operations OK\n");
}

void test_addressing_modes() {
    CPU *cpu = setup_test_environment();
    
    // Test immédiat
    void *imm = immediate_addressing(cpu, "42");
    assert(imm != NULL && *(int*)imm == 42);
    
    // Test registre indirect
    void *indirect = register_indirect_addressing(cpu, "[AX]");
    assert(indirect != NULL && *(int*)indirect == 5); // AX=3 -> DS[3]=35
    
    cpu_destroy(cpu);
    printf("addressing modes OK\n");
}

void test_mov_instruction() {
    CPU *cpu = setup_test_environment();
    int *ax = (int*)hashmap_get(cpu->context, "AX");
    int *bx = (int*)hashmap_get(cpu->context, "BX");
    
    *ax = 10;
    handle_MOV(cpu, ax, bx);
    assert(*bx == 10);
    
    cpu_destroy(cpu);
    printf("MOV instruction OK\n");
}

int main() {
    //Premier test tres rapidement
    test_cpu_init_destroy();
    test_register_operations();
    test_memory_operations();
    test_addressing_modes();
    test_mov_instruction();
    
    printf("Tous les tests CPU ont réussi !\n");

    //Deuxieme test de la question 5.7
    CPU *cpu2 = setup_test_environment();
    if (cpu2 == NULL) {
        fprintf(stderr, "Erreur d'initialisation du CPU\n");
        return 1;
    }
    void *immediat_ad = immediate_addressing(cpu2,"4");
    printf("immediat_ad = %d\n",*(int*)immediat_ad);

    void *register_ad = register_addressing(cpu2,"AX");
    printf("register_ad = %d\n",*(int*)register_ad);

    void *memory_direct_ad = memory_direct_addressing(cpu2,"[4]");
    printf("memory_direct_ad = %d\n",*(int*)memory_direct_ad);

    void *register_indirect_ad = register_indirect_addressing(cpu2,"[AX]");
    printf("register_indirect_ad = %d\n",*(int*)register_indirect_ad);

    handle_MOV(cpu2,immediat_ad,register_ad);
    handle_MOV(cpu2,immediat_ad,memory_direct_ad);
    handle_MOV(cpu2,immediat_ad,register_indirect_ad);

    printf("AX = %d\n",*(int*)hashmap_get(cpu2->context,"AX"));
    printf("DS[4] = %d\n",*(int*)load(cpu2->memory_handler,"DS",4));
    printf("DS[AX] = %d\n",*(int*)load(cpu2->memory_handler,"DS",*(int*)hashmap_get(cpu2->context,"AX")));
    cpu_destroy(cpu2);
    return 0;
}
