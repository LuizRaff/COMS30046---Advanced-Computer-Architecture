#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef int32_t Word;

#define NUM_REGS 32
#define VLEN 4
#define NUM_VREG 8

typedef struct {
  Word e[VLEN];
} VectorRegister;

#endif