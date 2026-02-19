#include "exec.h"
#include <inttypes.h>
#include <stdio.h>

uint32_t addr_of(const instr_t *inst, regfile_t *rf) {
  int64_t base = (int64_t)(uint32_t)reg_get(rf, inst->rs1);
  int64_t off = (int64_t)inst->imm;
  int64_t addr = base + off;

  if (addr < 0 || addr > 0xFFFFFFFFLL) {
    printf("Address overflow/underflow: base=%" PRId64 " off=%" PRId64 "\n",
           base, off);
    return 0;
  }
  return (uint32_t)addr;
}

void do_ld(const instr_t *inst, regfile_t *rf, memory_t *mem) {
  uint32_t addr = addr_of(inst, rf);
  word_t v = mem_load(mem, addr);
  reg_set(rf, inst->rd, v);
}

void do_ldc(const instr_t *inst, regfile_t *rf) {
  reg_set(rf, inst->rd, inst->imm);
}

void do_st(const instr_t *inst, regfile_t *rf, memory_t *mem) {
  uint32_t addr = addr_of(inst, rf);
  word_t v = reg_get(rf, inst->rs2);
  mem_store(mem, addr, v);
}

void do_add(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a + b));
}

void do_addi(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  reg_set(rf, inst->rd, (word_t)(a + inst->imm));
}

void do_sub(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a - b));
}

void do_subi(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  reg_set(rf, inst->rd, (word_t)(a - inst->imm));
}

void do_mul(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a * b));
}

void do_muli(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  reg_set(rf, inst->rd, (word_t)(a * inst->imm));
}

void do_and(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a & b));
}

void do_or(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a | b));
}

void do_xor(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  reg_set(rf, inst->rd, (word_t)(a ^ b));
}

void do_not(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  reg_set(rf, inst->rd, (word_t)(~a));
}

void do_cmp(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  word_t b = reg_get(rf, inst->rs2);
  if (a < b) {
    reg_set(rf, inst->rd, (word_t)(-1));
  } else if (a == b) {
    reg_set(rf, inst->rd, (word_t)(0));
  } else {
    reg_set(rf, inst->rd, (word_t)(1));
  }
}

void do_shl(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  uint32_t sh = (uint32_t)inst->imm & 31u;
  reg_set(rf, inst->rd, (word_t)((uint32_t)a << sh));
}

void do_shr(const instr_t *inst, regfile_t *rf) {
  word_t a = reg_get(rf, inst->rs1);
  uint32_t sh = (uint32_t)inst->imm & 31u;
  reg_set(rf, inst->rd, (word_t)(a >> sh));
}

void run_instr(const instr_t *inst, regfile_t *rf, memory_t *mem) {
  switch (inst->op) {
  case OP_NOP:
    break;
  case OP_LD:
    do_ld(inst, rf, mem);
    break;
  case OP_LDC:
    do_ldc(inst, rf);
    break;
  case OP_ST:
    do_st(inst, rf, mem);
    break;
  case OP_ADD:
    do_add(inst, rf);
    break;
  case OP_ADDI:
    do_addi(inst, rf);
    break;
  case OP_SUB:
    do_sub(inst, rf);
    break;
  case OP_SUBI:
    do_subi(inst, rf);
    break;
  case OP_MUL:
    do_mul(inst, rf);
    break;
  case OP_MULI:
    do_muli(inst, rf);
    break;
  case OP_AND:
    do_and(inst, rf);
    break;
  case OP_OR:
    do_or(inst, rf);
    break;
  case OP_XOR:
    do_xor(inst, rf);
    break;
  case OP_NOT:
    do_not(inst, rf);
    break;
  case OP_CMP:
    do_cmp(inst, rf);
    break;
  case OP_SHR:
    do_shr(inst, rf);
    break;
  case OP_SHL:
    do_shl(inst, rf);
    break;
  default:
    printf("Unsupported opcode %d\n", inst->op);
    break;
  }
}
