#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "Handler.h"
#include "Parser.h"

int main(int argc, char *argv[]) {
    ParserResult *res = parse("test_ex3.asm");
    if (res == NULL) {
        fprintf(stderr, "Erreur d'analyse du fichier\n");
        return 1;
    }

    CPU *cpu = cpu_init(100);
    if (cpu == NULL) {
        fprintf(stderr, "Erreur d'initialisation du CPU\n");
        return 1;
    }

    allocate_variables(cpu, res->data_instructions, res->data_count);
    print_data_segment(cpu);
    free_parser_result(res);
    cpu_destroy(cpu);
    return 0;
}