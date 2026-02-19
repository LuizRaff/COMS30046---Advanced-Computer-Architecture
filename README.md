# COMS30046 — Advanced Computer Architecture
## CPU Pipeline Simulator

A cycle-accurate simulator of a **3-stage pipelined CPU** written in C, built as part of the Advanced Computer Architecture unit at the University of Bristol.

---

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
  - [Pipeline Stages](#pipeline-stages)
  - [Branch Handling](#branch-handling)
- [ISA — Instruction Set](#isa--instruction-set)
  - [Data Movement](#data-movement)
  - [Arithmetic](#arithmetic)
  - [Logic](#logic)
  - [Shifts](#shifts)
  - [Control Flow](#control-flow)
- [Core Components](#core-components)
  - [CPU (`cpu.c` / `cpu.h`)](#cpu-cpuc--cpuh)
  - [Register File (`regfile.c` / `regfile.h`)](#register-file-regfilec--regfileh)
  - [Memory (`memory.c` / `memory.h`)](#memory-memoryc--memoryh)
  - [Executor (`exec.c` / `exec.h`)](#executor-execc--exech)
- [How It Works — Step by Step](#how-it-works--step-by-step)
- [Demo Program (`main.c`)](#demo-program-mainc)
- [Performance Metrics](#performance-metrics)
- [Building & Running](#building--running)

---

## Overview

This project implements a **software simulation** of a CPU with a classic 3-stage pipeline:

```
Instruction Fetch (IF) → Instruction Decode (ID) → Execute (EX)
```

It is **not** a compiler or assembler. Programs are written directly as arrays of `instr_t` structs in C and fed to the CPU at runtime. The goal is to study and observe how pipeline mechanics — such as branch flushes, instruction throughput, and cycle counting — behave in a real microarchitecture.

---

## Project Structure

```
cpu_sim/src/
├── main.c                  # Entry point and demo program
├── core/
│   ├── types.h             # Shared primitive types (word_t, NUM_REGS)
│   ├── cpu.h / cpu.c       # CPU struct, pipeline step, run loop, stats
│   ├── memory.h / memory.c # Word-addressed data memory
│   └── regfile.h / regfile.c # 32-register register file
└── isa/
    ├── opcode.h            # Opcode enum (OP_NOP, OP_ADD, OP_B, ...)
    ├── instruction.h       # Instruction struct (instr_t)
    └── exec.h / exec.c     # Instruction execution logic
```

---

## Architecture

### Pipeline Stages

The CPU (`cpu_t`) models a **3-stage in-order pipeline** using two pipeline registers:

| Register | Connects Stages | Contents |
|----------|----------------|----------|
| `if_id`  | IF → ID        | Fetched instruction + its PC |
| `id_ex`  | ID → EX        | Decoded instruction ready to execute |

Each call to `cpu_step()` advances **all three stages simultaneously** in reverse order (EX first, then ID, then IF) to avoid read-after-write hazards on the pipeline registers:

```
Cycle N:
  1. EX  — execute whatever is in id_ex
  2. ID  — move if_id → id_ex (decode is trivial: no separate decode logic)
  3. IF  — fetch program[pc] → if_id, increment pc
```

After all stages are computed, the new state is committed atomically.

### Branch Handling

Branches are resolved in the **EX stage**. Because the pipeline has already fetched (and possibly decoded) the instruction(s) after the branch, those must be **squashed** (flushed) when a branch is taken:

- `OP_B` — absolute branch: always taken, sets `pc = imm`, squashes IF/ID and ID/EX.
- `OP_J` — relative jump: always taken, sets `pc = current_pc + imm`, squashes IF/ID and ID/EX.
- `OP_BLTH` — conditional branch: taken only if `rs1 < rs2`, sets `pc = imm`, squashes IF/ID and ID/EX.
- `OP_HALT` — stops fetching, squashes in-flight instructions, drains the pipeline.

> **Branch penalty:** Every taken branch costs **2 wasted cycles** (the two instructions already in IF and ID are squashed and replaced with bubbles).

---

## ISA — Instruction Set

All instructions are represented by the `instr_t` struct:

```c
typedef struct {
    opcode_t op;    // Operation
    int      rd;    // Destination register
    int      rs1;   // Source register 1
    int      rs2;   // Source register 2
    word_t   imm;   // Immediate value (int32_t)
    bool     has_imm;
} instr_t;
```

### Data Movement

| Opcode   | Syntax                        | Description |
|----------|-------------------------------|-------------|
| `OP_LDC` | `ldc rd, imm`                 | Load constant: `rd = imm` |
| `OP_LD`  | `ld rd, [rs1 + imm]`          | Load word from memory at address `rs1 + imm` into `rd` |
| `OP_ST`  | `st rs2 -> [rs1 + imm]`       | Store word from `rs2` to memory at address `rs1 + imm` |

### Arithmetic

| Opcode    | Syntax                   | Description |
|-----------|--------------------------|-------------|
| `OP_ADD`  | `add rd, rs1, rs2`       | `rd = rs1 + rs2` |
| `OP_ADDI` | `addi rd, rs1, imm`      | `rd = rs1 + imm` |
| `OP_SUB`  | `sub rd, rs1, rs2`       | `rd = rs1 - rs2` |
| `OP_SUBI` | `subi rd, rs1, imm`      | `rd = rs1 - imm` |
| `OP_MUL`  | `mul rd, rs1, rs2`       | `rd = rs1 * rs2` |
| `OP_MULI` | `muli rd, rs1, imm`      | `rd = rs1 * imm` |
| `OP_CMP`  | `cmp rd, rs1, rs2`       | `rd = -1` if `rs1 < rs2`, `0` if equal, `1` if greater |

### Logic

| Opcode   | Syntax              | Description |
|----------|---------------------|-------------|
| `OP_AND` | `and rd, rs1, rs2`  | `rd = rs1 & rs2` |
| `OP_OR`  | `or rd, rs1, rs2`   | `rd = rs1 \| rs2` |
| `OP_XOR` | `xor rd, rs1, rs2`  | `rd = rs1 ^ rs2` |
| `OP_NOT` | `not rd, rs1`       | `rd = ~rs1` |

### Shifts

| Opcode   | Syntax               | Description |
|----------|----------------------|-------------|
| `OP_SHL` | `shl rd, rs1, imm`   | `rd = rs1 << (imm & 31)` (logical left shift) |
| `OP_SHR` | `shr rd, rs1, imm`   | `rd = rs1 >> (imm & 31)` (arithmetic right shift) |

### Control Flow

| Opcode    | Syntax                    | Description |
|-----------|---------------------------|-------------|
| `OP_B`    | `b imm`                   | Unconditional absolute branch to instruction index `imm` |
| `OP_J`    | `j imm`                   | Unconditional relative jump: `pc += imm` |
| `OP_BLTH` | `blth rs1, rs2, imm`      | Branch to `imm` if `rs1 < rs2` |
| `OP_NOP`  | `nop`                     | Pipeline bubble, no operation |
| `OP_HALT` | `halt`                    | Stop execution and drain the pipeline |

---

## Core Components

### CPU (`cpu.c` / `cpu.h`)

The central `cpu_t` struct holds the entire processor state:

```c
typedef struct {
    uint32_t   pc;       // Program counter (next instruction to fetch)
    uint64_t   cycles;   // Total cycles elapsed
    uint64_t   instrs;   // Total non-NOP instructions retired
    pipe_reg_t if_id;    // IF/ID pipeline register
    pipe_reg_t id_ex;    // ID/EX pipeline register
    int        halted;   // Set to 1 when HALT is executed
    regfile_t  rf;       // Register file
    memory_t   mem;      // Data memory
} cpu_t;
```

Key functions:

| Function | Description |
|----------|-------------|
| `cpu_init(cpu, mem_words)` | Initialises the CPU and allocates `mem_words` words of data memory |
| `cpu_step(cpu, program, len)` | Advances the pipeline by exactly **one cycle**. Returns `0` (continue), `1` (halted & drained), `-1` (fault) |
| `cpu_run(cpu, program, len, max_steps)` | Runs `cpu_step` in a loop until halt or `max_steps` exceeded |
| `cpu_run_debug(...)` | Same as `cpu_run` but prints pipeline state every N cycles |
| `cpu_run_dump(...)` | Dumps full pipeline state to a file every cycle |
| `cpu_print_stats(cpu)` | Prints instructions retired, cycles, and IPC |
| `cpu_free(cpu)` | Frees allocated memory |

### Register File (`regfile.c` / `regfile.h`)

- **32 general-purpose registers** (`r0`–`r31`), each a 32-bit signed integer (`word_t = int32_t`).
- **`r0` is hardwired to 0**: writes to index `0` are silently ignored; reads always return `0`.
- All registers are zero-initialised on `regfile_init()`.

### Memory (`memory.c` / `memory.h`)

- A flat array of `word_t` (32-bit words), heap-allocated.
- **Word-addressed with byte-offset API**: addresses are byte offsets, but must be **4-byte aligned**. Unaligned accesses trigger a fatal `MEM FAULT` and call `exit(1)`.
- Out-of-bounds accesses also trigger a fatal fault.
- Default size in `main.c`: **64 words** (256 bytes).

```c
// Example: store 1234 at byte offset 8 (word index 2)
memory_store_w(&cpu.mem, 8, 1234);

// Load it back via a register holding address 8
// OP_LD rd=2, rs1=1 (r1=8), imm=0  →  r2 = mem[8] = 1234
```

### Executor (`exec.c` / `exec.h`)

`exec_instr()` is the single dispatch function called in the EX stage for all non-control-flow instructions. It switches on the opcode and calls the appropriate static helper (`exec_add`, `exec_ld`, `exec_st`, etc.).

Control-flow instructions (`OP_B`, `OP_J`, `OP_BLTH`, `OP_HALT`) are handled **directly inside `cpu_step()`** because they need to manipulate the pipeline registers and PC — not just the register file.

---

## How It Works — Step by Step

```
cpu_init()
    │
    ▼
cpu_run()  ──────────────────────────────────────────────────────────────┐
    │                                                                     │
    ▼                                                                     │
cpu_step() [each cycle]                                                  │
    │                                                                     │
    ├─ 1. EX stage: execute id_ex.inst                                   │
    │       ├─ control-flow? → set redirect + squash flags               │
    │       └─ other?        → call exec_instr() → updates rf / mem      │
    │                                                                     │
    ├─ 2. ID stage: move if_id → id_ex_next                              │
    │                                                                     │
    ├─ 3. IF stage: fetch program[pc] → if_id_next, pc++                 │
    │                                                                     │
    ├─ 4. Apply redirect (branch/halt): update pc, squash if needed      │
    │                                                                     │
    └─ 5. Commit: cpu->pc, cpu->if_id, cpu->id_ex = next values ────────┘
```

The simulation ends when:
1. `halted == 1` (HALT executed), **and**
2. Both `if_id.valid == 0` and `id_ex.valid == 0` (pipeline fully drained).

---

## Demo Program (`main.c`)

`main.c` demonstrates four key behaviours of the simulator:

```
Byte offset 8 in memory ← 1234
```

| Step | Instructions | What it tests |
|------|-------------|---------------|
| **1** | `LDC r1, 8` → `LD r2, [r1+0]` | Memory load: `r2` should be `1234` |
| **2** | `B 4` (skip next, land on index 4) | Absolute branch / flush: `r4` should be `222` (not `111`) |
| **3** | `LDC r10,0` / `LDC r11,5` / `LDC r12,1` / `ADD r10,r10,r12` / `BLTH r10,r11, 8` | Loop 5 times: `r10` should be `5` |
| **4** | `J +2` (skip next) | Relative jump / flush: `r5` should be `123` (not `999`) |
| **5** | `HALT` | Stop and drain |

Expected output:
```
r2=1234  (expected 1234)
r4=222   (expected 222)
r10=5    (expected 5)
r5=123   (expected 123)
instructions=N cycles=M IPC=X.XXX
```

---

## Performance Metrics

After execution, `cpu_print_stats()` reports:

| Metric | Description |
|--------|-------------|
| `instructions` | Number of non-NOP instructions retired (committed in EX) |
| `cycles` | Total clock cycles elapsed |
| `IPC` | Instructions Per Cycle = `instructions / cycles` |

Because every **taken branch** flushes 2 pipeline stages, the IPC will be below 1.0 for branch-heavy programs. A perfectly sequential program with no branches approaches **IPC = 1.0** (the theoretical maximum for this 3-stage in-order design).

---

## Building & Running

```bash
# From the project root (adjust paths as needed)
gcc -std=c11 -Wall -Wextra \
    cpu_sim/src/main.c \
    cpu_sim/src/core/cpu.c \
    cpu_sim/src/core/memory.c \
    cpu_sim/src/core/regfile.c \
    cpu_sim/src/isa/exec.c \
    -I cpu_sim/src \
    -o cpu_sim

./cpu_sim
```