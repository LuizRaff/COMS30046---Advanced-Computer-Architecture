#ifndef ISA_INSTRUCTION_H
#define ISA_INSTRUCTION_H

#include "../core/types.h"
#include "opcode.h"

typedef struct {
    opcode_t op;
    int rd;
    int rs1;
    int rs2;
    word_t imm;
    bool has_imm;
} instr_t;

#endif // ISA_INSTRUCTION_H