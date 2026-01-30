#ifndef ISA_OPCODE_H
#define ISA_OPCODE_H

typedef enum {
    OP_LD,
    OP_LDC,
    OP_ST,

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
    OP_SHL,

    // Stage 1.3: control-flow
    OP_B,      // absolute branch to immediate address
    OP_J,      // relative jump by immediate offset
    OP_BLTH,   // absolute branch if rs1 < rs2

    // convenience (optional)
    OP_HALT
} opcode_t;

#endif // ISA_OPCODE_H
