CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2 -Isrc

MMAIN=src/main.c 
MTEST=src/test.c
      
SRC=src/core/regfile.c \
      src/isa/exec.c \
	  src/core/memory.c

MOUT = sim
TOUT = test

all:
	make compile-main
	./$(MOUT)

compile-main:
	$(CC) $(CFLAGS) $(MMAIN) $(SRC) -o $(MOUT)

compile-test:
	$(CC) $(CFLAGS) $(MTEST) $(SRC) -o $(TOUT)

clean:
	rm -f $(MOUT)
	rm -f $(TOUT)

test:
	make compile-test
	./$(TOUT)