#ifndef ISA_EXEC_H
#define ISA_EXEC_H

#include "../core/regfile.h"
#include "../core/memory.h"
#include "instruction.h"

void exec_instr(const instr_t *inst, regfile_t *rf, memory_t *mem);

#endif // ISA_EXEC_H