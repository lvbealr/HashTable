section .text
global createNode
extern malloc, free

createNode:
    push rbx
    push r12
    push r13

    mov r12, rdi                ; wordPtr
    mov r13, rsi                ; length

    mov edi, 16
    call malloc
    test rax, rax
    jz .error

    mov rbx, rax
    mov [rbx + 8], r13          ; newNode->size = length

    lea rdi, [r13 + 1]
    call malloc
    test rax, rax
    jz .freeError

    mov [rbx], rax              ; newNode->data = rax

    mov rdi, rax                ; destination
    mov rsi, r12                ; source
    mov rcx, r13                ; length
    rep movsb                   ; copy length byte
    mov byte [rax + r13], 0

    mov rax, rbx                ; return newNode
    jmp .end

.freeError:
    mov rdi, rbx
    call free

.error:
    xor rax, rax

.end:
    pop r13
    pop r12
    pop rbx

    ret