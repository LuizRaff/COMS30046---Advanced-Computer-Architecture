#include "regfile.h"
#include <string.h>

/* Clears all general-purpose registers, resetting them to zero */
void regs_clear(RegisterFile *rf) {
  memset(rf->r, 0, sizeof(rf->r));
  rf->r[0] = 0;
}

/* Returns the value of the register at the specified index, yielding 0 if invalid */
Word reg_get(const RegisterFile *rf, int idx) {
  if (idx < 0 || idx >= NUM_REGS)
    return 0;
  return rf->r[idx];
}

/* Sets the value of the register at the specified index, preventing writes to register 0 */
void reg_set(RegisterFile *rf, int idx, Word value) {
  if (idx <= 0 || idx >= NUM_REGS)
    return;
  rf->r[idx] = value;
}
