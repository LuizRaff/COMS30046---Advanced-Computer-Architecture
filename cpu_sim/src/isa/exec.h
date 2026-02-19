#ifndef ISA_EXEC_H
#define ISA_EXEC_H

#include "../core/memory.h"
#include "../core/regfile.h"
#include "instruction.h"

void run_instr(const instr_t *inst, regfile_t *rf, memory_t *mem);

#endif