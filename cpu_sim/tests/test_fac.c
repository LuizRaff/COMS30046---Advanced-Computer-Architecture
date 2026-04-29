#include "../src/core/cpu.h"
#include "../src/core/regfile.h"
#include <stdint.h>
#include <stdio.h>

int main(void) {
  cpu_t cpu;
  cpu_config_t cfg = {.issue_width = 1,
                      .num_alus = 2,
                      .num_lsus = 1,
                      .num_brus = 1,
                      .num_vec = 1,
                      .bp_type = BP_TWO_BIT,
                      .benchmark_name = "test"};
  if (setup_cpu(&cpu, 64, cfg) != 0) {
    printf("Failed to init CPU\n");
    return 1;
  }

  instr_t program[] = {
      {.op = OP_LDC, .rd = 1, .imm = 5, .has_imm = true},
      {.op = OP_LDC, .rd = 2, .imm = 1, .has_imm = true},
      {.op = OP_LDC, .rd = 3, .imm = 1, .has_imm = true},
      {.op = OP_LDC, .rd = 4, .imm = 6, .has_imm = true},

      {.op = OP_BLTH, .rs1 = 2, .rs2 = 4, .imm = 6, .has_imm = true},
      {.op = OP_B, .imm = 9, .has_imm = true},

      {.op = OP_MUL, .rd = 3, .rs1 = 3, .rs2 = 2, .has_imm = false},
      {.op = OP_ADDI, .rd = 2, .rs1 = 2, .imm = 1, .has_imm = true},
      {.op = OP_B, .imm = 4, .has_imm = true},
      {.op = OP_HALT}};

  int r =
      run_program(&cpu, program, sizeof(program) / sizeof(program[0]), 1000000);
  if (r == -1) {
    printf("CPU fault\n");
    free_cpu(&cpu);
    return 1;
  }

  printf("fact(5) = %d (expected 120)\n", (int)reg_get(&cpu.rf, 3));
  print_stats(&cpu);
  free_cpu(&cpu);
  return 0;
}
