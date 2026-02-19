#include <stdint.h>
#include <stdio.h>

#include "core/cpu.h"
#include "core/memory.h"

int main(void) {
  cpu_t cpu;
  if (setup_cpu(&cpu, 64) != 0) {
    printf("Failed to init CPU\n");
    return 1;
  }

  mem_store(&cpu.mem, 8, 1234);

  instr_t program[] = {
      {.op = OP_LDC, .rd = 1, .imm = 8, .has_imm = true},
      {.op = OP_LD, .rd = 2, .rs1 = 1, .imm = 0, .has_imm = true},

      {.op = OP_B, .imm = 4, .has_imm = true},
      {.op = OP_LDC, .rd = 4, .imm = 111, .has_imm = true},
      {.op = OP_LDC, .rd = 4, .imm = 222, .has_imm = true},

      {.op = OP_LDC, .rd = 10, .imm = 0, .has_imm = true},
      {.op = OP_LDC, .rd = 11, .imm = 5, .has_imm = true},
      {.op = OP_LDC, .rd = 12, .imm = 1, .has_imm = true},
      {.op = OP_ADD, .rd = 10, .rs1 = 10, .rs2 = 12, .has_imm = false},
      {.op = OP_BLTH, .rs1 = 10, .rs2 = 11, .imm = 8, .has_imm = true},

      {.op = OP_J, .imm = 2, .has_imm = true},
      {.op = OP_LDC, .rd = 5, .imm = 999, .has_imm = true},
      {.op = OP_LDC, .rd = 5, .imm = 123, .has_imm = true},

      {.op = OP_HALT}};

  int r =
      run_program(&cpu, program, sizeof(program) / sizeof(program[0]), 1000000);
  if (r == -1) {
    printf("CPU fault while running\n");
    free_cpu(&cpu);
    return 1;
  }

  printf("r2=%d (expected 1234)\n", reg_get(&cpu.rf, 2));
  printf("r4=%d (expected 222)\n", reg_get(&cpu.rf, 4));
  printf("r10=%d (expected 5)\n", reg_get(&cpu.rf, 10));
  printf("r5=%d (expected 123)\n", reg_get(&cpu.rf, 5));

  print_stats(&cpu);

  free_cpu(&cpu);
  return 0;
}
