#include <stdio.h>
#include <stdint.h>

#include "core/cpu.h"
#include "core/memory.h"
#include "core/regfile.h"

#define A_BASE 0x0000
#define B_BASE 0x0040
#define C_BASE 0x0080

int main(void) {
    cpu_t cpu;
    if (cpu_init(&cpu, 64) != 0) {
        printf("Failed to init CPU\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        memory_store_w(&cpu.mem, (uint32_t)(B_BASE + i * 4), (word_t)(i + 1));
        memory_store_w(&cpu.mem, (uint32_t)(C_BASE + i * 4), (word_t)(i + 1));
        memory_store_w(&cpu.mem, (uint32_t)(A_BASE + i * 4), (word_t)(0));
    }

    const instr_t program[] = {
        { .op = OP_LDC, .rd = 1, .imm = A_BASE, .has_imm = true },  // r1 = &a[0]
        { .op = OP_LDC, .rd = 2, .imm = B_BASE, .has_imm = true },  // r2 = &b[0]
        { .op = OP_LDC, .rd = 3, .imm = C_BASE, .has_imm = true },  // r3 = &c[0]
        { .op = OP_LDC, .rd = 4, .imm = 0,      .has_imm = true },  // r4 = i = 0
        { .op = OP_LDC, .rd = 5, .imm = 10,     .has_imm = true },  // r5 = 10

        { .op = OP_LD,   .rd = 7, .rs1 = 2, .imm = 0, .has_imm = true }, // r7 = *r2 (b[i])
        { .op = OP_LD,   .rd = 8, .rs1 = 3, .imm = 0, .has_imm = true }, // r8 = *r3 (c[i])
        { .op = OP_ADD,  .rd = 9, .rs1 = 7, .rs2 = 8, .has_imm = false}, // r9 = r7 + r8
        { .op = OP_ST,   .rs1 = 1, .rs2 = 9, .imm = 0, .has_imm = true },// *r1 = r9 (a[i])

        { .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = 4, .has_imm = true }, // r1 += 4
        { .op = OP_ADDI, .rd = 2, .rs1 = 2, .imm = 4, .has_imm = true }, // r2 += 4
        { .op = OP_ADDI, .rd = 3, .rs1 = 3, .imm = 4, .has_imm = true }, // r3 += 4
        { .op = OP_ADDI, .rd = 4, .rs1 = 4, .imm = 1, .has_imm = true }, // i++

        { .op = OP_BLTH, .rs1 = 4, .rs2 = 5, .imm = 5, .has_imm = true },
    };

    int r = cpu_run(&cpu, program, (int)(sizeof(program) / sizeof(program[0])), 1000000);
    if (r == -1) {
        printf("CPU fault while running\n");
        cpu_free(&cpu);
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        word_t a = memory_load_w(&cpu.mem, (uint32_t)(A_BASE + i * 4));
        word_t b = memory_load_w(&cpu.mem, (uint32_t)(B_BASE + i * 4));
        word_t c = memory_load_w(&cpu.mem, (uint32_t)(C_BASE + i * 4));
        printf("%d + %d = %d\n", (int)b, (int)c, (int)a);
    }

    printf("cicles total: %d\n", cpu.cycles);

    cpu_free(&cpu);
    return 0;
}