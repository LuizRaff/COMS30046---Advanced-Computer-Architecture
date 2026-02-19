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

int setup_cpu(cpu_t *cpu, size_t mem_words);
void free_cpu(cpu_t *cpu);
void reset_cpu(cpu_t *cpu);

void print_stats(const cpu_t *cpu);

void dump_pipeline(const cpu_t *cpu);

int tick(cpu_t *cpu, const instr_t *program, size_t program_len);

int run_program(cpu_t *cpu, const instr_t *program, size_t program_len,
                uint64_t max_steps);

int run_with_debug(cpu_t *cpu, const instr_t *program, size_t program_len,
                   uint64_t max_steps, uint64_t debug_period);

int run_and_dump(cpu_t *cpu, const instr_t *program, size_t program_len,
                 uint64_t max_steps, bool dump_enable,
                 const char *dump_filename);

#endif