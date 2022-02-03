#ifndef PARSER_H
#define PARSER_H
#define BYTE unsigned char
#define TABLE_SIZE 34
#define BYTE_SIZE 8
#define OPCODE 2
#define BUFLEN 6000
#define ASMLEN 1400
#define RAM_SIZE 256
#define FIRST_SYM 65
#define RESET 0
#define DEFAULT 0

enum instr {
    MOV, CAL, RET,
    REF, ADD, PRINT,
    NOT, EQU, VAL,
    REG, STK, PTR, NUM,
    SYM, FUNC
};

struct code {
    enum instr opcode: 4;
    
    BYTE args: 3;
    BYTE dec: 8;
    BYTE end: 1;
};

struct assm {

    enum instr opcode: 4;
    BYTE args: 3;

    enum instr type1: 4;
    enum instr type2: 4;

    BYTE value1;
    BYTE value2;
};

int read_hex(char* binary_data, const char* directory);

void binary_asm(struct code* assembly, char* binary, int size);

void objdump_symbol_table(struct code** assembly, int* table, int start);

void vm_symbol_table(struct assm* assembly, int* table, BYTE start, int pgm_size);

#endif