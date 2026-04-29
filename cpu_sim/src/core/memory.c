#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void mem_err(const char *msg, uint32_t addr) {
  fprintf(stderr, "MEM FAULT: %s (addr=0x%08x)\n", msg, addr);
  exit(1);
}

/* Initialises memory */
int mem_init(MemoryBank *m, size_t nwords) {
  m->words = (Word *)calloc(nwords, sizeof(Word));
  if (!m->words)
    return -1;
  m->nwords = nwords;
  return 0;
}

/* Frees the memory */
void mem_free(MemoryBank *m) {
  free(m->words);
  m->words = NULL;
  m->nwords = 0;
}

/* Reads a single 32-bit word from the given byte address, ensures word
 * alignment */
Word mem_load(const MemoryBank *m, uint32_t addr_bytes) {
  if (addr_bytes & 3u)
    mem_err("unaligned word load", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_err("out of bounds load", addr_bytes);
  return m->words[idx];
}

/* Writes a single 32-bit word into the given byte address, ensures word
 * alignment */
void mem_store(MemoryBank *m, uint32_t addr_bytes, Word value) {
  if (addr_bytes & 3u)
    mem_err("unaligned word store", addr_bytes);
  uint32_t idx = addr_bytes >> 2;
  if (idx >= m->nwords)
    mem_err("out of bounds store", addr_bytes);
  m->words[idx] = value;
}
