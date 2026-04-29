#ifndef ISA_EXEC_H
#define ISA_EXEC_H

#include "../core/memory.h"
#include "../core/types.h"
#include "instruction.h"

uint32_t addr_of(Word base, Word imm);
Word do_alu(OpCode op, Word a, Word b, Word imm);
Word do_ld(Word base, Word imm, MemoryBank *mem);
void do_st(Word base, Word imm, Word val, MemoryBank *mem);

void do_vadd(Word *dst, const Word *a, const Word *b);
void do_vmul(Word *dst, const Word *a, const Word *b);

#endif