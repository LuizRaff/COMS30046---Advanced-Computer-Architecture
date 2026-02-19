#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H

#include "types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  word_t *words;
  size_t nwords;
} memory_t;

int memory_init(memory_t *m, size_t nwords);
void memory_free(memory_t *m);

word_t memory_load_w(const memory_t *m, uint32_t addr_bytes);
void memory_store_w(memory_t *m, uint32_t addr_bytes, word_t value);

#endif // CORE_MEMORY_H
