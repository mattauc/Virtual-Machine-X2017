#ifndef STACK_h
#define STACK_H
#define REG7 7
#define REG6 6
#define REG5 5
#include "parser.h"

struct vm {

    struct assm* assembly;

    BYTE registers[8];
    BYTE RAM[RAM_SIZE];

    enum instr op;
    enum instr dest;
    enum instr src;

    BYTE data1;
    BYTE data2;

    int sym_table[TABLE_SIZE];
    int pgm_size;
};

#endif