#include "regfile.h"
#include <string.h>

void regfile_init(regfile_t *rf) {
    memset(rf->r, 0, sizeof(rf->r));
    // Optional convention: r0 is always 0
    rf->r[0] = 0;
}

word_t regfile_read(const regfile_t *rf, int idx) {
    if (idx < 0 || idx >= NUM_REGS) return 0; // simple guard
    return rf->r[idx];
}

void regfile_write(regfile_t *rf, int idx, word_t value) {
    if (idx <= 0 || idx >= NUM_REGS) return; // keep r0 == 0 (optional)
    rf->r[idx] = value;
}
