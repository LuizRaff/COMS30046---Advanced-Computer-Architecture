#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef int32_t word_t;

#define NUM_REGS 32
#define VLEN     4
#define NUM_VREG 8

typedef struct {
  word_t e[VLEN];
} vreg_t;

#endif