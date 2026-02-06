#include <stdio.h>
#include <stdint.h>
#include "core/cpu.h"
#include "core/regfile.h"

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) { printf("Failed to init CPU\n"); return 1; }

    const instr_t program[] = {
        { .op=OP_LDC, .rd=1, .imm=45, .has_imm=true }, // x
        { .op=OP_LDC, .rd=2, .imm=1,  .has_imm=true }, // mask=1
        { .op=OP_LDC, .rd=3, .imm=0,  .has_imm=true }, // count=0

        // loopcheck PC=3: if (0 < x) body else end
        { .op=OP_BLTH, .rs1=0, .rs2=1, .imm=5, .has_imm=true },
        { .op=OP_B,    .imm=9, .has_imm=true },

        // body PC=5
        { .op=OP_AND,  .rd=4, .rs1=1, .rs2=2, .has_imm=false }, // bit = x & 1
        { .op=OP_ADD,  .rd=3, .rs1=3, .rs2=4, .has_imm=false }, // count += bit
        { .op=OP_SHR,  .rd=1, .rs1=1, .imm=1, .has_imm=true },  // x >>= 1
        { .op=OP_B,    .imm=3, .has_imm=true },

        // end PC=9
        { .op=OP_HALT }
    };

    int r = cpu_run(&cpu, program, sizeof(program)/sizeof(program[0]), 1000000);
    if (r == -1) { printf("CPU fault\n"); cpu_free(&cpu); return 1; }

    printf("hamming(45) = %d (expected 4)\n", (int)regfile_read(&cpu.rf, 3));
    cpu_free(&cpu);
    return 0;
}
