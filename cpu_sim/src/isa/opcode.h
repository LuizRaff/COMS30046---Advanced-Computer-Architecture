#ifndef ISA_OPCODE_H
#define ISA_OPCODE_H

typedef enum {
  OP_NOP,

  OP_LD,
  OP_LDC,
  OP_ST,

  OP_ADD,
  OP_ADDI,
  OP_SUB,
  OP_SUBI,

  OP_MUL,
  OP_MULI,

  OP_AND,
  OP_OR,
  OP_XOR,
  OP_NOT,

  OP_CMP,

  OP_SHR,
  OP_SHL,

  OP_B,
  OP_J,
  OP_BLTH,

  OP_HALT,

  /* Vector integer instructions */
  OP_VLD,   /* VLD  vd, [rbase + imm]  — load VLEN ints  */
  OP_VST,   /* VST  vs, [rbase + imm]  — store VLEN ints */
  OP_VADD,  /* VADD vd, vs1, vs2       — element-wise add */
  OP_VMUL   /* VMUL vd, vs1, vs2       — element-wise mul */
} opcode_t;

#endif // ISA_OPCODE_H