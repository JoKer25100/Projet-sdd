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
    Instruction *instr = parse_data_instruction("x DW 42,2", mem);
    
    assert(strcmp(instr->mnemonic, "x") == 0);
    assert(*(int*)hashmap_get(mem, "x_1") == 1);
    assert(*(int*)hashmap_get(mem, "x") == 0);
    
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
    printf("test_parse_file\n");
    FILE *f = fopen("test.asm", "w");
    fprintf(f, ".DATA\nx DW 42\narr DB 20,21,22,23\ny DB 10\n.CODE\nstart: MOV AX,x\nloop: ADD AX,y\nJMP loop\n");
    fclose(f);
    
    ParserResult *res = parse("test.asm");
    assert(res->data_count == 3);
    assert(res->code_count == 3);
    
    printf("%d\n",hashmap_size(res->memory_locations));
    free_parser_result(res);
    remove("test.asm");
    printf("parse OK\n");
}

void test_resolve_constants() {
    printf("test_resolve_constants\n");
    FILE *f = fopen("test.asm", "w");
    fprintf(f, ".DATA\nx DW 42\narr DB 20,21,22,23\ny DB 10\n.CODE\nstart: MOV AX,[x]\nloop: ADD AX,[y]\nJMP loop\n");
    fclose(f);
    ParserResult *res = parse("test.asm");
    
    printf("x : %s\n", res->code_instructions[1]->operand2);
    printf("y : %s\n", res->code_instructions[1]->operand2);
    printf("loop : %s\n", res->code_instructions[2]->operand1);
    int i = resolve_constants(res);
    assert(i == 0);
    
    // Vérification des valeurs résolues
    
    printf("x : %s\n", res->code_instructions[0]->operand2);
    printf("y : %s\n", res->code_instructions[1]->operand2);
    printf("loop : %s\n", res->code_instructions[2]->operand1);

    free_parser_result(res);
    remove("test.asm");
    printf("resolve OK\n");
}

int main() {
    test_count_elements();
    test_parse_data();
    test_parse_code();
    test_parse_file();
    test_resolve_constants();
    printf("Tous les tests OK\n");
    return 0;
}