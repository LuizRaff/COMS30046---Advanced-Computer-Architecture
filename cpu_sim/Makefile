CC := gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O0 -Isrc

MOUT := main
SRC := src/main.c \
       src/core/regfile.c \
       src/core/memory.c \
       src/core/cpu.c \
       src/isa/exec.c

RESULTS_DIR := results

.PHONY: all build clean test test-all test-vector test-branch test-superscalar run-demo help

all: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(MOUT)

clean:
	rm -f $(MOUT) test test_* *.o src/*.o src/core/*.o src/isa/*.o

test: build
	@echo ""
	@echo "== Smoke test: default configurations =="
	./$(MOUT) --benchmark=independent_alu
	./$(MOUT) --benchmark=dependent_alu
	./$(MOUT) --benchmark=branch_loop
	./$(MOUT) --benchmark=branch_pattern
	./$(MOUT) --benchmark=scalar_vector_add
	./$(MOUT) --benchmark=vectorized_vector_add
	@echo "== Smoke test complete =="

test-all: test test-vector test-branch test-superscalar

test-vector: build
	@mkdir -p $(RESULTS_DIR)
	@echo ""
	@echo "== Vector unit experiment =="
	./$(MOUT) --benchmark=scalar_vector_add | tee $(RESULTS_DIR)/scalar_vector_add.txt
	./$(MOUT) --benchmark=vectorized_vector_add | tee $(RESULTS_DIR)/vectorized_vector_add.txt
	@echo "== Vector unit experiment complete =="

test-branch: build
	@mkdir -p $(RESULTS_DIR)
	@echo ""
	@echo "== Branch prediction experiment: branch_pattern =="
	./$(MOUT) --benchmark=branch_pattern --bp=none | tee $(RESULTS_DIR)/branch_pattern_bp_none.txt
	./$(MOUT) --benchmark=branch_pattern --bp=always | tee $(RESULTS_DIR)/branch_pattern_bp_always.txt
	./$(MOUT) --benchmark=branch_pattern --bp=two-bit | tee $(RESULTS_DIR)/branch_pattern_bp_two_bit.txt
	./$(MOUT) --benchmark=branch_pattern --bp=two-level | tee $(RESULTS_DIR)/branch_pattern_bp_two_level.txt
	@echo ""
	@echo "== Branch prediction experiment: branch_loop =="
	./$(MOUT) --benchmark=branch_loop --bp=none | tee $(RESULTS_DIR)/branch_loop_bp_none.txt
	./$(MOUT) --benchmark=branch_loop --bp=always | tee $(RESULTS_DIR)/branch_loop_bp_always.txt
	./$(MOUT) --benchmark=branch_loop --bp=two-bit | tee $(RESULTS_DIR)/branch_loop_bp_two_bit.txt
	./$(MOUT) --benchmark=branch_loop --bp=two-level | tee $(RESULTS_DIR)/branch_loop_bp_two_level.txt
	@echo "== Branch prediction experiment complete =="

test-superscalar: build
	@mkdir -p $(RESULTS_DIR)
	@echo ""
	@echo "== Superscalar scaling experiment: independent_alu =="
	./$(MOUT) --benchmark=independent_alu --issue-width=1 --alus=1 | tee $(RESULTS_DIR)/independent_alu_w1_a1.txt
	./$(MOUT) --benchmark=independent_alu --issue-width=2 --alus=2 | tee $(RESULTS_DIR)/independent_alu_w2_a2.txt
	./$(MOUT) --benchmark=independent_alu --issue-width=4 --alus=4 | tee $(RESULTS_DIR)/independent_alu_w4_a4.txt
	@echo ""
	@echo "== Superscalar scaling experiment: dependent_alu =="
	./$(MOUT) --benchmark=dependent_alu --issue-width=1 --alus=1 | tee $(RESULTS_DIR)/dependent_alu_w1_a1.txt
	./$(MOUT) --benchmark=dependent_alu --issue-width=2 --alus=2 | tee $(RESULTS_DIR)/dependent_alu_w2_a2.txt
	./$(MOUT) --benchmark=dependent_alu --issue-width=4 --alus=4 | tee $(RESULTS_DIR)/dependent_alu_w4_a4.txt
	@echo "== Superscalar scaling experiment complete =="

run-demo: build
	./$(MOUT) --benchmark=branch_pattern --bp=two-level --issue-width=2 --alus=2

help:
	@echo "Available targets:"
	@echo "  make                 Build simulator"
	@echo "  make clean           Remove build artifacts"
	@echo "  make test            Run smoke test with default configs"
	@echo "  make test-vector     Run scalar vs vector benchmark"
	@echo "  make test-branch     Run branch predictor comparison"
	@echo "  make test-superscalar Run issue width / ALU scaling experiment"
	@echo "  make test-all        Run all tests and experiment suites"
	@echo "  make run-demo        Run one representative demo benchmark"
