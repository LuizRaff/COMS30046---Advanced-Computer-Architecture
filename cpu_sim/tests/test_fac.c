#include <stdio.h>
#include <stdint.h>
#include "../src/core/cpu.h"
#include "../src/core/regfile.h"

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) { printf("Failed to init CPU\n"); return 1; }

    instr_t program[] = {
        { .op=OP_LDC, .rd=1, .imm=5, .has_imm=true }, // n=5
        { .op=OP_LDC, .rd=2, .imm=1, .has_imm=true }, // i=1
        { .op=OP_LDC, .rd=3, .imm=1, .has_imm=true }, // res=1
        { .op=OP_LDC, .rd=4, .imm=6, .has_imm=true }, // n+1 = 6

        { .op=OP_BLTH, .rs1=2, .rs2=4, .imm=6, .has_imm=true }, // if (i<6) body
        { .op=OP_B,    .imm=9, .has_imm=true },                 // else end

        { .op=OP_MUL,  .rd=3, .rs1=3, .rs2=2, .has_imm=false }, // res *= i
        { .op=OP_ADDI, .rd=2, .rs1=2, .imm=1, .has_imm=true },  // i++
        { .op=OP_B,    .imm=4, .has_imm=true },                 // back
        { .op=OP_HALT }
    };

    int r = cpu_run(&cpu, program, sizeof(program)/sizeof(program[0]), 1000000);
    if (r == -1) { printf("CPU fault\n"); cpu_free(&cpu); return 1; }

    printf("fact(5) = %d (expected 120)\n", (int)regfile_read(&cpu.rf, 3));
    cpu_print_stats(&cpu);
    cpu_free(&cpu);
    return 0;
}
