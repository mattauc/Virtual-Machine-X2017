#include "parser.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

//Increments the program counter and retrieves source/destination/values
void retrieve(struct vm* vm) {

    if ((vm->registers[REG7] + 1) < vm->pgm_size) {
        vm->registers[REG7] += 1;
    }
    
    vm->op = vm->assembly[vm->registers[REG7]].opcode;
    if (vm->assembly[vm->registers[REG7]].args >= 3) {
        vm->dest = vm->assembly[vm->registers[REG7]].type1;
        vm->data1 = vm->assembly[vm->registers[REG7]].value1;

         if (vm->assembly[vm->registers[REG7]].args == 5) {
            vm->src = vm->assembly[vm->registers[REG7]].type2;
            vm->data2 = vm->assembly[vm->registers[REG7]].value2;
        }
    }
}

//Handles MOV source content
void mov_function(struct vm* vm, BYTE* storage, int offset) {

    switch(vm->src) {
        case REG:
            *storage = vm->registers[vm->data2];
            break;
        case VAL:
            *storage = vm->data2;
            break;
        case PTR:
            *storage = vm->RAM[vm->RAM[(vm->registers[REG5]+1)+offset]];
            break;
        case STK:
            *storage = vm->RAM[(vm->registers[REG5]+1)+offset];
            break;
        default:
            break;
    }
}

//Handles MOV destination content
void mov(struct vm* vm, int offset1, int offset2) {

    switch(vm->dest) {
        case REG:
            mov_function(vm, &vm->registers[vm->data1], offset2);
            break;
        case PTR:
            mov_function(vm, &vm->RAM[vm->RAM[(vm->registers[REG5]+1)+offset1]], offset2);
            break;
        case STK:
            mov_function(vm, &vm->RAM[(vm->registers[REG5]+1)+offset1], offset2);
            vm->registers[REG6] += 1;
            break;
        default:
            break;
    }
}

//Prints the value
void print(struct vm* vm, int offset1) {

    if (vm->dest == REG) {
        printf("%d\n",vm->registers[vm->data1]);

    } else if (vm->dest == STK) {
        printf("%d\n",vm->RAM[(vm->registers[REG5]+1)+offset1]);
                
    } else if (vm->dest == VAL) {
        printf("%d\n",vm->data1);

    } else if (vm->dest == PTR) {
        printf("%d\n",vm->RAM[vm->RAM[(vm->registers[REG5]+1)+offset1]]);
    }
}

//Updates stack frames
void cal(struct vm* vm) {

    //Organizing RAM to track return addresses
    vm->RAM[vm->registers[REG6]] = vm->registers[REG7];
    vm->RAM[vm->registers[REG6] += 1] = vm->registers[REG5];
    vm->registers[REG6] += 1;

    int func_check = 0;
    //Locating the position of the next function in program code
    for (int i = 0; i < vm->pgm_size; i++) {
        if (vm->assembly[i].opcode == FUNC && vm->assembly[i].value1 == vm->data1) {
            vm->registers[REG7] = i;
            if (func_check) {
                func_check = 0;
            } else {
                func_check = 1;
            }
        }
    }
    if (func_check == 0 || vm->data1 >= 8) {
        fprintf(stderr,"Problem with function call!\n");
        exit(1);
    }
    //Updating current frame symbols and further stack modification
    vm_symbol_table(vm->assembly, vm->sym_table, vm->registers[REG7], vm->pgm_size);
    vm->registers[REG5] = vm->registers[REG6]-1;
    vm->op = vm->assembly[vm->registers[REG7]].opcode;

}

//Handles REF source and destination content
void ref(struct vm* vm, int offset1, int offset2) {

        if (vm->dest == REG) {
            if (vm->src == PTR) {
                vm->registers[vm->data1] = vm->RAM[(vm->registers[REG5]+1)+offset2];
            } else if (vm->src== STK) {
                vm->registers[vm->data1] = (vm->registers[REG5]+1)+offset2;
            }
        } else if (vm->dest == PTR) {
            if (vm->src == PTR) {
                vm->RAM[vm->RAM[(vm->registers[REG5]+1)+offset1]] = vm->RAM[(vm->registers[REG5]+1)+offset2];
            } else if (vm->src == STK) {
                vm->RAM[vm->RAM[(vm->registers[REG5]+1)+offset1]] = (vm->registers[REG5]+1)+offset2;
            }
        } else {
             if (vm->src == PTR) {
                vm->RAM[(vm->registers[REG5]+1)+offset1] = vm->RAM[(vm->registers[REG5]+1)+offset2];
            } else if (vm->src == STK) {
                vm->RAM[(vm->registers[REG5]+1)+offset1] = (vm->registers[REG5]+1)+offset2;
            }
        }
}

//Adds REG values, updating opcode if REG 7 is manipulated
void add(struct vm* vm) {

    if (vm->data1 == 7) {
        vm->registers[REG7] += vm->registers[vm->data2];
        vm->op = vm->assembly[vm->registers[REG7]].opcode;
    } else {
        vm->registers[vm->data1] = (vm->registers[vm->data1] + vm->registers[vm->data2]);
    }
}

//Performs EQU functionality
void equ(struct vm* vm) {

    if (vm->registers[vm->data1] == 0) {
        vm->registers[vm->data1] = 1;
    } else {
        vm->registers[vm->data1] = 0;
    }
}

//Handles RET functionality
void ret(struct vm* vm) {

    //Resets core REG values to the previous stack frame
    if (vm->registers[REG5] >= 1) {
        vm->registers[REG7] = vm->RAM[vm->registers[REG5]-1];
        vm->registers[REG6] = vm->registers[REG5]-1;
    }
    vm->registers[REG5] = vm->RAM[vm->registers[REG5]];

    //Collects current frame symbol table and updates opcode
    int start_location = vm->registers[REG7];
    while (vm->assembly[start_location].opcode != FUNC) {
        start_location--;
    }
    vm_symbol_table(vm->assembly,vm->sym_table, start_location, vm->pgm_size);
    vm->op = vm->assembly[vm->registers[REG7]].opcode;
}

//Opcode control centre, calls functionality based off opcode.
void execute(struct vm* vm) {

    //Pre-determines symbol offset before functionality
    int offset1 = 0;
    int offset2 = 0;
    if (vm->assembly[vm->registers[REG7]].args >= 3 && (vm->dest == PTR || vm->dest== STK)) {
        offset1 = vm->sym_table[vm->data1];
    }
    if (vm->assembly[vm->registers[REG7]].args>= 5 && (vm->src == PTR || vm->src == STK)) {
        offset2 = vm->sym_table[vm->data2];
    } 

    switch(vm->op) {
        case MOV:
            mov(vm, offset1, offset2);
            break;
        case PRINT:
            print(vm, offset1);
            break;
        case CAL:
            cal(vm);
            break;
        case REF:
            ref(vm, offset1, offset2);
            break;
        case ADD:
            add(vm);
            break;
        case NOT:
            vm->registers[vm->data1] = ~vm->registers[vm->data1]; 
            break;
        case EQU:
            equ(vm);
            break;
        case RET:
            ret(vm);
            break;
        default:
            break;
    }
}

//Program retrieval and execution (Program loop)
void run(struct vm* vm) {

    while (1) {
        if (vm->registers[REG5] == 0 && vm->op == RET) {
            break;
        }
        if (vm->registers[REG6] == RAM_SIZE-1) {
            fprintf(stderr, "Stack overflow!\n");
            exit(1);
        }
        if (vm->registers[REG7] <= vm->pgm_size) {
            retrieve(vm);
            execute(vm);
        } else {
            break;
        }
    }
}

//Constructing my VM
void create_vm(struct vm* vm, struct assm* assembly, int* func_indices) {
    /*
    REG7 = PROGRAM_COUNT
    REG6 = STACK POINTER
    REG5 = FRAME POINTER
    */
    vm->assembly = assembly;
    vm->registers[REG7] = 255;
    for (int i = 0; i < 7; i++) {
        if (assembly[func_indices[i]].value1 == 0 && (func_indices[i] >= 0)) {
            vm->registers[REG7] = func_indices[i];
        }
    }
    if (vm->registers[REG7] == 255) {
        fprintf(stderr, "Main function does not exist!\n");
        exit(1);
    }
    vm->op = vm->assembly[vm->registers[REG7]].opcode;
    vm_symbol_table(vm->assembly,vm->sym_table, vm->registers[REG7], vm->pgm_size);
    vm->registers[REG6] = 1;
    vm->registers[REG5] = 0;
    vm->RAM[0] = DEFAULT;
}

//Building components of the program code
void build_program(struct assm* assembly, struct code* pre_asm, struct assm* code, int* assm_index, int* i) {

    //Assigns values based off opcode argument size
    if (pre_asm[*i].args >= 3) {
        code->opcode = pre_asm[*i].opcode;
        code->args = pre_asm[*i].args;
        code->type1 = pre_asm[*i-1].opcode;
        code->value1 = pre_asm[*i-2].dec;
        assembly[*assm_index] = *code;

        if (pre_asm[*i].args == 5) {
            assembly[*assm_index].type2 = pre_asm[*i-3].opcode;
            assembly[*assm_index].value2 = pre_asm[*i-4].dec;
        }
        *i -= pre_asm[*i].args-1;
        *assm_index += 1;

    //Assigning RET values
    } else if (pre_asm[*i].opcode == RET) {
        code->args = pre_asm[*i].args;
        code->opcode = pre_asm[*i].opcode;
        assembly[*assm_index] = *code;

        i -= pre_asm[*i].args-1;
        *assm_index += 1;
    }
}

//Function to build program code
void define_asm(struct vm* vm, int* func_indices, struct code* pre_asm, struct assm* assembly) {
    
    int func_index = DEFAULT;
    int assm_index = DEFAULT;
    int check = 0;

    for (int i = ASMLEN-1; i >= 0; i--) {
        struct assm code;
        if (pre_asm[i].opcode == FUNC) {
            code.opcode = pre_asm[i].opcode;
            code.args = pre_asm[i].args;
            code.value1 = pre_asm[i].dec;
            assembly[assm_index] = code;
            func_indices[func_index] = assm_index;
            
            check = 1;
            func_index++;
            assm_index++;
            i -= pre_asm[i].args-1;
        
        //Skips garbage values
        } else if (check == 0) {
            continue;
        }
        build_program(assembly, pre_asm, &code, &assm_index, &i);
    }
    vm->pgm_size = assm_index;
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

    struct assm assembly[RAM_SIZE];
    int func_indices[7] = {-1};
    struct vm vm;
    
    //Defining, creating and running program
    define_asm(&vm, func_indices, pre_asm, assembly);
    create_vm(&vm, assembly, func_indices);
    run(&vm);

    return 0;
}   