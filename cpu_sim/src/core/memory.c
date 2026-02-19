#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void mem_fault(const char *msg, uint32_t addr) {
  fprintf(stderr, "MEM FAULT: %s (addr=0x%08x)\n", msg, addr);
  exit(1);
}

int memory_init(memory_t *m, size_t nwords) {
  m->words = (word_t *)calloc(nwords, sizeof(word_t));
  if (!m->words)
    return -1;
  m->nwords = nwords;
  return 0;
}

void memory_free(memory_t *m) {
  free(m->words);
  m->words = NULL;
  m->nwords = 0;
}

word_t memory_load_w(const memory_t *m, uint32_t addr_bytes) {
  if (addr_bytes & 3u)
    mem_fault("unaligned word load", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_fault("out of bounds load", addr_bytes);
  return m->words[idx];
}

void memory_store_w(memory_t *m, uint32_t addr_bytes, word_t value) {
  if (addr_bytes & 3u)
    mem_fault("unaligned word store", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_fault("out of bounds store", addr_bytes);
  m->words[idx] = value;
}
