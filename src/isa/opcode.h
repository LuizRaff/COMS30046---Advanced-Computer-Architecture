#ifndef ISA_OPCODE_H
#define ISA_OPCODE_H

typedef enum {
    OP_ADD,
    OP_ADDI,
    OP_SUB,
    OP_SUBI,
    OP_MUL,
    OP_MULI,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_CMP,
    OP_SHR,
    OP_SHL
} opcode_t;

#endif // ISA_OPCODE_H