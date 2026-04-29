# Superscalar CPU Simulator

This is a simple cycle-level CPU simulator written in C for the Advanced Computer Architecture assignment.

The simulator models the core mechanisms of a superscalar, out-of-order processor:

- instruction fetch and issue;
- reservation stations;
- multiple execution units;
- reorder buffer with in-order commit;
- register renaming through a register alias table;
- configurable branch prediction;
- configurable issue width and ALU count;
- a simple integer vector unit.

The goal of the simulator is not to model a commercial CPU in full detail. The goal is to isolate a few architectural features and show how they affect cycles, IPC, branch accuracy, flushes and dynamic instruction count.

---

## 1. Build and Run

Everything needs to be run/compiled inside 'cpu_sim' directory.

### Compile

```bash
make clean
make
```

This builds the simulator executable:

```bash
./main
```

### Run a single benchmark

```bash
./main --benchmark=branch_pattern --bp=two-level --issue-width=2 --alus=2
```

### Run the basic smoke test

```bash
make test
```

### Run all experiment suites

```bash
make test-all
```

---

## 2. Configuration Options

The simulator accepts the following command-line options:

```bash
./main --benchmark=NAME [--bp=MODE] [--issue-width=N] [--alus=N]
```

### Benchmarks

| Benchmark               | Purpose                                                      |
| ----------------------- | ------------------------------------------------------------ |
| `independent_alu`       | Independent ALU operations; used to test superscalar scaling |
| `dependent_alu`         | Dependent ALU chain; used to show dependency bottlenecks     |
| `branch_loop`           | Simple loop branch behavior                                  |
| `branch_pattern`        | Repeating branch pattern; used to compare branch predictors  |
| `scalar_vector_add`     | Scalar vector-add baseline                                   |
| `vectorized_vector_add` | Vectorized vector-add using the vector unit                  |

### Branch predictor modes

| Mode        | Behavior                                      |
| ----------- | --------------------------------------------- |
| `none`      | Always predicts not taken                     |
| `always`    | Always predicts taken                         |
| `two-bit`   | Uses 2-bit saturating counters                |
| `two-level` | Uses local branch history and a pattern table |

Default branch predictor:

```bash
--bp=two-bit
```

### Superscalar configuration

| Option            | Meaning                                         | Default |
| ----------------- | ----------------------------------------------- | ------: |
| `--issue-width=N` | Maximum number of instructions issued per cycle |       2 |
| `--alus=N`        | Number of ALU execution units                   |       2 |

Example:

```bash
./main --benchmark=independent_alu --issue-width=4 --alus=4
```

---

## 3. Processor Model

The processor model includes:

- scalar integer register file;
- memory for loads and stores;
- instruction queue;
- reservation stations;
- ALU, load/store, branch and vector execution units;
- reorder buffer for precise in-order commit;
- register alias table for register renaming;
- branch prediction and pipeline flush on misprediction;
- vector register file with fixed vector length of 4 integers.

The main execution flow is:

1. fetch instructions;
2. issue instructions into the ROB and reservation stations;
3. wait until operands are ready;
4. execute ready instructions on available execution units;
5. broadcast results to waiting reservation stations;
6. mark ROB entries as ready;
7. commit completed instructions in program order.

---

## 4. Experiment 1: Branch Prediction

### Hypothesis

Dynamic branch prediction should reduce mispredictions, branch flushes and total cycles compared with simple static prediction, especially when the branch behavior follows a repeated pattern.

### Command

```bash
make test-branch
```

### Key result: `branch_pattern`

| Predictor | Cycles |  IPC | Branch accuracy | Branch flushes |
| --------- | -----: | ---: | --------------: | -------------: |
| none      |    298 | 0.57 |          28.33% |             43 |
| always    |    173 | 0.98 |          76.62% |             17 |
| two-bit   |    196 | 0.87 |          60.66% |             23 |
| two-level |    136 | 1.25 |          83.61% |             10 |

### Interpretation

The two-level predictor performs best on the patterned branch benchmark. It learns repeated local branch behavior better than static prediction and the simple two-bit counter. This reduces branch flushes and improves IPC.

For the simpler `branch_loop` benchmark, `always` performs best because most loop branches are taken until the final iteration. This is expected and shows that the best predictor depends on the branch pattern.

---

## 5. Experiment 2: Superscalar Scaling

### Hypothesis

Increasing issue width and the number of ALUs should improve IPC when the program has enough instruction-level parallelism. The same hardware increase should have limited benefit when the instruction stream is dominated by true data dependencies.

### Command

```bash
make test-superscalar
```

### Key result: `independent_alu`

| Configuration          | Cycles |  IPC |
| ---------------------- | -----: | ---: |
| issue-width 1 / 1 ALU  |     25 | 0.84 |
| issue-width 2 / 2 ALUs |     15 | 1.40 |
| issue-width 4 / 4 ALUs |     10 | 2.10 |

### Key result: `dependent_alu`

| Configuration          | Cycles |  IPC |
| ---------------------- | -----: | ---: |
| issue-width 1 / 1 ALU  |     36 | 0.89 |
| issue-width 2 / 2 ALUs |     35 | 0.91 |
| issue-width 4 / 4 ALUs |     35 | 0.91 |

### Interpretation

The independent benchmark scales well because the out-of-order backend can issue multiple ready instructions to multiple ALUs. The dependent benchmark does not scale because each instruction waits for the previous one. In that case, the bottleneck is the dependency chain, not the amount of hardware.

---

## 6. Experiment 3: Vector Unit

### Hypothesis

Vector instructions should reduce dynamic instruction count and total cycles for data-parallel workloads because one vector instruction processes multiple data elements.

### Command

```bash
make test-vector
```

### Result

| Benchmark             | Correctness | Cycles | Instructions committed |  IPC |
| --------------------- | ----------- | -----: | ---------------------: | ---: |
| scalar_vector_add     | PASS        |    143 |                    150 | 1.05 |
| vectorized_vector_add | PASS        |     49 |                     42 | 0.86 |

### Interpretation

The vectorized benchmark commits fewer instructions and completes in fewer cycles. IPC is slightly lower, but overall performance improves because each vector instruction performs work on 4 integers at once.

---

## 7. Output Metrics

Each run prints a metrics block like this:

```text
=== Simulation Results ===
Benchmark:           branch_pattern
Branch predictor:    two-level-local
Issue width:         2
ALUs:                2
Vector unit:         disabled
Cycles:              136
Instructions committed: 170
IPC:                 1.25
Branches:            61
Correct predictions: 51
Mispredictions:      10
Branch accuracy:     83.61%
Branch flushes:      10
ROB full stalls:     0
RS full stalls:      0
==========================
```

---

## 8. Known Limitations

This simulator intentionally keeps several parts simple:

1. **No cache hierarchy**  
   Memory is modeled as a simple memory system. There is no L1/L2 cache model.

2. **Integer-only execution**  
   There is no floating-point unit.

3. **Single-core only**  
   There is no multicore or simultaneous multithreading support.

4. **Conservative memory ordering**  
   Loads are blocked behind older unresolved stores to avoid incorrect memory behavior without implementing a full load/store queue.

5. **Simple vector model**  
   The vector unit uses 8 vector registers with fixed vector length 4. Vector register renaming is not implemented, so vector operations are kept simple and ordered enough to preserve correctness.

6. **Simplified timing model**  
   The simulator is cycle-level, but it is not intended to reproduce the exact timing of a real commercial processor.
