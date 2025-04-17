global myStrcmp

section .text

myStrcmp:
    xor rdx, rdx
    xor rax, rax

.loop:
    movdqu xmm1, [rdi + rdx]
    pcmpistri xmm1, [rsi + rdx], 0x18

    jc .diff
    jz .equal

    add rdx, 16
    jmp .loop

.equal:
    xor eax, eax
    ret

.diff:
    lea r9, [rdx + rcx]
    movzx eax, byte [rdi + r9]
    movzx r8d, byte [rsi + r9]
    sub eax, r8d
    ret