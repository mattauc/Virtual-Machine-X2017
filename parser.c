#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Resets values when the argument count equals the max argument based off opcode
void reset_values(char* bin_pair, int* arg_count, int* max_args, int* bin_count, 
int* instr_count, struct code* op) {

    if (*arg_count == *max_args && op->opcode != RET) {
        *bin_count = OPCODE;
        *arg_count = RESET;
        *instr_count+=1;
        op->end = 1;
    } else if (op->opcode != RET) {
        op->end = 0;
    }
    //Resets the binary buffer
    for (int i = 0; i < BYTE_SIZE; i++) {
        bin_pair[i] = -1;
    }
}

//Determines the symbol table upon new frame for VM
void vm_symbol_table(struct assm* assembly, int* table, BYTE start, int pgm_size) {

    int sym_count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = -1;
    }
    for (int i = (int)start+1; i < RAM_SIZE; i++) {
        if (assembly[i].opcode == FUNC || i+1 == pgm_size) {
            break;
        }
        //Assigning values based off argument size and symbol
        if (assembly[i].args >= 3 && assembly[i].type1 == STK) {
            if (table[assembly[i].value1] == -1) {
                table[assembly[i].value1] = sym_count;
                sym_count++;
            }
        } 
        if (assembly[i].args == 5 && assembly[i].type2 == STK) {
            if (table[assembly[i].value2] == -1) {
                table[assembly[i].value2] = sym_count;
                sym_count++;
            }
        }
    }
}

//Determines the symbol table upon new frame for Objdump
void objdump_symbol_table(struct code** assembly, int* table, int start) {

    int sym_count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = -1;
    }
    for (int i = start-1; i >= 0; i--) {
        if (assembly[i]->opcode == FUNC) {
            break;
        //Assigning values based off SYM and decimal No.
        } else if (assembly[i]->opcode == SYM) {
            if (table[assembly[i]->dec] == -1) {
                table[assembly[i]->dec] = sym_count;
                sym_count++;
            }
        }
    }
}

/*
* Assembles the current instruction based off binary combination
* Updates the assembly index, determines argument size and sets values
*/
void assemble(char* bin_pair, struct code* op, struct code* previous, int* bin_count, int* arg_count, int* max_args, 
int* index, int* instr_count) {
        
    int num_check = 1;

    if (previous != NULL) {
        char* ptr;
        switch(previous->opcode) {
        case REG:
            *bin_count = 1;
            op->opcode = NUM;
            op->dec = strtol(bin_pair, &ptr, 2);
            break;
        case VAL:
            *bin_count = 1;
            op->opcode = NUM;
            op->dec = strtol(bin_pair, &ptr, 2);
            break;
        case STK:
            *bin_count = 1;
            op->opcode = SYM;
            op->dec = strtol(bin_pair, &ptr, 2);
            break;
        case PTR:
            *bin_count = 1;
            op->opcode = SYM;
            op->dec = strtol(bin_pair, &ptr, 2);
            break;
        default:
            num_check = 0;
        }
        if (previous->opcode == FUNC) {
            *index-=1;
        }
    } 
    if (bin_pair[0] == '0') {
        bin_pair[0] = '2';
    }

    int bin_value = atoi(bin_pair); 
    if (num_check == 0 || previous == NULL) {
        switch(bin_value) {
        case 20: //00
            *bin_count = 7;
            op->opcode = VAL;
            break;
        case 21: //01
            *bin_count = 2;
            op->opcode = REG;
            break;
        case 10: //10
            *bin_count = 4;
            op->opcode = STK;
            break;
        case 11: //11
            *bin_count = 4;
            op->opcode = PTR;
            break;
        case 200: //000
            *bin_count = 1;
            *max_args = 5;
            *index += (*max_args);
            op->opcode = MOV;
            op->args = *max_args;
            break;
        case 201: //001
            *bin_count = 1;
            *max_args = 3;
            *index += (*max_args);
            op->opcode = CAL;
            op->args = *max_args;
            break;
        case 210: //010
            op->opcode = RET;
            *arg_count = RESET;
            *bin_count = OPCODE;
            *instr_count+=1;
            op->end = 1;
            op->args = 1;
            if (previous != NULL) {
                *index+=1;
            }
            break;
        case 211: //011
            *bin_count = 1;
            *max_args = 5;
            *index += (*max_args);
            op->opcode = REF;
            op->args = *max_args;
            break;
        case 100: //100
            *bin_count = 1;
            *max_args = 5;
            *index += (*max_args);
            op->opcode = ADD;
            op->args = *max_args;
            break;
        case 101: //101
            *bin_count = 1;
            *max_args = 3;
            *index += (*max_args);
            op->opcode = PRINT;
            op->args = *max_args;
            break;
        case 110: //110
            *bin_count = 1;
            *max_args = 3;
            *index += (*max_args);
            op->opcode = NOT;
            op->args = *max_args;
            break;
        case 111: //111
            *bin_count = 1;
            *max_args = 3;
            *index += (*max_args);
            op->opcode = EQU;
            op->args = *max_args;
            break;  
        }
    }
    //Checks whether conditions need to be reset 
    reset_values(bin_pair, arg_count, max_args, bin_count, instr_count, op);
} 

//Determines the amount of isntructions present in a function
void find_function_instructions(char* binary, long* max_instr, int size) {

    char binary_buff[6] = {-1};
    int index = 4;

    for (int i = size; i > size-5; i--) {
        binary_buff[index] = binary[i]+'0';
        index--;
    }
    char* ptr;
    *max_instr = strtol(binary_buff, &ptr, 2);
}

//Evaluates function properties and creates an ASM code
void function_properties(char* binary, char* binary_buff, int* instr, struct code* op, int* bin_index, 
long* max_instr, int* bin_count) {

    char* ptr;
    if (*bin_index >= 4) {
        find_function_instructions(binary, max_instr, *bin_index);
    }

    *bin_count = OPCODE;
    *instr = RESET;
    op->opcode = FUNC;
    op->args = 1;
    op->dec = strtol(binary_buff, &ptr, 2);

    if (*bin_index-5 >= 0) {
        *bin_index = *bin_index-5;
    }
}

//Applies an offset so that instruction contents are stored in reverse
void apply_offset(int* offset, struct code* op, int* arg_count) {

    if (!op->end) {
        *offset+=1;
    } else {
        *offset = RESET;
    }
    *arg_count+=1;
}

//Assembles program code
void binary_asm(struct code* assembly, char* binary, int size) {
    
    char binary_buff[9] = {-1};
    int max_args = DEFAULT;
    int arg_count = DEFAULT;;
    int index = DEFAULT;
    int offset = DEFAULT;
    int bin_count = OPCODE;
    int instr_count = DEFAULT;
    long max_instr;

    find_function_instructions(binary, &max_instr, size-1);
    struct code* previous = NULL;
    for (int i = size-6; i >= -1; i--) { 
 
        struct code op;
        //Checks if the binary buffer is full, based on argument size
        if (bin_count == -1) {
            //Resets FUNC code
            if ((instr_count == max_instr)) {
                function_properties(binary, binary_buff, &instr_count, &op, &i, 
                &max_instr, &bin_count);
                
                assembly[index+1] = op;
                if (i-5 > 0) {
                    index+=2;
                }
            //Assembles opcode    
            } else {
                assemble(binary_buff, &op, previous, &bin_count, &arg_count, &max_args, 
                &index, &instr_count);

                assembly[index-offset] = op;
                apply_offset(&offset, &op, &arg_count);
            }
            previous = &op;
        }
        if (i >= 0) {
            binary_buff[bin_count] = binary[i]+'0';
        }
        bin_count--;
    }
}

//Converts decimal to binary
void dec_binary(int* binary, int num) {

    int number_bin[8];
    int i = 0;
    while (num > 0) {
        number_bin[i] = num % 2;
        num = num / 2;
        i++;
    }
    //Stores values in reverse, then fills the 0s
    for (int j = i-1; j >= 0; j--) {
        binary[j] = number_bin[j];
    }
    for (int k = i; k < BYTE_SIZE; k++) {
        binary[k] = 0;
    }
}

//Converts hex to binary and builds upon an array
void hex_binary(unsigned char* line, char* binary_data, int cursor) {

    int binary[8];
    dec_binary(binary, line[0]);

    for (int j = 1; j <= BYTE_SIZE; j++) {
        binary_data[cursor-j] = binary[j-1];
    }
}

//Reads the binary file
int read_hex(char* binary_data, const char* directory) {

    BYTE buffer[sizeof(BYTE)] = {0};
    FILE* file;
    if ((file = fopen(directory, "rb")) == NULL) {
        fprintf(stderr, "File does not exist!\n");
        exit(1);
    }
    int cursor = 8;

    while (fread(buffer, sizeof(BYTE), sizeof(BYTE), file)) {
        hex_binary(buffer, binary_data, cursor);
        cursor+=8;
    }
    if (cursor-8 > 5048) {
        fprintf(stderr, "Binary file exceeds capacity!\n");
        exit(1);
    }
    fclose(file);
    return cursor-8;
} 