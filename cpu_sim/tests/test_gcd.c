#include <stdio.h>
#include <stdint.h>
#include "core/cpu.h"
#include "core/regfile.h"

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) { printf("Failed to init CPU\n"); return 1; }

    const instr_t program[] = {
        { .op=OP_LDC, .rd=1, .imm=48, .has_imm=true }, // a
        { .op=OP_LDC, .rd=2, .imm=18, .has_imm=true }, // b

        // loop PC=2
        { .op=OP_BLTH, .rs1=1, .rs2=2, .imm=5, .has_imm=true }, // if a<b goto less
        { .op=OP_BLTH, .rs1=2, .rs2=1, .imm=7, .has_imm=true }, // if b<a goto greater
        { .op=OP_B,    .imm=9, .has_imm=true },                 // equal -> end

        // less: b = b - a
        { .op=OP_SUB,  .rd=2, .rs1=2, .rs2=1, .has_imm=false },
        { .op=OP_B,    .imm=2, .has_imm=true },

        // greater: a = a - b
        { .op=OP_SUB,  .rd=1, .rs1=1, .rs2=2, .has_imm=false },
        { .op=OP_B,    .imm=2, .has_imm=true },

        // end
        { .op=OP_HALT }
    };

    int r = cpu_run(&cpu, program, sizeof(program)/sizeof(program[0]), 1000000);
    if (r == -1) { printf("CPU fault\n"); cpu_free(&cpu); return 1; }

    printf("gcd = %d (expected 6)\n", (int)regfile_read(&cpu.rf, 1));
    cpu_print_stats(&cpu);
    cpu_free(&cpu);
    return 0;
}
