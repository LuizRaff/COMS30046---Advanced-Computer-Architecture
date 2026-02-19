#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void mem_err(const char *msg, uint32_t addr) {
  fprintf(stderr, "MEM FAULT: %s (addr=0x%08x)\n", msg, addr);
  exit(1);
}

int mem_init(memory_t *m, size_t nwords) {
  m->words = (word_t *)calloc(nwords, sizeof(word_t));
  if (!m->words)
    return -1;
  m->nwords = nwords;
  return 0;
}

void mem_free(memory_t *m) {
  free(m->words);
  m->words = NULL;
  m->nwords = 0;
}

word_t mem_load(const memory_t *m, uint32_t addr_bytes) {
  if (addr_bytes & 3u)
    mem_err("unaligned word load", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_err("out of bounds load", addr_bytes);
  return m->words[idx];
}

void mem_store(memory_t *m, uint32_t addr_bytes, word_t value) {
  if (addr_bytes & 3u)
    mem_err("unaligned word store", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_err("out of bounds store", addr_bytes);
  m->words[idx] = value;
}
