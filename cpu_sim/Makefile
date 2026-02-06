CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2 -Isrc

MMAIN=src/main.c
TEST ?= vecadd
MTEST=tests/test_$(TEST).c

SRC=src/core/regfile.c \
    src/core/memory.c \
    src/core/cpu.c \
    src/isa/exec.c

MOUT=main
TOUT=test

all: clean-main compile-main
	./$(MOUT)

compile-main:
	$(CC) $(CFLAGS) $(MMAIN) $(SRC) -o $(MOUT)

compile-test:
	$(CC) $(CFLAGS) $(MTEST) $(SRC) -o $(TOUT)

clean-all: clean-main clean-test
clean-main:
	rm -f $(MOUT)

clean-test:
	rm -f $(TOUT)

test: clean-test compile-test
	./$(TOUT)

.SILENT:
test-all:
	$(CC) $(CFLAGS) tests/test_bubblesort.c 	$(SRC) 	-o test_bubblesort
	$(CC) $(CFLAGS) tests/test_fac.c 			$(SRC)  -o test_fac
	$(CC) $(CFLAGS) tests/test_gcd.c 			$(SRC) 	-o test_gcd
	$(CC) $(CFLAGS) tests/test_ham_wei.c 		$(SRC) 	-o test_ham_wei
	$(CC) $(CFLAGS) tests/test_liver_loops.c 	$(SRC)  -o test_liver_loops
	$(CC) $(CFLAGS) tests/test_vec_add.c 		$(SRC) 	-o test_vec_add

	echo ""
	./test_bubblesort
	echo ""
	./test_fac
	echo ""
	./test_gcd
	echo ""
	./test_ham_wei
	echo ""
	./test_liver_loops
	echo ""
	./test_vec_add

	rm -f test_bubblesort
	rm -f test_fac
	rm -f test_gcd
	rm -f test_ham_wei
	rm -f test_liver_loops
	rm -f test_vec_add