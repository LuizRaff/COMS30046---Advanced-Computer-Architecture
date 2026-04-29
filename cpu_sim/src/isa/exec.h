#ifndef ISA_EXEC_H
#define ISA_EXEC_H

#include "../core/memory.h"
#include "../core/types.h"
#include "instruction.h"

uint32_t addr_of(word_t base, word_t imm);
word_t do_alu(opcode_t op, word_t a, word_t b, word_t imm);
word_t do_ld(word_t base, word_t imm, memory_t *mem);
void do_st(word_t base, word_t imm, word_t val, memory_t *mem);

/* Vector helpers — operate on VLEN-element arrays */
void do_vadd(word_t *dst, const word_t *a, const word_t *b);
void do_vmul(word_t *dst, const word_t *a, const word_t *b);

#endif