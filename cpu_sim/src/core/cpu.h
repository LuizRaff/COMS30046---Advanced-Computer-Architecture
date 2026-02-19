#ifndef CORE_CPU_H
#define CORE_CPU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../isa/instruction.h"
#include "regfile.h"
#include "memory.h"

typedef struct {
    instr_t  inst;
    uint32_t pc;
    int      valid;
} pipe_reg_t;

typedef struct {
    // Next instruction index to fetch from program[] (instruction memory).
    uint32_t pc;
    uint64_t cycles;
    uint64_t instrs;

    // Simple 3-stage pipeline registers:
    //   IF/ID holds fetched instruction,
    //   ID/EX holds decoded instruction (executed in the EX stage).
    pipe_reg_t if_id;
    pipe_reg_t id_ex;

    int halted; // stop fetching new instructions once set

    regfile_t rf;
    memory_t mem;
} cpu_t;

int  cpu_init(cpu_t *cpu, size_t mem_words);
void cpu_free(cpu_t *cpu);
void cpu_reset(cpu_t *cpu);

// Convenience helper to print Stage-3-Part-1 style metrics.
// Prints: instructions executed, total cycles, and IPC.
void cpu_print_stats(const cpu_t *cpu);

// Debug helper: dump current pipeline/register state.
void cpu_dump_pipeline(const cpu_t *cpu);

// Advance the pipelined core by one *cycle*.
// Returns: 0 = continue, 1 = finished (halted/drained), -1 = fault.
int cpu_step(cpu_t *cpu, const instr_t *program, size_t program_len);

int cpu_run(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps);

// Same as cpu_run, but enables optional pipeline dumping.
// If debug_period > 0, dumps every N cycles (e.g., 1 = every cycle).
int cpu_run_debug(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps, uint64_t debug_period);

// Convenience: dump the full pipeline state for *every* cycle into a file.
// If dump_enable is false, behaves like cpu_run().
// If dump_enable is true, dump_filename must be a valid path.
int cpu_run_dump(cpu_t *cpu,
                 const instr_t *program,
                 size_t program_len,
                 uint64_t max_steps,
                 bool dump_enable,
                 const char *dump_filename);

#endif // CORE_CPU_H