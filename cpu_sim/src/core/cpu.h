#ifndef CORE_CPU_H
#define CORE_CPU_H

#include <stddef.h>
#include <stdint.h>
#include "../isa/instruction.h"
#include "regfile.h"
#include "memory.h"

typedef struct {
    uint32_t pc;    // program counter: instruction index into program[]
    regfile_t rf;
    memory_t mem;
} cpu_t;

// Returns 0 on success, -1 on allocation failure
int  cpu_init(cpu_t *cpu, size_t mem_words);
void cpu_free(cpu_t *cpu);
void cpu_reset(cpu_t *cpu);

// Execute one instruction. 
// Returns: 0 = executed, 1 = halted/finished, -1 = fault (bad PC/branch)
int cpu_step(cpu_t *cpu, const instr_t *program, size_t program_len);

// Run until halt/finish or max_steps reached.
// Returns: 0 = halted/finished, 1 = max_steps reached, -1 = fault
int cpu_run(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps);

#endif // CORE_CPU_H
