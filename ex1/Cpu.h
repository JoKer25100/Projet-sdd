#ifndef _CPU_H_
#define _CPU_H_
#include "MemoryHandler.h"
#include "hash.h"

typedef struct {
    MemoryHandler *memory_handler ; // Gestionnaire de memoire
    HashMap *context ; // Registres (AX, BX, CX, DX)
} CPU ;

CPU *cpu_init(int memory_size);

#endif // _CPU_H_