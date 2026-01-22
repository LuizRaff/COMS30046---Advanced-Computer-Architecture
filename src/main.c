#include <stdio.h>
#include "core/regfile.h"
#include "isa/exec.h"

int main(void) {
    regfile_t rf;
    regfile_init(&rf);

    regfile_write(&rf, 1, 10);
    regfile_write(&rf, 2, 32);

    instr_t inst = {
        .op = OP_ADD,
        .rd = 3,
        .rs1 = 1,
        .rs2 = 2,
        .imm = 0,
        .has_imm = false
    };

    exec_instr(&inst, &rf);
    printf("r3 = %d (expected 42)\n", regfile_read(&rf, 3));
    
    return 0;
}
