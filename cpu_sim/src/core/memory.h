#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H

#include "types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  word_t *words;
  size_t nwords;
} memory_t;

int mem_init(memory_t *m, size_t nwords);
void mem_free(memory_t *m);

word_t mem_load(const memory_t *m, uint32_t addr_bytes);
void mem_store(memory_t *m, uint32_t addr_bytes, word_t value);

#endif
