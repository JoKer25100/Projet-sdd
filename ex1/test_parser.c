#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "hash.h"
#include "Handler.h"
#include "Parser.h"

void test_count_elements() {
    assert(count_elements("a,b,c") == 3);
    assert(count_elements("") == 0);
    printf("count_elements OK\n");
}

void test_parse_data() {
    HashMap *mem = hashmap_create();
    Instruction *instr = parse_data_instruction("x DW 42", mem);
    
    assert(strcmp(instr->mnemonic, "x") == 0);
    assert((intptr_t)hashmap_get(mem, "x") == 0);
    
    free(instr->mnemonic);
    free(instr->operand1);
    free(instr->operand2);
    free(instr);
    hashmap_destroy(mem);
    printf("parse_data_instruction OK\n");
}

void test_parse_code() {
    HashMap *labels = hashmap_create();
    Instruction *instr = parse_code_instruction("mov AX,1", labels, 0);
    
    assert(strcmp(instr->mnemonic, "mov") == 0);
    assert(hashmap_size(labels) == 0);
    
    free(instr->mnemonic);
    free(instr->operand1);
    free(instr->operand2);
    free(instr);
    hashmap_destroy(labels);
    printf("parse_code_instruction OK\n");
}

void test_parse_file() {
    // Crée un fichier test minimal
    FILE *f = fopen("test.asm", "w");
    fprintf(f, ".DATA\nx DW 1\n.CODE\nmov AX,x\n");
    fclose(f);
    
    ParserResult *res = parse("test.asm");
    assert(res->data_count == 1);
    assert(res->code_count == 1);
    
    free_parser_result(res);
    remove("test.asm");
    printf("parse OK\n");
}

int main() {
    test_count_elements();
    test_parse_data();
    test_parse_code();
    test_parse_file();
    printf("Tous les tests OK\n");
    return 0;
}