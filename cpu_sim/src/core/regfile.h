#ifndef CORE_REGFILE_H
#define CORE_REGFILE_H

#include "types.h"

typedef struct {
  Word r[NUM_REGS];
} RegisterFile;

void regs_clear(RegisterFile *rf);
Word reg_get(const RegisterFile *rf, int idx);
void reg_set(RegisterFile *rf, int idx, Word value);

#endif