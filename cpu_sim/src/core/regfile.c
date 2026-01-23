#include "regfile.h"
#include <string.h>

void regfile_init(regfile_t *rf) {
    memset(rf->r, 0, sizeof(rf->r));
    rf->r[0] = 0;
}

word_t regfile_read(const regfile_t *rf, int idx) {
    if (idx < 0 || idx >= NUM_REGS) return 0;
    return rf->r[idx];
}

void regfile_write(regfile_t *rf, int idx, word_t value) {
    if (idx <= 0 || idx >= NUM_REGS) return;
    rf->r[idx] = value;
}
