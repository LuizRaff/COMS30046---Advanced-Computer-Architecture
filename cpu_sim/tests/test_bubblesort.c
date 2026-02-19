#include "../src/core/cpu.h"
#include "../src/core/memory.h"
#include <stdint.h>
#include <stdio.h>

#define A_BASE 0x0000
#define N 10

int main(void) {
  cpu_t cpu;
  if (setup_cpu(&cpu, 64) != 0) {
    printf("Failed to init CPU\n");
    return 1;
  }

  int init[N] = {9, 1, 8, 2, 7, 3, 6, 4, 5, 0};
  for (int i = 0; i < N; i++)
    mem_store(&cpu.mem, (uint32_t)(A_BASE + i * 4), (word_t)init[i]);

  instr_t program[] = {
      {.op = OP_LDC, .rd = 1, .imm = A_BASE, .has_imm = true},
      {.op = OP_LDC, .rd = 3, .imm = N, .has_imm = true},
      {.op = OP_LDC, .rd = 11, .imm = N - 1, .has_imm = true},
      {.op = OP_LDC, .rd = 2, .imm = 0, .has_imm = true},

      {.op = OP_SUB, .rd = 5, .rs1 = 11, .rs2 = 2, .has_imm = false},
      {.op = OP_LDC, .rd = 4, .imm = 0, .has_imm = true},
      {.op = OP_ADDI, .rd = 6, .rs1 = 1, .imm = 0, .has_imm = true},

      {.op = OP_LD, .rd = 7, .rs1 = 6, .imm = 0, .has_imm = true},
      {.op = OP_ADDI, .rd = 12, .rs1 = 6, .imm = 4, .has_imm = true},
      {.op = OP_LD, .rd = 8, .rs1 = 12, .imm = 0, .has_imm = true},

      {.op = OP_BLTH, .rs1 = 8, .rs2 = 7, .imm = 12, .has_imm = true},
      {.op = OP_B, .imm = 14, .has_imm = true},

      {.op = OP_ST, .rs1 = 6, .rs2 = 8, .imm = 0, .has_imm = true},
      {.op = OP_ST, .rs1 = 12, .rs2 = 7, .imm = 0, .has_imm = true},

      {.op = OP_ADDI, .rd = 6, .rs1 = 6, .imm = 4, .has_imm = true},
      {.op = OP_ADDI, .rd = 4, .rs1 = 4, .imm = 1, .has_imm = true},
      {.op = OP_BLTH, .rs1 = 4, .rs2 = 5, .imm = 7, .has_imm = true},

      {.op = OP_ADDI, .rd = 2, .rs1 = 2, .imm = 1, .has_imm = true},
      {.op = OP_BLTH, .rs1 = 2, .rs2 = 11, .imm = 4, .has_imm = true},
      {.op = OP_HALT}};

  int r =
      run_program(&cpu, program, sizeof(program) / sizeof(program[0]), 5000000);
  if (r == -1) {
    printf("CPU fault\n");
    free_cpu(&cpu);
    return 1;
  }

  printf("A (sorted) = { ");
  for (int i = 0; i < N; i++) {
    word_t v = mem_load(&cpu.mem, (uint32_t)(A_BASE + i * 4));
    printf("%d%s", (int)v, (i == N - 1) ? " " : ", ");
  }
  printf("}\n");

  print_stats(&cpu);

  free_cpu(&cpu);
  return 0;
}
