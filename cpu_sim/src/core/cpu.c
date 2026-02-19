#include "cpu.h"
#include "../isa/exec.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

char *op_to_str(opcode_t op) {
  switch (op) {
  case OP_NOP:
    return "nop";
  case OP_LD:
    return "ld";
  case OP_LDC:
    return "ldc";
  case OP_ST:
    return "st";
  case OP_ADD:
    return "add";
  case OP_ADDI:
    return "addi";
  case OP_SUB:
    return "sub";
  case OP_SUBI:
    return "subi";
  case OP_MUL:
    return "mul";
  case OP_MULI:
    return "muli";
  case OP_AND:
    return "and";
  case OP_OR:
    return "or";
  case OP_XOR:
    return "xor";
  case OP_NOT:
    return "not";
  case OP_CMP:
    return "cmp";
  case OP_SHR:
    return "shr";
  case OP_SHL:
    return "shl";
  case OP_B:
    return "b";
  case OP_J:
    return "j";
  case OP_BLTH:
    return "blth";
  case OP_HALT:
    return "halt";
  default:
    return "?";
  }
}

void inst_to_str(char *buf, size_t n, const instr_t *inst) {
  const char *op = op_to_str(inst->op);
  switch (inst->op) {
  case OP_LD:
    snprintf(buf, n, "%s r%d, [r%d %+d]", op, inst->rd, inst->rs1,
             (int)inst->imm);
    break;
  case OP_ST:
    snprintf(buf, n, "%s r%d -> [r%d %+d]", op, inst->rs2, inst->rs1,
             (int)inst->imm);
    break;
  case OP_LDC:
    snprintf(buf, n, "%s r%d, %d", op, inst->rd, (int)inst->imm);
    break;

  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_AND:
  case OP_OR:
  case OP_XOR:
  case OP_CMP:
    snprintf(buf, n, "%s r%d, r%d, r%d", op, inst->rd, inst->rs1, inst->rs2);
    break;

  case OP_ADDI:
  case OP_SUBI:
  case OP_MULI:
    snprintf(buf, n, "%s r%d, r%d, %d", op, inst->rd, inst->rs1,
             (int)inst->imm);
    break;

  case OP_NOT:
    snprintf(buf, n, "%s r%d, r%d", op, inst->rd, inst->rs1);
    break;

  case OP_SHR:
  case OP_SHL:
    snprintf(buf, n, "%s r%d, r%d, %d", op, inst->rd, inst->rs1,
             (int)inst->imm);
    break;

  case OP_B:
    snprintf(buf, n, "%s %d", op, (int)inst->imm);
    break;
  case OP_J:
    snprintf(buf, n, "%s %+d", op, (int)inst->imm);
    break;
  case OP_BLTH:
    snprintf(buf, n, "%s r%d, r%d, %d", op, inst->rs1, inst->rs2,
             (int)inst->imm);
    break;

  case OP_NOP:
  case OP_HALT:
    snprintf(buf, n, "%s", op);
    break;

  default:
    snprintf(buf, n, "op=%d", (int)inst->op);
    break;
  }
}

void dump_pipeline_to(const cpu_t *cpu, FILE *out) {
  char if_buf[96], id_buf[96];
  if (cpu->if_id.valid)
    inst_to_str(if_buf, sizeof(if_buf), &cpu->if_id.inst);
  else
    snprintf(if_buf, sizeof(if_buf), "<empty>");

  if (cpu->id_ex.valid)
    inst_to_str(id_buf, sizeof(id_buf), &cpu->id_ex.inst);
  else
    snprintf(id_buf, sizeof(id_buf), "<empty>");

  fprintf(out,
          "[cycle=%" PRIu64 "] pc=%u halted=%d\n"
          "  IF/ID: valid=%d pc=%u  %s\n"
          "  ID/EX: valid=%d pc=%u  %s\n",
          cpu->cycles, cpu->pc, cpu->halted, cpu->if_id.valid, cpu->if_id.pc,
          if_buf, cpu->id_ex.valid, cpu->id_ex.pc, id_buf);
}

void dump_pipeline(const cpu_t *cpu) { dump_pipeline_to(cpu, stderr); }

int setup_cpu(cpu_t *cpu, size_t mem_words) {
  cpu->pc = 0;
  cpu->cycles = 0;
  cpu->instrs = 0;
  cpu->if_id = (pipe_reg_t){.valid = 0};
  cpu->id_ex = (pipe_reg_t){.valid = 0};
  cpu->halted = 0;
  regs_clear(&cpu->rf);
  if (mem_init(&cpu->mem, mem_words) != 0)
    return -1;
  return 0;
}

void free_cpu(cpu_t *cpu) { mem_free(&cpu->mem); }

void reset_cpu(cpu_t *cpu) {
  cpu->pc = 0;
  cpu->cycles = 0;
  cpu->instrs = 0;
  cpu->if_id = (pipe_reg_t){.valid = 0};
  cpu->id_ex = (pipe_reg_t){.valid = 0};
  cpu->halted = 0;
  regs_clear(&cpu->rf);
}

void print_stats(const cpu_t *cpu) {
  double ipc =
      (cpu->cycles == 0) ? 0.0 : ((double)cpu->instrs / (double)cpu->cycles);
  printf("instructions=%" PRIu64 " cycles=%" PRIu64 " IPC=%.3f\n", cpu->instrs,
         cpu->cycles, ipc);
}

int valid_pc(int64_t pc, size_t program_len) {
  return !(pc < 0 || pc > (int64_t)program_len);
}

instr_t make_nop(void) {
  return (instr_t){
      .op = OP_NOP, .rd = 0, .rs1 = 0, .rs2 = 0, .imm = 0, .has_imm = false};
}

int tick(cpu_t *cpu, const instr_t *program, size_t program_len) {
  if ((cpu->halted || cpu->pc >= program_len) && !cpu->if_id.valid &&
      !cpu->id_ex.valid) {
    return 1;
  }

  cpu->cycles++;

  // -----------------------------
  // Stage EX: execute ID/EX
  // -----------------------------
  int redirect = 0;
  int64_t redirect_pc = 0;
  int squash_younger = 0;

  if (cpu->id_ex.valid) {
    const instr_t *inst = &cpu->id_ex.inst;
    cpu->instrs += (inst->op != OP_NOP);

    switch (inst->op) {
    case OP_B:
      redirect = 1;
      redirect_pc = (int64_t)inst->imm;
      squash_younger = 1;
      break;

    case OP_J:
      redirect = 1;
      redirect_pc = (int64_t)cpu->id_ex.pc + (int64_t)inst->imm;
      squash_younger = 1;
      break;

    case OP_BLTH: {
      word_t a = reg_get(&cpu->rf, inst->rs1);
      word_t b = reg_get(&cpu->rf, inst->rs2);
      if (a < b) {
        redirect = 1;
        redirect_pc = (int64_t)inst->imm;
        squash_younger = 1;
      }
      break;
    }

    case OP_HALT:
      cpu->halted = 1;
      redirect = 1;
      redirect_pc = (int64_t)program_len;
      squash_younger = 1;
      break;

    case OP_NOP:
      break;

    default:
      run_instr(inst, &cpu->rf, &cpu->mem);
      break;
    }
  }

  if (redirect && !valid_pc(redirect_pc, program_len)) {
    fprintf(stderr, "PC fault: next_pc=%" PRId64 " out of range [0..%zu]\n",
            redirect_pc, program_len);
    return -1;
  }

  // -----------------------------
  // Stage ID: move IF/ID -> ID/EX
  // -----------------------------
  pipe_reg_t id_ex_next = cpu->if_id;
  if (!cpu->if_id.valid) {
    id_ex_next = (pipe_reg_t){.inst = make_nop(), .pc = 0, .valid = 0};
  }

  // -----------------------------
  // Stage IF: fetch into IF/ID
  // -----------------------------
  pipe_reg_t if_id_next = (pipe_reg_t){.inst = make_nop(), .pc = 0, .valid = 0};
  uint32_t pc_next = cpu->pc;

  if (!cpu->halted && cpu->pc < program_len) {
    if_id_next.inst = program[cpu->pc];
    if_id_next.pc = cpu->pc;
    if_id_next.valid = 1;
    pc_next = cpu->pc + 1;
  }

  if (redirect) {
    pc_next = (uint32_t)redirect_pc;
    if (squash_younger) {
      if_id_next.valid = 0;
      id_ex_next.valid = 0;
    }
  }

  cpu->pc = pc_next;
  cpu->if_id = if_id_next;
  cpu->id_ex = id_ex_next;

  return 0;
}

int run_program(cpu_t *cpu, const instr_t *program, size_t program_len,
                uint64_t max_steps) {
  return run_with_debug(cpu, program, program_len, max_steps, 0);
}

int run_with_debug(cpu_t *cpu, const instr_t *program, size_t program_len,
                   uint64_t max_steps, uint64_t debug_period) {
  for (uint64_t step = 0; step < max_steps; step++) {
    int r = tick(cpu, program, program_len);

    if (debug_period > 0 && (cpu->cycles % debug_period) == 0) {
      dump_pipeline(cpu);
    }

    if (r == 1)
      return 0;
    if (r == -1)
      return -1;
  }
  return 1;
}

int run_and_dump(cpu_t *cpu, const instr_t *program, size_t program_len,
                 uint64_t max_steps, bool dump_enable,
                 const char *dump_filename) {
  if (!dump_enable) {
    return run_program(cpu, program, program_len, max_steps);
  }
  if (dump_filename == NULL || dump_filename[0] == '\0') {
    fprintf(stderr,
            "run_and_dump: dump_filename is required when dump_enable=true\n");
    return -1;
  }

  FILE *fp = fopen(dump_filename, "w");
  if (!fp) {
    fprintf(stderr, "run_and_dump: failed to open '%s': %s\n", dump_filename,
            strerror(errno));
    return -1;
  }

  fprintf(fp, "# pipeline dump (one entry per cycle)\n");
  dump_pipeline_to(cpu, fp);

  for (uint64_t step = 0; step < max_steps; step++) {
    int r = tick(cpu, program, program_len);
    dump_pipeline_to(cpu, fp);

    if (r == 1) {
      fclose(fp);
      return 0;
    }
    if (r == -1) {
      fclose(fp);
      return -1;
    }
  }

  fclose(fp);
  return 1;
}
