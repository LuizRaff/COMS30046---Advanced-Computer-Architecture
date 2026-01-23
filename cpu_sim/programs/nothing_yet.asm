; notyet.asm (NASM, Linux x86-64)
global _start

section .data
    msg db "not yet baby", 10      ; 10 = '\n'
    len equ $ - msg

section .text
_start:
    ; write(1, msg, len)
    mov rax, 1          ; syscall: sys_write
    mov rdi, 1          ; fd = stdout
    mov rsi, msg        ; buffer
    mov rdx, len        ; size
    syscall

    ; exit(0)
    mov rax, 60         ; syscall: sys_exit
    xor rdi, rdi        ; status = 0
    syscall
