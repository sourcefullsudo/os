; kernel.asm
[bits 64]
[global start]
start:
    mov rax, 0x12345678  ; Example operation
    hlt                  ; Halt the CPU