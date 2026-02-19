#ifndef CORE_CPU_H
#define CORE_CPU_H

#include "../isa/instruction.h"
#include "memory.h"
#include "regfile.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  instr_t inst;
  uint32_t pc;
  int valid;
} pipe_reg_t;

typedef struct {
  uint32_t pc;
  uint64_t cycles;
  uint64_t instrs;
  pipe_reg_t if_id;
  pipe_reg_t id_ex;

  int halted;

  regfile_t rf;
  memory_t mem;
} cpu_t;

int cpu_init(cpu_t *cpu, size_t mem_words);
void cpu_free(cpu_t *cpu);
void cpu_reset(cpu_t *cpu);

void cpu_print_stats(const cpu_t *cpu);

void cpu_dump_pipeline(const cpu_t *cpu);

int cpu_step(cpu_t *cpu, const instr_t *program, size_t program_len);

int cpu_run(cpu_t *cpu, const instr_t *program, size_t program_len,
            uint64_t max_steps);

int cpu_run_debug(cpu_t *cpu, const instr_t *program, size_t program_len,
                  uint64_t max_steps, uint64_t debug_period);

int cpu_run_dump(cpu_t *cpu, const instr_t *program, size_t program_len,
                 uint64_t max_steps, bool dump_enable,
                 const char *dump_filename);

#endif // CORE_CPU_H