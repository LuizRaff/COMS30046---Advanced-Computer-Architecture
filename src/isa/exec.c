#include "exec.h"
#include <stdio.h>

static void exec_add(const instr_t *inst, regfile_t *rf) {
    // add rd, rs1, rs2  => R[rd] = R[rs1] + R[rs2]
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a + b));
}

static void exec_addi(const instr_t *inst, regfile_t *rf) {
    // addi rd, rs1, imm => R[rd] = R[rs1] + imm
    word_t a = regfile_read(rf, inst->rs1);
    regfile_write(rf, inst->rd, (word_t)(a + inst->imm));
}

static void exec_sub(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a - b));
}

static void exec_subi(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    regfile_write(rf, inst->rd, (word_t)(a - inst->imm));
}

static void exec_mul(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a * b));
}

static void exec_muli(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    regfile_write(rf, inst->rd, (word_t)(a * inst->imm));
}

static void exec_and(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a & b));
}

static void exec_or(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a | b));
}

static void exec_xor(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    regfile_write(rf, inst->rd, (word_t)(a ^ b));
}

static void exec_not(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    regfile_write(rf, inst->rd, (word_t)(~a));
}

static void exec_cmp(const instr_t *inst, regfile_t *rf) {
    word_t a = regfile_read(rf, inst->rs1);
    word_t b = regfile_read(rf, inst->rs2);
    if (a < b){
        regfile_write(rf, inst->rd, (word_t)(1));
    } else if (a == b){
        regfile_write(rf, inst->rd, (word_t)(0));
    } else {
        regfile_write(rf, inst->rd, (word_t)(-1));
    }
}

static void exec_shl(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    uint32_t sh = (uint32_t)inst->imm & 31u;
    regfile_write(rf, inst->rd, (word_t)((uint32_t)a << sh));
}

static void exec_shr(const instr_t *inst, regfile_t *rf){
    word_t a = regfile_read(rf, inst->rs1);
    uint32_t sh = (uint32_t)inst->imm & 31u;
    regfile_write(rf, inst->rd, (word_t)(a >> sh));
}

void exec_instr(const instr_t *inst, regfile_t *rf) {
    switch (inst->op) {
        case OP_ADD:    exec_add(inst, rf);  break;
        case OP_ADDI:   exec_addi(inst, rf); break;
        case OP_SUB:    exec_sub(inst, rf); break;
        case OP_SUBI:   exec_subi(inst, rf); break;
        case OP_MUL:    exec_mul(inst, rf); break;
        case OP_MULI:   exec_muli(inst, rf); break;
        case OP_AND:    exec_and(inst, rf); break;
        case OP_OR:     exec_or(inst, rf); break;
        case OP_XOR:    exec_xor(inst, rf); break;
        case OP_NOT:    exec_not(inst, rf); break;
        case OP_CMP:    exec_cmp(inst, rf); break;
        case OP_SHR:    exec_shr(inst, rf); break;
        case OP_SHL:    exec_shl(inst, rf); break;
        default:
            printf("Unsupported opcode %d\n", inst->op);
            break;
    }
}
