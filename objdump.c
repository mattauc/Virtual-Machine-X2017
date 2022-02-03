#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

//Prints the values held in ASM array
void print_asm(struct code** assembly, int size) {

    int table[TABLE_SIZE];
    for (int i = size-1; i >= 0; i--) {
        if (assembly[i] == NULL) {
            continue;
        }
        //Newline if it's the end of an instruction
        if ((*assembly)[i+1].end == 1 ) {
            printf("\n");
        }
        switch((*assembly)[i].opcode) {
            case MOV:
                printf("    MOV");
                break;
            case REG:
                printf(" REG");
                break;
            case VAL:
                printf(" VAL");
                break;
            case STK:
                printf(" STK");
                break;
            case NUM:
                printf(" %d", (*assembly)[i].dec);
                break;
            case SYM:
                //Modifies values to print lowercase
                if ((table[(*assembly)[i].dec]+FIRST_SYM) > 90) {
                    printf(" %c", (table[(*assembly)[i].dec]+FIRST_SYM)+7);
                } else {
                    printf(" %c", table[(*assembly)[i].dec]+FIRST_SYM);
                }
                break;
            case ADD:
                printf("    ADD");
                break;
            case RET:
                printf("    RET");
                break;
            case FUNC:
                //Retrieves the current symbol table to print
                objdump_symbol_table(assembly, table, i);
                printf("FUNC LABEL %d", (*assembly)[i].dec);
                break;
            case PRINT:
                printf("    PRINT");
                break;
            case PTR:
                printf(" PTR");
                break;
            case EQU:
                printf("    EQU");
                break;
            case NOT:
                printf("    NOT");
                break;
            case REF:
                printf("    REF");
                break;
            case CAL:
                printf("    CAL");
                break;
            default:
                break;
        }
    }
}   

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "Insufficient args!\n");
        exit(1);
    }
    //Retrieving binary data
    char binary_data[BUFLEN] = { 'f' };
    int size = read_hex(binary_data, argv[1]);

    //Retrieving ASM data
    struct code pre_asm[ASMLEN];
    binary_asm(pre_asm, binary_data, (size));

    //Modifying the state of ASM for easy iteration
    struct code* assembly[ASMLEN] = { NULL };
    BYTE check = 1;
    for (int i = ASMLEN-1; i >= 0; i--) {
        if (pre_asm[i].opcode == FUNC) {
            check = 0;
        } else if (check == 1) {
            continue;
        }
        assembly[i] = &pre_asm[i];
    }
    //Prints values
    print_asm(assembly, ASMLEN);
    return 0;
}   