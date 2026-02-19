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
      {.op = OP_LDC, .rd = 1, .imm = 45, .has_imm = true},
      {.op = OP_LDC, .rd = 2, .imm = 1, .has_imm = true},
      {.op = OP_LDC, .rd = 3, .imm = 0, .has_imm = true},

      {.op = OP_BLTH, .rs1 = 0, .rs2 = 1, .imm = 5, .has_imm = true},
      {.op = OP_B, .imm = 9, .has_imm = true},

      {.op = OP_AND, .rd = 4, .rs1 = 1, .rs2 = 2, .has_imm = false},
      {.op = OP_ADD, .rd = 3, .rs1 = 3, .rs2 = 4, .has_imm = false},
      {.op = OP_SHR, .rd = 1, .rs1 = 1, .imm = 1, .has_imm = true},
      {.op = OP_B, .imm = 3, .has_imm = true},

      {.op = OP_HALT}};

  int r =
      run_program(&cpu, program, sizeof(program) / sizeof(program[0]), 1000000);
  if (r == -1) {
    printf("CPU fault\n");
    free_cpu(&cpu);
    return 1;
  }

  printf("hamming(45) = %d (expected 4)\n", (int)reg_get(&cpu.rf, 3));
  print_stats(&cpu);
  free_cpu(&cpu);
  return 0;
}
