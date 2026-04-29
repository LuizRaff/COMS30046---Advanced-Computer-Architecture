#ifndef CORE_CPU_H
#define CORE_CPU_H

#include "../isa/instruction.h"
#include "memory.h"
#include "regfile.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define ROB_SIZE 32
#define RS_SIZE 32
#define IQ_SIZE 16
#define BP_TABLE 256
#define MAX_EUS 8
#define LOCAL_HIST_SIZE 64              
#define LOCAL_HIST_BITS 4               
#define PHT_SIZE (1 << LOCAL_HIST_BITS) 


typedef enum {
  BP_NONE,           
  BP_ALWAYS_TAKEN,   
  BP_TWO_BIT,        
  BP_TWO_LEVEL_LOCAL 
} BranchPredictorType;


typedef enum {
  EU_TYPE_ALU,
  EU_TYPE_LSU,
  EU_TYPE_BRU,
  EU_TYPE_VEC,
  EU_TYPE_NONE 
} ExecutionUnitType;


typedef struct {
  int valid;
  Instruction inst;
  uint32_t pc;
  int predicted_taken;
  uint32_t predicted_target;
} FetchQueueItem;


typedef struct {
  int busy;
  Instruction inst;
  uint32_t pc;
  ExecutionUnitType req_eu;

  int qj; 
  int qk; 
  Word vj;
  Word vk;
  Word imm;
  int dest_rob; 
} ReservationStation;


typedef enum {
  ROB_TYPE_REG_WRITE,
  ROB_TYPE_STORE,
  ROB_TYPE_BRANCH,
  ROB_TYPE_HALT,
  ROB_TYPE_NOP,
  ROB_TYPE_VEC_WRITE, 
  ROB_TYPE_VEC_STORE  
} ReorderBufferType;


typedef struct {
  int valid;
  int ready;

  Instruction inst;
  uint32_t pc;
  ReorderBufferType type;

  int dest_reg;            
  int dest_vreg;           
  Word result;           
  Word store_data;       
  Word vec_result[VLEN]; 

  int branch_taken;
  int branch_mispredicted;
  uint32_t correct_pc;
} ReorderBufferEntry;


typedef struct {
  ExecutionUnitType type;
  int busy;
  int cycles_left;
  int rs_idx;
  int dest_rob;
  Word result;
  Word result_extra; 
  int flag_extra;      
  Word vec_result[VLEN];
} ExecutionUnit;


typedef struct {
  int issue_width; 
  int num_alus;    
  int num_lsus;    
  int num_brus;    
  int num_vec;     
  BranchPredictorType bp_type;
  const char *benchmark_name;
} ProcessorConfig;


typedef struct {
  uint32_t pc;
  uint64_t cycles;
  uint64_t instrs;
  int halted;

  ProcessorConfig cfg;

  
  uint8_t bp_table[BP_TABLE];             
  uint8_t local_hist[LOCAL_HIST_SIZE];    
  uint8_t pht[LOCAL_HIST_SIZE][PHT_SIZE]; 

  
  FetchQueueItem iq[IQ_SIZE];
  int iq_head;
  int iq_tail;
  int iq_count;

  
  int rat[NUM_REGS]; 

  
  ReservationStation rs[RS_SIZE];

  
  ReorderBufferEntry rob[ROB_SIZE];
  int rob_head;
  int rob_tail;

  
  ExecutionUnit eus[MAX_EUS];
  int num_eus;

  
  VectorRegister vreg[NUM_VREG];

  
  uint64_t total_branches;
  uint64_t correct_predictions;
  uint64_t mispredictions;
  uint64_t branch_flushes;
  uint64_t rob_full_stalls;
  uint64_t rs_full_stalls;

  RegisterFile rf;
  MemoryBank mem;
} Processor;


int setup_cpu(Processor *cpu, size_t mem_words, ProcessorConfig cfg);
void free_cpu(Processor *cpu);
void reset_cpu(Processor *cpu);

void print_stats(const Processor *cpu);

int tick(Processor *cpu, const Instruction *program, size_t program_len);

int run_program(Processor *cpu, const Instruction *program, size_t program_len,
                uint64_t max_steps);

#endif