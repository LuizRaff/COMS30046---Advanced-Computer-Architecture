#include "../src/core/cpu.h"
#include "../src/core/regfile.h"
#include <stdint.h>
#include <stdio.h>

int main(void) {
  cpu_t cpu;
  if (setup_cpu(&cpu, 64) != 0) {
    printf("Failed to init CPU\n");
    return 1;
  }

  instr_t program[] = {
      {.op = OP_LDC, .rd = 1, .imm = 48, .has_imm = true},
      {.op = OP_LDC, .rd = 2, .imm = 18, .has_imm = true},

      {.op = OP_BLTH, .rs1 = 1, .rs2 = 2, .imm = 5, .has_imm = true},
      {.op = OP_BLTH, .rs1 = 2, .rs2 = 1, .imm = 7, .has_imm = true},
      {.op = OP_B, .imm = 9, .has_imm = true},

      {.op = OP_SUB, .rd = 2, .rs1 = 2, .rs2 = 1, .has_imm = false},
      {.op = OP_B, .imm = 2, .has_imm = true},

      {.op = OP_SUB, .rd = 1, .rs1 = 1, .rs2 = 2, .has_imm = false},
      {.op = OP_B, .imm = 2, .has_imm = true},

      {.op = OP_HALT}};

  int r =
      run_program(&cpu, program, sizeof(program) / sizeof(program[0]), 1000000);
  if (r == -1) {
    printf("CPU fault\n");
    free_cpu(&cpu);
    return 1;
  }

  printf("gcd = %d (expected 6)\n", (int)reg_get(&cpu.rf, 1));
  print_stats(&cpu);
  free_cpu(&cpu);
  return 0;
}
