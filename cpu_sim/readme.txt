# C Superscalar CPU Simulator (Stage 5)

A cycle-level superscalar out-of-order CPU simulator written in C, developed for the Advanced Computer Architecture coursework.

## Architectural Features
- **Tomasulo's Algorithm:** Reservation stations (RS) and Common Data Bus (CDB) broadcasting.
- **In-Order Commit:** Reorder Buffer (ROB) supporting precise state recovery and in-order commit.
- **Register Renaming:** Register Alias Table (RAT) breaking WAW/WAR dependencies.
- **Non-blocking Issue:** Multi-issue instruction queue and issue logic that stalls gracefully on structural hazards (ROB/RS full).
- **Configurable Superscalar Execution:** Parameterized issue width and multiple parameterized ALUs.
- **Configurable Branch Prediction:** 
  - `none` (always not-taken)
  - `always` (always taken)
  - `two-bit` (saturating counters)
  - `two-level` (per-branch local history with pattern history table)
- **Vector Unit:** Simple integer vector instructions (VLEN=4) including `vld`, `vst`, `vadd`, `vmul`.
- **Memory Ordering:** Simple/conservative memory ordering (loads wait for all older unresolved stores).

---

## How to Compile

A standard `Makefile` is provided. The simulator requires no external dependencies.

```bash
# Clean and compile the main simulator
make clean-main && make compile-main

# Run the old legacy tests (verifies correctness)
make test-all
```

---

## How to Run Experiments

The main simulator executable (`./main`) contains embedded benchmarks to make testing structural changes simple. It outputs a presentation-ready metrics block at the end of execution.

### Command-Line Options
- `--benchmark=NAME`: Selects the benchmark to run (required).
- `--bp=MODE`: Selects the branch predictor (`none`, `always`, `two-bit`, `two-level`). Default is `two-bit`.
- `--issue-width=N`: Sets the superscalar issue and commit width. Default is 2.
- `--alus=N`: Sets the number of available ALU execution units. Default is 2.

### Available Benchmarks
- `independent_alu`: Chain of independent instructions (tests issue-width & ALU scaling).
- `dependent_alu`: Chain of dependent instructions (tests data dependency stalling).
- `branch_loop`: Loop-heavy benchmark (tests branch predictor accuracy).
- `branch_pattern`: Complex alternating branch pattern (tests two-level predictor).
- `scalar_vector_add`: Scalar baseline of vector addition loop.
- `vectorized_vector_add`: Hardware vector-unit version of the addition loop.

---

## Stage 5 Sample Experiment Commands

### 1. Branch Predictor Comparison
Observe how branch accuracy and flushes change across different predictors on a complex branch pattern.
```bash
./main --benchmark=branch_pattern --bp=none
./main --benchmark=branch_pattern --bp=always
./main --benchmark=branch_pattern --bp=two-bit
./main --benchmark=branch_pattern --bp=two-level
```

### 2. Superscalar Scaling (Issue Width & ALUs)
Observe how IPC scales when providing more hardware resources to an independent instruction stream.
```bash
./main --benchmark=independent_alu --issue-width=1 --alus=1
./main --benchmark=independent_alu --issue-width=2 --alus=2
./main --benchmark=independent_alu --issue-width=4 --alus=4
```
*(You can also compare this against the `dependent_alu` benchmark, where IPC should remain bottlenecked near ~1.0 regardless of resources).*

### 3. Vectorization Speedup
Compare the cycle count and instruction count of scalar loops vs vectorized instructions.
```bash
./main --benchmark=scalar_vector_add
./main --benchmark=vectorized_vector_add
```

---

## Known Limitations
1. **No Cache Hierarchy:** Memory is modeled as a flat, perfect memory space with fixed latencies.
2. **Simple Memory Ordering:** Loads are conservatively blocked if there is *any* older uncommitted store in the ROB, preventing some theoretical ILP but ensuring correctness without complex load/store queues.
3. **Integer Only:** No floating-point execution units or registers.
4. **No Multicore/SMT:** Strictly a single-core implementation.
5. **Basic Vector Implementation:** Fixed VLEN=4 with 8 architectural vector registers. Vector writes are eager but correctly serialized. No variable-length vector support.
