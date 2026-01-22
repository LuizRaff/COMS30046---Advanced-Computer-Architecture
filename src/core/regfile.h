#ifndef CORE_REGFILE_H
#define CORE_REGFILE_H

#include "types.h"

typedef struct {
    word_t r[NUM_REGS];
} regfile_t;

void   regfile_init(regfile_t *rf);
word_t regfile_read(const regfile_t *rf, int idx);
void   regfile_write(regfile_t *rf, int idx, word_t value);

#endif // CORE_REGFILE_H