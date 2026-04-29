#include "cpu.h"
#include "../isa/exec.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* ---- String helpers ---------------------------------------------------- */
static const char *op_to_str(opcode_t op) {
  switch (op) {
  case OP_NOP: return "nop"; case OP_LD: return "ld"; case OP_LDC: return "ldc";
  case OP_ST: return "st"; case OP_ADD: return "add"; case OP_ADDI: return "addi";
  case OP_SUB: return "sub"; case OP_SUBI: return "subi"; case OP_MUL: return "mul";
  case OP_MULI: return "muli"; case OP_AND: return "and"; case OP_OR: return "or";
  case OP_XOR: return "xor"; case OP_NOT: return "not"; case OP_CMP: return "cmp";
  case OP_SHR: return "shr"; case OP_SHL: return "shl"; case OP_B: return "b";
  case OP_J: return "j"; case OP_BLTH: return "blth"; case OP_HALT: return "halt";
  case OP_VLD: return "vld"; case OP_VST: return "vst";
  case OP_VADD: return "vadd"; case OP_VMUL: return "vmul";
  default: return "?";
  }
}

void inst_to_str(char *buf, size_t n, const instr_t *inst) {
  const char *op = op_to_str(inst->op);
  switch (inst->op) {
  case OP_LD: snprintf(buf,n,"%s r%d,[r%d%+d]",op,inst->rd,inst->rs1,(int)inst->imm); break;
  case OP_ST: snprintf(buf,n,"%s r%d->[r%d%+d]",op,inst->rs2,inst->rs1,(int)inst->imm); break;
  case OP_LDC: snprintf(buf,n,"%s r%d,%d",op,inst->rd,(int)inst->imm); break;
  case OP_ADD: case OP_SUB: case OP_MUL: case OP_AND: case OP_OR: case OP_XOR: case OP_CMP:
    snprintf(buf,n,"%s r%d,r%d,r%d",op,inst->rd,inst->rs1,inst->rs2); break;
  case OP_ADDI: case OP_SUBI: case OP_MULI:
    snprintf(buf,n,"%s r%d,r%d,%d",op,inst->rd,inst->rs1,(int)inst->imm); break;
  case OP_NOT: snprintf(buf,n,"%s r%d,r%d",op,inst->rd,inst->rs1); break;
  case OP_SHR: case OP_SHL:
    snprintf(buf,n,"%s r%d,r%d,%d",op,inst->rd,inst->rs1,(int)inst->imm); break;
  case OP_B: snprintf(buf,n,"%s %d",op,(int)inst->imm); break;
  case OP_J: snprintf(buf,n,"%s %+d",op,(int)inst->imm); break;
  case OP_BLTH: snprintf(buf,n,"%s r%d,r%d,%d",op,inst->rs1,inst->rs2,(int)inst->imm); break;
  case OP_VLD: snprintf(buf,n,"%s v%d,[r%d%+d]",op,inst->rd,inst->rs1,(int)inst->imm); break;
  case OP_VST: snprintf(buf,n,"%s v%d->[r%d%+d]",op,inst->rs2,inst->rs1,(int)inst->imm); break;
  case OP_VADD: case OP_VMUL:
    snprintf(buf,n,"%s v%d,v%d,v%d",op,inst->rd,inst->rs1,inst->rs2); break;
  case OP_NOP: case OP_HALT: snprintf(buf,n,"%s",op); break;
  default: snprintf(buf,n,"op=%d",(int)inst->op); break;
  }
}

/* ---- Pipeline classification ------------------------------------------- */
static int writes_dest(const instr_t *inst) {
  switch (inst->op) {
  case OP_LD: case OP_LDC: case OP_ADD: case OP_ADDI: case OP_SUB: case OP_SUBI:
  case OP_MUL: case OP_MULI: case OP_AND: case OP_OR: case OP_XOR: case OP_NOT:
  case OP_CMP: case OP_SHR: case OP_SHL: return 1;
  default: return 0;
  }
}
static int writes_vreg(const instr_t *inst) {
  return (inst->op == OP_VLD || inst->op == OP_VADD || inst->op == OP_VMUL);
}
static int get_dest(const instr_t *inst) { return writes_dest(inst) ? inst->rd : -1; }
static int get_vdest(const instr_t *inst) { return writes_vreg(inst) ? inst->rd : -1; }

static void get_sources(const instr_t *inst, int *rs1, int *rs2) {
  *rs1 = -1; *rs2 = -1;
  switch (inst->op) {
  case OP_LD: case OP_ST: case OP_ADD: case OP_SUB: case OP_MUL:
  case OP_AND: case OP_OR: case OP_XOR: case OP_CMP: case OP_BLTH:
    *rs1 = inst->rs1; *rs2 = inst->rs2; break;
  case OP_ADDI: case OP_SUBI: case OP_MULI: case OP_NOT: case OP_SHR: case OP_SHL:
    *rs1 = inst->rs1; break;
  case OP_VLD: case OP_VST: *rs1 = inst->rs1; break;
  default: break;
  }
}

static eu_type_t get_required_eu(const instr_t *inst) {
  switch (inst->op) {
  case OP_LD: case OP_ST: return EU_TYPE_LSU;
  case OP_B: case OP_J: case OP_BLTH: return EU_TYPE_BRU;
  case OP_VLD: case OP_VST: case OP_VADD: case OP_VMUL: return EU_TYPE_VEC;
  case OP_NOP: case OP_HALT: return EU_TYPE_NONE;
  default: return EU_TYPE_ALU;
  }
}

static int get_eu_latency(eu_type_t t) {
  if (t == EU_TYPE_LSU) return 2;
  if (t == EU_TYPE_VEC) return 2;
  return 1;
}


static int has_older_store(const cpu_t *cpu, int rob_idx) {
  int i = cpu->rob_head;
  while (i != rob_idx) {
    if (cpu->rob[i].valid && cpu->rob[i].type == ROB_TYPE_STORE) return 1;
    i = (i + 1) % ROB_SIZE;
  }
  return 0;
}

/* ---- Branch predictor -------------------------------------------------- */
static int bp_predict(cpu_t *cpu, uint32_t pc, const instr_t *inst) {
  int is_branch = (inst->op == OP_B || inst->op == OP_J || inst->op == OP_BLTH);
  if (!is_branch) return 0;
  switch (cpu->cfg.bp_type) {
  case BP_NONE: return 0;
  case BP_ALWAYS_TAKEN: return 1;
  case BP_TWO_BIT: return cpu->bp_table[pc % BP_TABLE] >= 2;
  case BP_TWO_LEVEL_LOCAL: {
    int idx = pc % LOCAL_HIST_SIZE;
    uint8_t hist = cpu->local_hist[idx] & ((1 << LOCAL_HIST_BITS) - 1);
    return cpu->pht[idx][hist] >= 2;
  }
  default: return 0;
  }
}

static void bp_update(cpu_t *cpu, uint32_t pc, int taken) {
  switch (cpu->cfg.bp_type) {
  case BP_NONE: case BP_ALWAYS_TAKEN: break;
  case BP_TWO_BIT: {
    uint8_t *c = &cpu->bp_table[pc % BP_TABLE];
    if (taken && *c < 3) (*c)++; else if (!taken && *c > 0) (*c)--;
    break;
  }
  case BP_TWO_LEVEL_LOCAL: {
    int idx = pc % LOCAL_HIST_SIZE;
    uint8_t hist = cpu->local_hist[idx] & ((1 << LOCAL_HIST_BITS) - 1);
    uint8_t *c = &cpu->pht[idx][hist];
    if (taken && *c < 3) (*c)++; else if (!taken && *c > 0) (*c)--;
    cpu->local_hist[idx] = ((cpu->local_hist[idx] << 1) | (taken ? 1 : 0)) & ((1 << LOCAL_HIST_BITS) - 1);
    break;
  }
  }
}

/* ---- Setup / reset / free ---------------------------------------------- */
int setup_cpu(cpu_t *cpu, size_t mem_words, cpu_config_t cfg) {
  memset(cpu, 0, sizeof(cpu_t));
  cpu->cfg = cfg;
  if (mem_init(&cpu->mem, mem_words) != 0) return -1;
  /* Build execution units */
  int idx = 0;
  for (int i = 0; i < cfg.num_alus && idx < MAX_EUS; i++, idx++)
    cpu->eus[idx].type = EU_TYPE_ALU;
  for (int i = 0; i < cfg.num_lsus && idx < MAX_EUS; i++, idx++)
    cpu->eus[idx].type = EU_TYPE_LSU;
  for (int i = 0; i < cfg.num_brus && idx < MAX_EUS; i++, idx++)
    cpu->eus[idx].type = EU_TYPE_BRU;
  for (int i = 0; i < cfg.num_vec && idx < MAX_EUS; i++, idx++)
    cpu->eus[idx].type = EU_TYPE_VEC;
  cpu->num_eus = idx;
  reset_cpu(cpu);
  return 0;
}

void free_cpu(cpu_t *cpu) { mem_free(&cpu->mem); }

void reset_cpu(cpu_t *cpu) {
  cpu->pc = 0; cpu->cycles = 0; cpu->instrs = 0; cpu->halted = 0;
  /* BP state */
  memset(cpu->bp_table, 1, sizeof(cpu->bp_table)); /* weakly not-taken */
  memset(cpu->local_hist, 0, sizeof(cpu->local_hist));
  memset(cpu->pht, 1, sizeof(cpu->pht)); /* weakly not-taken */
  /* IQ */
  memset(cpu->iq, 0, sizeof(cpu->iq));
  cpu->iq_head = 0; cpu->iq_tail = 0; cpu->iq_count = 0;
  /* RAT */
  for (int i = 0; i < NUM_REGS; i++) cpu->rat[i] = -1;
  /* RS, ROB */
  memset(cpu->rs, 0, sizeof(cpu->rs));
  memset(cpu->rob, 0, sizeof(cpu->rob));
  cpu->rob_head = 0; cpu->rob_tail = 0;
  /* EUs */
  for (int i = 0; i < cpu->num_eus; i++) {
    eu_type_t t = cpu->eus[i].type;
    memset(&cpu->eus[i], 0, sizeof(exec_unit_t));
    cpu->eus[i].type = t;
  }
  /* Vector regs */
  memset(cpu->vreg, 0, sizeof(cpu->vreg));
  /* Metrics */
  cpu->total_branches = 0; cpu->correct_predictions = 0;
  cpu->mispredictions = 0; cpu->branch_flushes = 0;
  cpu->rob_full_stalls = 0; cpu->rs_full_stalls = 0;
  /* Scalar regs */
  regs_clear(&cpu->rf);
}



/* ---- tick: one cycle of the OOO pipeline ------------------------------- */
int tick(cpu_t *cpu, const instr_t *program, size_t program_len) {
  /* Check termination */
  if ((cpu->halted || cpu->pc >= program_len) && cpu->iq_count == 0) {
    int active = 0;
    for (int i = 0; i < cpu->num_eus; i++) if (cpu->eus[i].busy) active = 1;
    for (int i = 0; i < ROB_SIZE; i++) if (cpu->rob[i].valid) active = 1;
    if (!active) return 1;
  }
  cpu->cycles++;

  /* ---- Phase 1: COMMIT (in-order from ROB head) ---- */
  int flush = 0; uint32_t flush_pc = 0;
  for (int c = 0; c < cpu->cfg.issue_width && !flush; c++) {
    rob_entry_t *h = &cpu->rob[cpu->rob_head];
    if (!h->valid || !h->ready) break;

    if (h->type == ROB_TYPE_REG_WRITE) {
      if (h->dest_reg > 0) reg_set(&cpu->rf, h->dest_reg, h->result);
    } else if (h->type == ROB_TYPE_STORE) {
      do_st(h->result, h->inst.imm, h->store_data, &cpu->mem);
    } else if (h->type == ROB_TYPE_VEC_WRITE) {
      if (h->dest_vreg >= 0 && h->dest_vreg < NUM_VREG)
        for (int v = 0; v < VLEN; v++) cpu->vreg[h->dest_vreg].e[v] = h->vec_result[v];
    } else if (h->type == ROB_TYPE_VEC_STORE) {
      uint32_t addr = addr_of(h->result, h->inst.imm);
      for (int v = 0; v < VLEN; v++) mem_store(&cpu->mem, addr + v*4, h->vec_result[v]);
    } else if (h->type == ROB_TYPE_HALT) {
      cpu->halted = 1;
    }
    if (h->inst.op != OP_NOP) cpu->instrs++;

    /* RAT cleanup */
    if (h->type == ROB_TYPE_REG_WRITE && h->dest_reg > 0) {
      if (cpu->rat[h->dest_reg] == cpu->rob_head) cpu->rat[h->dest_reg] = -1;
    }

    if (h->branch_mispredicted) {
      flush = 1; flush_pc = h->correct_pc; cpu->branch_flushes++;
    }
    h->valid = 0;
    cpu->rob_head = (cpu->rob_head + 1) % ROB_SIZE;
  }

  if (flush) {
    /* Flush everything younger */
    memset(cpu->iq, 0, sizeof(cpu->iq));
    cpu->iq_head = 0; cpu->iq_tail = 0; cpu->iq_count = 0;
    for (int i = 0; i < NUM_REGS; i++) cpu->rat[i] = -1;
    for (int i = 0; i < RS_SIZE; i++) cpu->rs[i].busy = 0;
    for (int i = 0; i < ROB_SIZE; i++) cpu->rob[i].valid = 0;
    cpu->rob_head = 0; cpu->rob_tail = 0;
    for (int i = 0; i < cpu->num_eus; i++) cpu->eus[i].busy = 0;
    cpu->pc = flush_pc;
    return 0;
  }

  /* ---- Phase 2: COMPLETE / WRITEBACK (CDB broadcast) ---- */
  for (int i = 0; i < cpu->num_eus; i++) {
    if (!cpu->eus[i].busy) continue;
    cpu->eus[i].cycles_left--;
    if (cpu->eus[i].cycles_left > 0) continue;
    int dr = cpu->eus[i].dest_rob;
    rob_entry_t *re = &cpu->rob[dr];
    re->result = cpu->eus[i].result;
    if (cpu->eus[i].type == EU_TYPE_VEC) {
      memcpy(re->vec_result, cpu->eus[i].vec_result, sizeof(re->vec_result));
      /* Eagerly write vector results so dependent vector ops read correct data */
      if (re->dest_vreg >= 0 && re->dest_vreg < NUM_VREG) {
        memcpy(cpu->vreg[re->dest_vreg].e, re->vec_result, sizeof(re->vec_result));
      }
    }
    if (cpu->eus[i].flag_extra) {
      re->branch_mispredicted = 1; re->correct_pc = cpu->eus[i].result_extra;
    }
    re->ready = 1;
    /* Broadcast to RS */
    for (int j = 0; j < RS_SIZE; j++) {
      if (!cpu->rs[j].busy) continue;
      if (cpu->rs[j].qj == dr) { cpu->rs[j].qj = -1; cpu->rs[j].vj = cpu->eus[i].result; }
      if (cpu->rs[j].qk == dr) { cpu->rs[j].qk = -1; cpu->rs[j].vk = cpu->eus[i].result; }
    }
    cpu->eus[i].busy = 0;
  }

  /* ---- Phase 3: SCHEDULE (dispatch ready RS to free EUs) ---- */
  for (int i = 0; i < cpu->num_eus; i++) {
    if (cpu->eus[i].busy) continue;
    for (int j = 0; j < RS_SIZE; j++) {
      if (!cpu->rs[j].busy || cpu->rs[j].req_eu != cpu->eus[i].type) continue;
      if (cpu->rs[j].qj != -1 || cpu->rs[j].qk != -1) continue;

      /* Load ordering: block load if older store in ROB */
      if (cpu->rs[j].inst.op == OP_LD && has_older_store(cpu, cpu->rs[j].dest_rob)) continue;

      /* Vector ordering: block vector-reading ops until older vector writes complete */
      if (cpu->rs[j].inst.op == OP_VADD || cpu->rs[j].inst.op == OP_VMUL || cpu->rs[j].inst.op == OP_VST) {
        int blocked = 0;
        int ri = cpu->rob_head;
        while (ri != cpu->rs[j].dest_rob) {
          if (cpu->rob[ri].valid && !cpu->rob[ri].ready &&
              (cpu->rob[ri].type == ROB_TYPE_VEC_WRITE))
            { blocked = 1; break; }
          ri = (ri + 1) % ROB_SIZE;
        }
        if (blocked) continue;
      }

      cpu->eus[i].busy = 1;
      cpu->eus[i].cycles_left = get_eu_latency(cpu->rs[j].req_eu);
      cpu->eus[i].rs_idx = j;
      cpu->eus[i].dest_rob = cpu->rs[j].dest_rob;
      cpu->eus[i].flag_extra = 0;
      memset(cpu->eus[i].vec_result, 0, sizeof(cpu->eus[i].vec_result));

      instr_t *inst = &cpu->rs[j].inst;
      word_t vj = cpu->rs[j].vj, vk = cpu->rs[j].vk, imm = cpu->rs[j].imm;

      if (cpu->eus[i].type == EU_TYPE_ALU) {
        cpu->eus[i].result = do_alu(inst->op, vj, vk, imm);
      } else if (cpu->eus[i].type == EU_TYPE_LSU) {
        if (inst->op == OP_LD) {
          cpu->eus[i].result = do_ld(vj, imm, &cpu->mem);
        } else { /* ST */
          cpu->eus[i].result = vj;
          cpu->rob[cpu->rs[j].dest_rob].store_data = vk;
        }
      } else if (cpu->eus[i].type == EU_TYPE_BRU) {
        int taken = 0; uint32_t tpc = cpu->rs[j].pc + 1;
        if (inst->op == OP_B) { taken = 1; tpc = (uint32_t)imm; }
        else if (inst->op == OP_J) { taken = 1; tpc = cpu->rs[j].pc + (uint32_t)imm; }
        else if (inst->op == OP_BLTH) { if (vj < vk) { taken = 1; tpc = (uint32_t)imm; } }
        cpu->eus[i].result = taken;
        rob_entry_t *rb = &cpu->rob[cpu->rs[j].dest_rob];
        cpu->total_branches++;
        if (rb->branch_taken != taken || (taken && rb->correct_pc != tpc)) {
          cpu->mispredictions++;
          cpu->eus[i].flag_extra = 1; cpu->eus[i].result_extra = tpc;
        } else {
          cpu->correct_predictions++;
        }
        bp_update(cpu, cpu->rs[j].pc, taken);
      } else if (cpu->eus[i].type == EU_TYPE_VEC) {
        rob_entry_t *rb = &cpu->rob[cpu->rs[j].dest_rob];
        if (inst->op == OP_VLD) {
          uint32_t addr = addr_of(vj, imm);
          for (int v = 0; v < VLEN; v++) cpu->eus[i].vec_result[v] = mem_load(&cpu->mem, addr + v*4);
        } else if (inst->op == OP_VST) {
          cpu->eus[i].result = vj; /* base addr */
          int vs = inst->rs2;
          if (vs >= 0 && vs < NUM_VREG)
            memcpy(cpu->eus[i].vec_result, cpu->vreg[vs].e, sizeof(cpu->eus[i].vec_result));
        } else if (inst->op == OP_VADD || inst->op == OP_VMUL) {
          int vs1 = inst->rs1, vs2 = inst->rs2;
          word_t a[VLEN] = {0}, b[VLEN] = {0};
          if (vs1 >= 0 && vs1 < NUM_VREG) memcpy(a, cpu->vreg[vs1].e, sizeof(a));
          if (vs2 >= 0 && vs2 < NUM_VREG) memcpy(b, cpu->vreg[vs2].e, sizeof(b));
          if (inst->op == OP_VADD) do_vadd(cpu->eus[i].vec_result, a, b);
          else do_vmul(cpu->eus[i].vec_result, a, b);
        }
        (void)rb;
      }
      cpu->rs[j].busy = 0;
      break;
    }
  }

  /* ---- Phase 4: ISSUE (from IQ into ROB + RS) ---- */
  for (int c = 0; c < cpu->cfg.issue_width && cpu->iq_count > 0; c++) {
    iq_entry_t *iqe = &cpu->iq[cpu->iq_head];
    if (!iqe->valid) break;
    instr_t *inst = &iqe->inst;
    eu_type_t eu = get_required_eu(inst);

    /* Check ROB space */
    if (cpu->rob[cpu->rob_tail].valid) { cpu->rob_full_stalls++; break; }

    /* Check RS space (not needed for NOP/HALT) */
    int free_rs = -1;
    if (eu != EU_TYPE_NONE) {
      for (int i = 0; i < RS_SIZE; i++) if (!cpu->rs[i].busy) { free_rs = i; break; }
      if (free_rs == -1) { cpu->rs_full_stalls++; break; }
    }

    int dest = cpu->rob_tail;
    rob_entry_t *re = &cpu->rob[dest];
    memset(re, 0, sizeof(rob_entry_t));
    re->valid = 1; re->inst = *inst; re->pc = iqe->pc;
    re->dest_reg = -1; re->dest_vreg = -1;

    if (eu == EU_TYPE_NONE) {
      re->ready = 1;
      re->type = (inst->op == OP_HALT) ? ROB_TYPE_HALT : ROB_TYPE_NOP;
    } else {
      re->ready = 0;
      re->dest_reg = get_dest(inst);
      re->dest_vreg = get_vdest(inst);

      if (eu == EU_TYPE_BRU) {
        re->type = ROB_TYPE_BRANCH;
        re->branch_taken = iqe->predicted_taken;
        re->correct_pc = iqe->predicted_target;
      } else if (inst->op == OP_ST) {
        re->type = ROB_TYPE_STORE;
      } else if (inst->op == OP_VST) {
        re->type = ROB_TYPE_VEC_STORE;
      } else if (writes_vreg(inst)) {
        re->type = ROB_TYPE_VEC_WRITE;
      } else {
        re->type = ROB_TYPE_REG_WRITE;
      }

      /* Fill RS */
      int rs1, rs2;
      get_sources(inst, &rs1, &rs2);
      rs_entry_t *rse = &cpu->rs[free_rs];
      memset(rse, 0, sizeof(rs_entry_t));
      rse->busy = 1; rse->inst = *inst; rse->pc = iqe->pc;
      rse->req_eu = eu; rse->imm = inst->imm; rse->dest_rob = dest;

      /* Operand 1 */
      if (rs1 <= 0) { rse->qj = -1; rse->vj = 0; }
      else if (cpu->rat[rs1] != -1) {
        int q = cpu->rat[rs1];
        if (cpu->rob[q].ready) { rse->qj = -1; rse->vj = cpu->rob[q].result; }
        else rse->qj = q;
      } else { rse->qj = -1; rse->vj = reg_get(&cpu->rf, rs1); }

      /* Operand 2 */
      if (rs2 <= 0) { rse->qk = -1; rse->vk = 0; }
      else if (eu == EU_TYPE_VEC) { rse->qk = -1; rse->vk = 0; } /* vec ops use vreg directly */
      else if (cpu->rat[rs2] != -1) {
        int q = cpu->rat[rs2];
        if (cpu->rob[q].ready) { rse->qk = -1; rse->vk = cpu->rob[q].result; }
        else rse->qk = q;
      } else { rse->qk = -1; rse->vk = reg_get(&cpu->rf, rs2); }

      /* For ST: rs2 is the data register, need its value */
      if (inst->op == OP_ST && rs2 > 0) {
        if (cpu->rat[rs2] != -1) {
          int q = cpu->rat[rs2];
          if (cpu->rob[q].ready) { rse->qk = -1; rse->vk = cpu->rob[q].result; }
          else rse->qk = q;
        } else { rse->qk = -1; rse->vk = reg_get(&cpu->rf, rs2); }
      }

      /* Update RAT for scalar dest */
      if (re->dest_reg > 0) cpu->rat[re->dest_reg] = dest;
    }

    cpu->rob_tail = (dest + 1) % ROB_SIZE;
    iqe->valid = 0;
    cpu->iq_head = (cpu->iq_head + 1) % IQ_SIZE;
    cpu->iq_count--;
  }

  /* ---- Phase 5: FETCH (into IQ) ---- */
  for (int c = 0; c < cpu->cfg.issue_width && !cpu->halted && cpu->pc < program_len; c++) {
    if (cpu->iq_count >= IQ_SIZE) break;
    iq_entry_t *iqe = &cpu->iq[cpu->iq_tail];
    iqe->inst = program[cpu->pc];
    iqe->pc = cpu->pc;
    iqe->valid = 1;
    int pred = bp_predict(cpu, cpu->pc, &iqe->inst);
    iqe->predicted_taken = pred;
    if (pred) {
      if (iqe->inst.op == OP_B || iqe->inst.op == OP_BLTH)
        iqe->predicted_target = (uint32_t)iqe->inst.imm;
      else if (iqe->inst.op == OP_J)
        iqe->predicted_target = cpu->pc + (uint32_t)iqe->inst.imm;
      else iqe->predicted_target = cpu->pc + 1;
      cpu->pc = iqe->predicted_target;
    } else {
      iqe->predicted_target = cpu->pc + 1;
      cpu->pc = cpu->pc + 1;
    }
    cpu->iq_tail = (cpu->iq_tail + 1) % IQ_SIZE;
    cpu->iq_count++;
  }
  return 0;
}

/* ---- Stats ------------------------------------------------------------- */
static const char *bp_name(bp_type_t t) {
  switch (t) {
  case BP_NONE: return "none"; case BP_ALWAYS_TAKEN: return "always-taken";
  case BP_TWO_BIT: return "two-bit"; case BP_TWO_LEVEL_LOCAL: return "two-level-local";
  default: return "?";
  }
}

void print_stats(const cpu_t *cpu) {
  double ipc = cpu->cycles ? (double)cpu->instrs / cpu->cycles : 0.0;
  double acc = cpu->total_branches ? 100.0 * cpu->correct_predictions / cpu->total_branches : 0.0;
  printf("\n=== Simulation Results ===\n");
  printf("Benchmark:           %s\n", cpu->cfg.benchmark_name ? cpu->cfg.benchmark_name : "unknown");
  printf("Branch predictor:    %s\n", bp_name(cpu->cfg.bp_type));
  printf("Issue width:         %d\n", cpu->cfg.issue_width);
  printf("ALUs:                %d\n", cpu->cfg.num_alus);
  printf("Vector unit:         %s\n", cpu->cfg.num_vec ? "enabled" : "disabled");
  printf("Cycles:              %" PRIu64 "\n", cpu->cycles);
  printf("Instructions committed: %" PRIu64 "\n", cpu->instrs);
  printf("IPC:                 %.2f\n", ipc);
  printf("Branches:            %" PRIu64 "\n", cpu->total_branches);
  printf("Correct predictions: %" PRIu64 "\n", cpu->correct_predictions);
  printf("Mispredictions:      %" PRIu64 "\n", cpu->mispredictions);
  printf("Branch accuracy:     %.2f%%\n", acc);
  printf("Branch flushes:      %" PRIu64 "\n", cpu->branch_flushes);
  printf("ROB full stalls:     %" PRIu64 "\n", cpu->rob_full_stalls);
  printf("RS full stalls:      %" PRIu64 "\n", cpu->rs_full_stalls);
  printf("==========================\n");
}

int run_program(cpu_t *cpu, const instr_t *program, size_t program_len, uint64_t max_steps) {
  for (uint64_t s = 0; s < max_steps; s++) {
    int r = tick(cpu, program, program_len);
    if (r == 1) return 0;
    if (r == -1) return -1;
  }
  return 1;
}
