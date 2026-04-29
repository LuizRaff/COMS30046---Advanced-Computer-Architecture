#include "exec.h"
#include <inttypes.h>
#include <stdio.h>

/* Calculates a memory address from a base and an immediate offset, preventing overflow */
uint32_t addr_of(Word base, Word imm) {
  int64_t b = (int64_t)(uint32_t)base;
  int64_t off = (int64_t)imm;
  int64_t addr = b + off;

  if (addr < 0 || addr > 0xFFFFFFFFLL) {
    printf("Address overflow/underflow: base=%" PRId64 " off=%" PRId64 "\n", b,
           off);
    return 0;
  }
  return (uint32_t)addr;
}

/* Executes an Arithmetic Logic Unit operation given the opcode and operands */
Word do_alu(OpCode op, Word a, Word b, Word imm) {
  switch (op) {
  case OP_ADD:
    return (Word)(a + b);
  case OP_ADDI:
    return (Word)(a + imm);
  case OP_SUB:
    return (Word)(a - b);
  case OP_SUBI:
    return (Word)(a - imm);
  case OP_MUL:
    return (Word)(a * b);
  case OP_MULI:
    return (Word)(a * imm);
  case OP_AND:
    return (Word)(a & b);
  case OP_OR:
    return (Word)(a | b);
  case OP_XOR:
    return (Word)(a ^ b);
  case OP_NOT:
    return (Word)(~a);
  case OP_CMP:
    if (a < b)
      return (Word)(-1);
    else if (a == b)
      return (Word)(0);
    else
      return (Word)(1);
  case OP_SHR: {
    uint32_t sh = (uint32_t)imm & 31u;
    return (Word)(a >> sh);
  }
  case OP_SHL: {
    uint32_t sh = (uint32_t)imm & 31u;
    return (Word)((uint32_t)a << sh);
  }
  case OP_LDC:
    return imm;
  default:
    return 0;
  }
}

/* Simulates a Load operation from memory */
Word do_ld(Word base, Word imm, MemoryBank *mem) {
  uint32_t addr = addr_of(base, imm);
  return mem_load(mem, addr);
}

/* Simulates a Store operation to memory */
void do_st(Word base, Word imm, Word val, MemoryBank *mem) {
  uint32_t addr = addr_of(base, imm);
  mem_store(mem, addr, val);
}

/* Performs a vector addition operation on an entire vector register */
void do_vadd(Word *dst, const Word *a, const Word *b) {
  for (int i = 0; i < VLEN; i++)
    dst[i] = a[i] + b[i];
}

/* Performs a vector multiplication operation on an entire vector register */
void do_vmul(Word *dst, const Word *a, const Word *b) {
  for (int i = 0; i < VLEN; i++)
    dst[i] = a[i] * b[i];
}
