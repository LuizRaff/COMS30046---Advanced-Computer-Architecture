#ifndef CORE_CPU_H
#define CORE_CPU_H

#include "../isa/instruction.h"
#include "memory.h"
#include "regfile.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ----- Sizing constants -------------------------------------------------- */
#define ROB_SIZE 32
#define RS_SIZE 32
#define IQ_SIZE 16
#define BP_TABLE 256
#define MAX_EUS 8
#define LOCAL_HIST_SIZE 64              /* entries in local history table */
#define LOCAL_HIST_BITS 4               /* bits per local history register */
#define PHT_SIZE (1 << LOCAL_HIST_BITS) /* pattern history table entries */

/* ----- Branch predictor modes -------------------------------------------- */
typedef enum {
  BP_NONE,           /* always predict not-taken */
  BP_ALWAYS_TAKEN,   /* always predict taken */
  BP_TWO_BIT,        /* 2-bit saturating counter per PC */
  BP_TWO_LEVEL_LOCAL /* per-branch local history + PHT */
} bp_type_t;

/* ----- Execution unit types ---------------------------------------------- */
typedef enum {
  EU_TYPE_ALU,
  EU_TYPE_LSU,
  EU_TYPE_BRU,
  EU_TYPE_VEC,
  EU_TYPE_NONE /* for NOP / HALT — no EU needed */
} eu_type_t;

/* ----- Instruction queue entry ------------------------------------------- */
typedef struct {
  int valid;
  instr_t inst;
  uint32_t pc;
  int predicted_taken;
  uint32_t predicted_target;
} iq_entry_t;

/* ----- Reservation station entry ----------------------------------------- */
typedef struct {
  int busy;
  instr_t inst;
  uint32_t pc;
  eu_type_t req_eu;

  int qj; /* ROB index of source 1, -1 if ready */
  int qk; /* ROB index of source 2, -1 if ready */
  word_t vj;
  word_t vk;
  word_t imm;
  int dest_rob; /* ROB entry index */
} rs_entry_t;

/* ----- ROB entry types --------------------------------------------------- */
typedef enum {
  ROB_TYPE_REG_WRITE,
  ROB_TYPE_STORE,
  ROB_TYPE_BRANCH,
  ROB_TYPE_HALT,
  ROB_TYPE_NOP,
  ROB_TYPE_VEC_WRITE, /* writes a vector register */
  ROB_TYPE_VEC_STORE  /* vector store */
} rob_inst_type_t;

/* ----- ROB entry --------------------------------------------------------- */
typedef struct {
  int valid;
  int ready;

  instr_t inst;
  uint32_t pc;
  rob_inst_type_t type;

  int dest_reg;            /* architectural scalar dest, or -1 */
  int dest_vreg;           /* architectural vector dest, or -1 */
  word_t result;           /* scalar result / store addr / branch outcome */
  word_t store_data;       /* scalar store data */
  word_t vec_result[VLEN]; /* vector result data */

  int branch_taken;
  int branch_mispredicted;
  uint32_t correct_pc;
} rob_entry_t;

/* ----- Execution unit ---------------------------------------------------- */
typedef struct {
  eu_type_t type;
  int busy;
  int cycles_left;
  int rs_idx;
  int dest_rob;
  word_t result;
  word_t result_extra; /* e.g. correct_pc */
  int flag_extra;      /* e.g. branch_mispredicted */
  word_t vec_result[VLEN];
} exec_unit_t;

/* ----- CPU configuration ------------------------------------------------- */
typedef struct {
  int issue_width; /* instructions issued per cycle */
  int num_alus;    /* number of ALU execution units */
  int num_lsus;    /* load/store units (default 1) */
  int num_brus;    /* branch units (default 1) */
  int num_vec;     /* vector units (0 or 1) */
  bp_type_t bp_type;
  const char *benchmark_name;
} cpu_config_t;

/* ----- CPU state --------------------------------------------------------- */
typedef struct {
  uint32_t pc;
  uint64_t cycles;
  uint64_t instrs;
  int halted;

  cpu_config_t cfg;

  /* Branch predictor state */
  uint8_t bp_table[BP_TABLE];             /* 2-bit counters */
  uint8_t local_hist[LOCAL_HIST_SIZE];    /* local history regs */
  uint8_t pht[LOCAL_HIST_SIZE][PHT_SIZE]; /* pattern history table */

  /* Instruction queue (between fetch and issue) */
  iq_entry_t iq[IQ_SIZE];
  int iq_head;
  int iq_tail;
  int iq_count;

  /* Register alias table */
  int rat[NUM_REGS]; /* maps arch reg to ROB index, -1 if in regfile */

  /* Reservation stations */
  rs_entry_t rs[RS_SIZE];

  /* Reorder buffer */
  rob_entry_t rob[ROB_SIZE];
  int rob_head;
  int rob_tail;

  /* Execution units (dynamically configured) */
  exec_unit_t eus[MAX_EUS];
  int num_eus;

  /* Vector registers */
  vreg_t vreg[NUM_VREG];

  /* Metrics */
  uint64_t total_branches;
  uint64_t correct_predictions;
  uint64_t mispredictions;
  uint64_t branch_flushes;
  uint64_t rob_full_stalls;
  uint64_t rs_full_stalls;

  regfile_t rf;
  memory_t mem;
} cpu_t;

/* ----- API --------------------------------------------------------------- */
int setup_cpu(cpu_t *cpu, size_t mem_words, cpu_config_t cfg);
void free_cpu(cpu_t *cpu);
void reset_cpu(cpu_t *cpu);

void print_stats(const cpu_t *cpu);

int tick(cpu_t *cpu, const instr_t *program, size_t program_len);

int run_program(cpu_t *cpu, const instr_t *program, size_t program_len,
                uint64_t max_steps);

#endif