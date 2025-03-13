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
    free_parser_result(res);
    return 0;
}