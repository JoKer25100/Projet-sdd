#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "Parser.h"
#include "hash.h"
#include "Handler.h"
#include "Cpu.h"


CPU *cpu_init(int memory_size){
    CPU *res = (CPU*)malloc(sizeof(CPU));
    res->memory_handler = memory_init(memory_size);
    res->context = hashmap_create();
    hashmap_insert(res->context, "AX", (void*)0);
    hashmap_insert(res->context, "BX", (void*)0);
    hashmap_insert(res->context, "CX", (void*)0);
    hashmap_insert(res->context, "DX", (void*)0);
    return res;
}