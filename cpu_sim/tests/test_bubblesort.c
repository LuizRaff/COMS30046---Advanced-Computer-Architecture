#include <stdio.h>
#include <stdint.h>
#include "../src/core/cpu.h"
#include "../src/core/memory.h"

#define A_BASE 0x0000
#define N 10

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) { printf("Failed to init CPU\n"); return 1; }

    int init[N] = {9,1,8,2,7,3,6,4,5,0};
    for (int i = 0; i < N; i++) memory_store_w(&cpu.mem, (uint32_t)(A_BASE + i*4), (word_t)init[i]);

    instr_t program[] = {
        { .op=OP_LDC, .rd=1, .imm=A_BASE, .has_imm=true }, // r1=base
        { .op=OP_LDC, .rd=3, .imm=N,      .has_imm=true }, // r3=N
        { .op=OP_LDC, .rd=11,.imm=N-1,    .has_imm=true }, // r11=9
        { .op=OP_LDC, .rd=2, .imm=0,      .has_imm=true }, // i=0

        { .op=OP_SUB, .rd=5, .rs1=11,.rs2=2, .has_imm=false }, // limit = (N-1) - i
        { .op=OP_LDC, .rd=4, .imm=0,  .has_imm=true },          // j=0
        { .op=OP_ADDI,.rd=6, .rs1=1, .imm=0, .has_imm=true },   // ptr=base

        { .op=OP_LD,  .rd=7, .rs1=6, .imm=0, .has_imm=true },   // x = *ptr
        { .op=OP_ADDI,.rd=12,.rs1=6, .imm=4, .has_imm=true },   // ptr2=ptr+4
        { .op=OP_LD,  .rd=8, .rs1=12,.imm=0, .has_imm=true },   // y = *ptr2

        { .op=OP_BLTH,.rs1=8, .rs2=7, .imm=12, .has_imm=true }, // goto swap
        { .op=OP_B,   .imm=14, .has_imm=true },                 // skip swap -> cont

        { .op=OP_ST,  .rs1=6,  .rs2=8, .imm=0, .has_imm=true }, // *ptr  = y
        { .op=OP_ST,  .rs1=12, .rs2=7, .imm=0, .has_imm=true }, // *ptr2 = x

        { .op=OP_ADDI,.rd=6, .rs1=6, .imm=4, .has_imm=true },   // ptr += 4
        { .op=OP_ADDI,.rd=4, .rs1=4, .imm=1, .has_imm=true },   // j++
        { .op=OP_BLTH,.rs1=4, .rs2=5, .imm=7, .has_imm=true },  // if (j<limit) inner

        { .op=OP_ADDI,.rd=2, .rs1=2, .imm=1, .has_imm=true },   // i++
        { .op=OP_BLTH,.rs1=2, .rs2=11,.imm=4, .has_imm=true },  // if (i<9) outer
        { .op=OP_HALT }
    };

    int r = cpu_run(&cpu, program, sizeof(program)/sizeof(program[0]), 5000000);
    if (r == -1) { printf("CPU fault\n"); cpu_free(&cpu); return 1; }

    printf("A (sorted) = { ");
    for (int i = 0; i < N; i++) {
        word_t v = memory_load_w(&cpu.mem, (uint32_t)(A_BASE + i*4));
        printf("%d%s", (int)v, (i==N-1) ? " " : ", ");
    }
    printf("}\n");

    cpu_print_stats(&cpu);

    cpu_free(&cpu);
    return 0;
}
