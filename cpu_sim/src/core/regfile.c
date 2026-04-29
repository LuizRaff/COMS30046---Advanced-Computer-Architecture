#include "regfile.h"
#include <string.h>

void regs_clear(RegisterFile *rf) {
  memset(rf->r, 0, sizeof(rf->r));
  rf->r[0] = 0;
}

Word reg_get(const RegisterFile *rf, int idx) {
  if (idx < 0 || idx >= NUM_REGS)
    return 0;
  return rf->r[idx];
}

void reg_set(RegisterFile *rf, int idx, Word value) {
  if (idx <= 0 || idx >= NUM_REGS)
    return;
  rf->r[idx] = value;
}
