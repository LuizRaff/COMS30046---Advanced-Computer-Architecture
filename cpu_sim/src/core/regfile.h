#ifndef CORE_REGFILE_H
#define CORE_REGFILE_H

#include "types.h"

typedef struct {
  word_t r[NUM_REGS];
} regfile_t;

void regs_clear(regfile_t *rf);
word_t reg_get(const regfile_t *rf, int idx);
void reg_set(regfile_t *rf, int idx, word_t value);

#endif