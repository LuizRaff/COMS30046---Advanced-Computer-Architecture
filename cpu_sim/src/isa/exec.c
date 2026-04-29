#include "exec.h"
#include <inttypes.h>
#include <stdio.h>

uint32_t addr_of(word_t base, word_t imm) {
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

word_t do_alu(opcode_t op, word_t a, word_t b, word_t imm) {
  switch (op) {
  case OP_ADD:
    return (word_t)(a + b);
  case OP_ADDI:
    return (word_t)(a + imm);
  case OP_SUB:
    return (word_t)(a - b);
  case OP_SUBI:
    return (word_t)(a - imm);
  case OP_MUL:
    return (word_t)(a * b);
  case OP_MULI:
    return (word_t)(a * imm);
  case OP_AND:
    return (word_t)(a & b);
  case OP_OR:
    return (word_t)(a | b);
  case OP_XOR:
    return (word_t)(a ^ b);
  case OP_NOT:
    return (word_t)(~a);
  case OP_CMP:
    if (a < b)
      return (word_t)(-1);
    else if (a == b)
      return (word_t)(0);
    else
      return (word_t)(1);
  case OP_SHR: {
    uint32_t sh = (uint32_t)imm & 31u;
    return (word_t)(a >> sh);
  }
  case OP_SHL: {
    uint32_t sh = (uint32_t)imm & 31u;
    return (word_t)((uint32_t)a << sh);
  }
  case OP_LDC:
    return imm;
  default:
    return 0;
  }
}

word_t do_ld(word_t base, word_t imm, memory_t *mem) {
  uint32_t addr = addr_of(base, imm);
  return mem_load(mem, addr);
}

void do_st(word_t base, word_t imm, word_t val, memory_t *mem) {
  uint32_t addr = addr_of(base, imm);
  mem_store(mem, addr, val);
}

void do_vadd(word_t *dst, const word_t *a, const word_t *b) {
  for (int i = 0; i < VLEN; i++)
    dst[i] = a[i] + b[i];
}

void do_vmul(word_t *dst, const word_t *a, const word_t *b) {
  for (int i = 0; i < VLEN; i++)
    dst[i] = a[i] * b[i];
}
