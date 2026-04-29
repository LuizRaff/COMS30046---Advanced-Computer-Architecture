#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H

#include "types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  Word *words;
  size_t nwords;
} MemoryBank;

int mem_init(MemoryBank *m, size_t nwords);
void mem_free(MemoryBank *m);

Word mem_load(const MemoryBank *m, uint32_t addr_bytes);
void mem_store(MemoryBank *m, uint32_t addr_bytes, Word value);

#endif
