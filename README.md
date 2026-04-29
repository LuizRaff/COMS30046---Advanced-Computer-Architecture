# Superscalar CPU Simulator

This is a simple cycle-level CPU simulator implemented in C for the Advanced Computer Architecture assignment. The CPU simulated within the simulator has the following features:

- instruction fetch and issue
- reservation stations
- multiple execution units (ALU, load/store, branch, vector)
- out-of-order execution with in-order commit (reorder buffer)
- register renaming using the register alias table
- branch prediction
- configurable issue width and ALU count
- an integer vector unit

The goal of this CPU is not to provide a realistic model of any commercial CPU. Instead, the goal is to implement each of these features individually, such that they can be tested individually to observe their effect on the simulator.

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

| Benchmark             | Description                                                                                                                                                |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| independent_alu       | A workload composed of operations that can be performed in parallel by the superscalar execution units; used to test the superscalar capability of the CPU |
| dependent_alu         | A workload composed of operations that are performed in sequence (due to data dependency); used to test the ability of the CPU to stall execution units    |
| branch_loop           | A loop that branches to itself; used to test branch prediction and pipeline flush on misprediction                                                         |
| branch_pattern        | A workload whose branches can be predicted by the branch predictors; used to compare the accuracy of the branch prediction units                           |
| scalar_vector_add     | A workload that performs vector operations with both scalar and vector units                                                                               |
| vectorized_vector_add | A workload that performs vector operations using the vector execution unit                                                                                 |

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

The features of the processor within the simulator are the following:

- scalar integer register file
- memory for loads and stores
- instruction queue
- reservation stations
- ALU, load/store, branch and vector execution units
- reorder buffer
- register alias table
- branch prediction and flush on misprediction
- vector register file of fixed length of 4 integers

The execution flow for the simulator is:

- fetch instructions
- issue instructions into the reorder buffer and reservation stations
- wait for instructions to become ready (have all operands)
- execute issued instructions in the execution units
- broadcast results to reservation stations
- mark instructions in the reorder buffer as ready
- commit reordered instructions in program order

---

## 4. Experiment 1: Branch Prediction

### Hypothesis

Dynamic branch prediction will reduce the number of mispredictions, branch flushes, and cycles taken by the processor relative to static prediction alone.

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

The two-level predictor works best on benchmarks with patterned branches. It can learn the local branch patterns better than static prediction and the two-bit counter. This reduces branch flushes and improves the instruction fetch and execute cycle.

For the branch_loop benchmark, the always predictor is the best. This is because most of the branches in the loop are taken until the last iteration. The best predictor for each benchmark depends on the pattern of branches in that benchmark.

---

## 5. Experiment 2: Superscalar Scaling

### Hypothesis

Increasing the issue width and the number of ALUs will improve the IPC for programs with enough ILP. Adding more hardware will have limited benefit for programs with many true data dependencies between instructions.

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

The independent benchmark will scale well with increasing issue width and number of ALUs. The dependent benchmark will not scale because each instruction in the dependent chain will have to wait for the previous one to complete. The dependencies will be the limiting factor in the execution of the dependent benchmark.

---

## 6. Output Metrics

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

## 7. Known Limitations

The simulator is intentionally simple with a few features removed or simplified:

1. No cache hierarchy. Memory is modeled as a simple memory system with no L1 or L2 cache model.

2. Integer-only unit. The simulator has no floating point unit.

3. Single-core processor. There is no support for multicore or multithreading.

4. Memory ordering. Loads can be blocked behind stores with unresolved addresses to simulate dependencies without implementing a load/store queue.

5. Vector unit. The vector unit has 8 registers with a length of 4. There is no vector register renaming.

6. Timing model. The simulator can model the processor at the cycle level but does not aim to model the real commercial processor’s timing.
