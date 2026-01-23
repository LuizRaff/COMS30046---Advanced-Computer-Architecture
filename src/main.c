#include <stdio.h>
#include "core/regfile.h"
#include "core/memory.h"
#include "isa/exec.h"

int main(void) {
    regfile_t rf;
    memory_t mem;
    regfile_init(&rf);
    memory_init(&mem, 64); // 64 words = 256 bytes

    // escreve um valor em mem[8 bytes] (word index 2)
    memory_store_w(&mem, 8, 1234);

    // r1 = 8 (endereço base)
    instr_t i1 = {.op=OP_LDC, .rd=1, .imm=8, .has_imm=true};
    exec_instr(&i1, &rf, &mem);

    // r2 = MEM[r1 + 0]
    instr_t i2 = {.op=OP_LD, .rd=2, .rs1=1, .imm=0, .has_imm=true};
    exec_instr(&i2, &rf, &mem);

    printf("r2=%d (expected 1234)\n", regfile_read(&rf, 2));

    memory_free(&mem);
    return 0;
}
