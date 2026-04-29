#ifndef ISA_INSTRUCTION_H
#define ISA_INSTRUCTION_H

#include "../core/types.h"
#include "opcode.h"

typedef struct {
  OpCode op;
  int rd;
  int rs1;
  int rs2;
  Word imm;
  bool has_imm;
} Instruction;

#endif 