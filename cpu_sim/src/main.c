#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/cpu.h"
#include "core/memory.h"

#define A_BASE 0x0000
#define B_BASE 0x0100
#define C_BASE 0x0200
#define ARRAY_N 16

static size_t bm_independent_alu(Instruction *p) {
  int n = 0;
  for (int i = 1; i <= 20; i++)
    p[n++] = (Instruction){
        .op = OP_ADDI, .rd = i, .rs1 = 0, .imm = i, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

static size_t bm_dependent_alu(Instruction *p) {
  int n = 0;
  p[n++] = (Instruction){.op = OP_LDC, .rd = 1, .imm = 0, .has_imm = true};
  for (int i = 0; i < 30; i++)
    p[n++] = (Instruction){
        .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

static size_t bm_branch_loop(Instruction *p) {
  int n = 0;
  p[n++] = (Instruction){.op = OP_LDC, .rd = 1, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 2, .imm = 20, .has_imm = true};

  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_BLTH, .rs1 = 1, .rs2 = 2, .imm = 2, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

static size_t bm_branch_pattern(Instruction *p) {

  int n = 0;
  p[n++] = (Instruction){.op = OP_LDC, .rd = 1, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 2, .imm = 30, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 3, .imm = 2, .has_imm = true};

  p[n++] = (Instruction){
      .op = OP_AND, .rd = 4, .rs1 = 1, .rs2 = 3, .has_imm = false};
  p[n++] = (Instruction){
      .op = OP_BLTH, .rs1 = 0, .rs2 = 4, .imm = 6, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 5, .rs1 = 5, .imm = 1, .has_imm = true};

  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 6, .rs1 = 6, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_BLTH, .rs1 = 1, .rs2 = 2, .imm = 3, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

/* Initializes memory arrays for vector benchmarks */
static void setup_arrays(Processor *cpu) {
  for (int i = 0; i < ARRAY_N; i++) {
    mem_store(&cpu->mem, (uint32_t)(B_BASE + i * 4), (Word)(i + 1));
    mem_store(&cpu->mem, (uint32_t)(C_BASE + i * 4), (Word)(i + 1));
    mem_store(&cpu->mem, (uint32_t)(A_BASE + i * 4), 0);
  }
}

static size_t bm_scalar_vector_add(Instruction *p) {
  int n = 0;
  p[n++] = (Instruction){.op = OP_LDC, .rd = 1, .imm = A_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 2, .imm = B_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 3, .imm = C_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 4, .imm = 0, .has_imm = true};
  p[n++] =
      (Instruction){.op = OP_LDC, .rd = 5, .imm = ARRAY_N, .has_imm = true};

  p[n++] =
      (Instruction){.op = OP_LD, .rd = 7, .rs1 = 2, .imm = 0, .has_imm = true};
  p[n++] =
      (Instruction){.op = OP_LD, .rd = 8, .rs1 = 3, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADD, .rd = 9, .rs1 = 7, .rs2 = 8, .has_imm = false};
  p[n++] =
      (Instruction){.op = OP_ST, .rs1 = 1, .rs2 = 9, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 2, .rs1 = 2, .imm = 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 3, .rs1 = 3, .imm = 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 4, .rs1 = 4, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_BLTH, .rs1 = 4, .rs2 = 5, .imm = 5, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

static size_t bm_vectorized_vector_add(Instruction *p) {
  int n = 0;
  p[n++] = (Instruction){.op = OP_LDC, .rd = 1, .imm = A_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 2, .imm = B_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 3, .imm = C_BASE, .has_imm = true};
  p[n++] = (Instruction){.op = OP_LDC, .rd = 4, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_LDC, .rd = 5, .imm = ARRAY_N / VLEN, .has_imm = true};

  p[n++] =
      (Instruction){.op = OP_VLD, .rd = 0, .rs1 = 2, .imm = 0, .has_imm = true};
  p[n++] =
      (Instruction){.op = OP_VLD, .rd = 1, .rs1 = 3, .imm = 0, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_VADD, .rd = 2, .rs1 = 0, .rs2 = 1, .has_imm = false};
  p[n++] = (Instruction){
      .op = OP_VST, .rs1 = 1, .rs2 = 2, .imm = 0, .has_imm = true};

  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 1, .rs1 = 1, .imm = VLEN * 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 2, .rs1 = 2, .imm = VLEN * 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 3, .rs1 = 3, .imm = VLEN * 4, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_ADDI, .rd = 4, .rs1 = 4, .imm = 1, .has_imm = true};
  p[n++] = (Instruction){
      .op = OP_BLTH, .rs1 = 4, .rs2 = 5, .imm = 5, .has_imm = true};
  p[n++] = (Instruction){.op = OP_HALT};
  return n;
}

/* Prints the correct CLI usage instructions */
static void usage(void) {
  printf("Usage: ./main --benchmark=NAME [--bp=MODE] [--issue-width=N] "
         "[--alus=N]\n");
  printf("Benchmarks: independent_alu, dependent_alu, branch_loop, "
         "branch_pattern,\n");
  printf("            scalar_vector_add, vectorized_vector_add\n");
  printf("BP modes: none, always, two-bit, two-level\n");
}

int main(int argc, char **argv) {
  const char *benchmark = NULL;
  BranchPredictorType bp = BP_TWO_BIT;
  int issue_width = 2, num_alus = 2;

  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--benchmark=", 12) == 0)
      benchmark = argv[i] + 12;
    else if (strncmp(argv[i], "--bp=", 5) == 0) {
      const char *m = argv[i] + 5;
      if (strcmp(m, "none") == 0)
        bp = BP_NONE;
      else if (strcmp(m, "always") == 0)
        bp = BP_ALWAYS_TAKEN;
      else if (strcmp(m, "two-bit") == 0)
        bp = BP_TWO_BIT;
      else if (strcmp(m, "two-level") == 0)
        bp = BP_TWO_LEVEL_LOCAL;
      else {
        printf("Unknown bp mode: %s\n", m);
        return 1;
      }
    } else if (strncmp(argv[i], "--issue-width=", 14) == 0)
      issue_width = atoi(argv[i] + 14);
    else if (strncmp(argv[i], "--alus=", 7) == 0)
      num_alus = atoi(argv[i] + 7);
    else if (strcmp(argv[i], "--help") == 0) {
      usage();
      return 0;
    }
  }
  if (!benchmark) {
    usage();
    return 1;
  }

  int need_vec = (strcmp(benchmark, "vectorized_vector_add") == 0);
  ProcessorConfig cfg = {.issue_width = issue_width,
                         .num_alus = num_alus,
                         .num_lsus = 1,
                         .num_brus = 1,
                         .num_vec = need_vec ? 1 : 0,
                         .bp_type = bp,
                         .benchmark_name = benchmark};

  Processor cpu;
  if (setup_cpu(&cpu, 256, cfg) != 0) {
    printf("Failed to init CPU\n");
    return 1;
  }

  Instruction prog[128];
  size_t plen = 0;
  int needs_arrays = 0;

  if (strcmp(benchmark, "independent_alu") == 0)
    plen = bm_independent_alu(prog);
  else if (strcmp(benchmark, "dependent_alu") == 0)
    plen = bm_dependent_alu(prog);
  else if (strcmp(benchmark, "branch_loop") == 0)
    plen = bm_branch_loop(prog);
  else if (strcmp(benchmark, "branch_pattern") == 0)
    plen = bm_branch_pattern(prog);
  else if (strcmp(benchmark, "scalar_vector_add") == 0) {
    plen = bm_scalar_vector_add(prog);
    needs_arrays = 1;
  } else if (strcmp(benchmark, "vectorized_vector_add") == 0) {
    plen = bm_vectorized_vector_add(prog);
    needs_arrays = 1;
  } else {
    printf("Unknown benchmark: %s\n", benchmark);
    free_cpu(&cpu);
    return 1;
  }

  if (needs_arrays)
    setup_arrays(&cpu);

  int r = run_program(&cpu, prog, plen, 5000000);
  if (r == -1) {
    printf("CPU fault\n");
    free_cpu(&cpu);
    return 1;
  }
  if (r == 1) {
    printf("Timeout\n");
    free_cpu(&cpu);
    return 1;
  }

  if (needs_arrays) {
    int pass = 1;
    for (int i = 0; i < ARRAY_N; i++) {
      Word a = mem_load(&cpu.mem, (uint32_t)(A_BASE + i * 4));
      Word b = mem_load(&cpu.mem, (uint32_t)(B_BASE + i * 4));
      Word c = mem_load(&cpu.mem, (uint32_t)(C_BASE + i * 4));
      if (a != b + c) {
        printf("FAIL: A[%d]=%d expected %d\n", i, a, b + c);
        pass = 0;
      }
    }
    printf("Array result: %s\n", pass ? "PASS" : "FAIL");
  }

  print_stats(&cpu);
  free_cpu(&cpu);
  return 0;
}
