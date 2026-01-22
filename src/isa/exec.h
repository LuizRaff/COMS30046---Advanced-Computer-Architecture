#ifndef ISA_EXEC_H
#define ISA_EXEC_H

#include "../core/regfile.h"
#include "instruction.h"

void exec_instr(const instr_t *inst, regfile_t *rf);

#endif // ISA_EXEC_H