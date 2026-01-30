# COMS30046---Advanced-Computer-Architecture

## Table of Contents
- [Intro](#intro)
- [Stage1](#stage-1)
    - [Features](#features)
    - [Memory](#memory)
    - [Control-Flow](#control-flow)

## Introduction
The code presented in this repository aims to reproduce what a ISA (Instruction Set Architecture) does in a real life computer. It is important to notice that this is not a Compiler or nothing similar to it, it is a CPU simaltor, its purpose is to test and show how branch prediction, buffer handling, out-of-order instructions, etc... Behave. 

### Features
This simulator is capable of "executing" assembly like code and have the following features:

- OP_LD: Load a value from memory into a register.
    - Example:
- OP_LDC: Load a immidiate value into a register.
    - Example:
- OP_ST: Stores a value, immidiate or from a register, into memory.
    - Example:
- OP_ADD: Adds two register and stores the sum in another register.
    - Example:
- OP_ADDI: Adds a immidiate value into a register and stores the sum indo another register.
    - Example:
- OP_SUB: Subtracts two register and stores the difference in another register.
    - Example:
- OP_SUBI: Subtracts a immidiate value into a register and stores the difference in another register.
    - Example:
- OP_MUL: Multiplies two register and stores the product in another register.
    - Example:
- OP_MULI: Multiplies a immidiate value into a register and stores the product in another register.
    - Example:
- OP_AND: Executes the "and" logic operation between two registers and stores the result in another register.
    - Example:
- OP_OR: Executes the "or" logic operation between two registers and stores the result in another register.
    - Example:
- OP_XOR: Executes the "xor" logic operation between two registers and stores the result in another register.
    - Example:
- OP_NOT: Executes the "not" logic operation between two registers and stores the result in another register.
    - Example:
- OP_SHR: Executes the "Shift Right" logic operation in the value of a register and stores the result in another register.
    - Example:
- OP_SHL: Executes the "Shift Left" logic operation in the value of a register and stores the result in another register.
    - Example:
- OP_CMP: Compares two registers and set the result in another register.
    - Example:

### Memory
In this simulator memory is disguised as a struct which has a pointer to the "memory", a vector of 32bit words, and the number of total values in this memory. The ideia of having it separate from the rest of the code and its own thing, is to simulate the memory stack available which the user can change as he pleases or stick with the 64 base value.

### Control-Flow