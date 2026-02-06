#ifndef CORE_CPU_H
#define CORE_CPU_H

#include <stddef.h>
#include <stdint.h>
#include "../isa/instruction.h"
#include "regfile.h"
#include "memory.h"

typedef struct {
    uint32_t pc;
    uint32_t cycles;
    regfile_t rf;
    memory_t mem;
} cpu_t;

int  cpu_init(cpu_t *cpu, size_t mem_words);
void cpu_free(cpu_t *cpu);
void cpu_reset(cpu_t *cpu);

int cpu_step(cpu_t *cpu, const instr_t *program, size_t program_len);

int cpu_run(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps);

#endif // CORE_CPU_H