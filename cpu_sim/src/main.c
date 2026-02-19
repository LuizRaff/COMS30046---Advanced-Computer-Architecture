#include <stdio.h>
#include <stdint.h>

#include "core/cpu.h"
#include "core/memory.h"

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) {
        printf("Failed to init CPU\n");
        return 1;
    }

    memory_store_w(&cpu.mem, 8, 1234);

    const instr_t program[] = {
        { .op=OP_LDC, .rd=1,  .imm=8,  .has_imm=true },
        { .op=OP_LD,  .rd=2,  .rs1=1,  .imm=0,  .has_imm=true },

        { .op=OP_B,   .imm=4, .has_imm=true },
        { .op=OP_LDC, .rd=4,  .imm=111, .has_imm=true }, // skipped
        { .op=OP_LDC, .rd=4,  .imm=222, .has_imm=true }, // executed

        { .op=OP_LDC, .rd=10, .imm=0,  .has_imm=true },  // r10 = 0
        { .op=OP_LDC, .rd=11, .imm=5,  .has_imm=true },  // r11 = 5
        { .op=OP_LDC, .rd=12, .imm=1,  .has_imm=true },  // r12 = 1
        { .op=OP_ADD, .rd=10, .rs1=10, .rs2=12, .has_imm=false }, // r10++
        { .op=OP_BLTH,.rs1=10,.rs2=11, .imm=8,  .has_imm=true },  // if r10 < r11 goto PC=8

        { .op=OP_J,   .imm=2, .has_imm=true },
        { .op=OP_LDC, .rd=5,  .imm=999, .has_imm=true }, // skipped
        { .op=OP_LDC, .rd=5,  .imm=123, .has_imm=true }, // executed

        { .op=OP_HALT }
    };

    int r = cpu_run(&cpu, program, sizeof(program)/sizeof(program[0]), 1000000);
    if (r == -1) {
        printf("CPU fault while running\n");
        cpu_free(&cpu);
        return 1;
    }

    printf("r2=%d (expected 1234)\n", regfile_read(&cpu.rf, 2));
    printf("r4=%d (expected 222)\n",  regfile_read(&cpu.rf, 4));
    printf("r10=%d (expected 5)\n",   regfile_read(&cpu.rf, 10));
    printf("r5=%d (expected 123)\n",  regfile_read(&cpu.rf, 5));

    cpu_print_stats(&cpu);

    cpu_free(&cpu);
    return 0;
}
