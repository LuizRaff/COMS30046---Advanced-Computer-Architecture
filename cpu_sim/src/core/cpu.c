#include "cpu.h"
#include <stdio.h>
#include <inttypes.h>
#include "../isa/exec.h"

int cpu_init(cpu_t *cpu, size_t mem_words) {
    cpu->pc = 0;
    regfile_init(&cpu->rf);
    if (memory_init(&cpu->mem, mem_words) != 0) return -1;
    return 0;
}

void cpu_free(cpu_t *cpu) {
    memory_free(&cpu->mem);
}

void cpu_reset(cpu_t *cpu) {
    cpu->pc = 0;
    regfile_init(&cpu->rf);
    // memory contents are preserved by default; clear here if you prefer
}

static int set_pc_checked(cpu_t *cpu, int64_t next_pc, size_t program_len) {
    // allow next_pc == program_len to mean "finish"
    if (next_pc < 0 || next_pc > (int64_t)program_len) {
        fprintf(stderr, "PC fault: next_pc=%" PRId64 " out of range [0..%zu]\n",
                next_pc, program_len);
        return -1;
    }
    cpu->pc = (uint32_t)next_pc;
    return 0;
}

int cpu_step(cpu_t *cpu, const instr_t *program, size_t program_len) {
    if (cpu->pc >= program_len) return 1; // finished

    const instr_t *inst = &program[cpu->pc];

    switch (inst->op) {
        case OP_B: {
            // absolute branch: PC = imm
            return set_pc_checked(cpu, (int64_t)inst->imm, program_len);
        }

        case OP_J: {
            // relative jump: PC = PC + imm (imm can be negative)
            int64_t next = (int64_t)cpu->pc + (int64_t)inst->imm;
            return set_pc_checked(cpu, next, program_len);
        }

        case OP_BLTH: {
            // if R[rs1] < R[rs2] then PC = imm else PC = PC + 1
            word_t a = regfile_read(&cpu->rf, inst->rs1);
            word_t b = regfile_read(&cpu->rf, inst->rs2);
            if (a < b) {
                return set_pc_checked(cpu, (int64_t)inst->imm, program_len);
            } else {
                cpu->pc += 1;
                return 0;
            }
        }

        case OP_HALT:
            return 1;

        default:
            // normal instruction: execute and advance PC
            exec_instr(inst, &cpu->rf, &cpu->mem);
            cpu->pc += 1;
            return 0;
    }
}

int cpu_run(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps) {
    for (uint64_t step = 0; step < max_steps; step++) {
        int r = cpu_step(cpu, program, program_len);
        if (r == 1) return 0;   // halted/finished
        if (r == -1) return -1; // fault
    }
    return 1; // max steps reached
}
