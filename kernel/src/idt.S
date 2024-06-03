.globl realHandler
.extern realHandler

.macro _swapgs_if_user
    cmpq $0x18, 24(%rsp)
    jne 1f
    swapgs
1:
.endm

isr_handler_stub:
    _swapgs_if_user
    pushq %rdi
    pushq %rsi
    pushq %rbp
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %cr0, %rax
    pushq %rax
    movq %cr2, %rax
    pushq %rax
    movq %cr3, %rax
    pushq %rax
    movq %cr4, %rax
    pushq %rax

    movq %ds, %rax
    pushq %rax
    movq %es, %rax
    pushq %rax

    cld

    movq %rsp, %rdi
    movq 168(%rsp), %rbx
    shlq $3, %rbx
    leaq realHandler(%rip), %rax
    addq %rbx, %rax
    callq *(%rax)

    addq $48, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rbp
    popq %rsi
    popq %rdi
    addq $16, %rsp

    _swapgs_if_user

    iretq

.macro ISR index
.global _isr\index
.type _isr\index, @function
_isr\index:
.if 0x\index != 8 && 0x\index != 10 && 0x\index != 11 && 0x\index != 12 && 0x\index != 13 && 0x\index != 14 && 0x\index != 17 && 0x\index != 30
    pushq $0
.endif
    pushq $0x\index
    jmp isr_handler_stub
.endm

.macro ISRADDR index
    .quad _isr\index
.endm


.irpc i, 0123456789abcdef
.irpc j, 0123456789abcdef
    ISR \i\j
.endr
.endr

.section .data

.global stubs
.align 8
stubs:
.irpc i, 0123456789abcdef
.irpc j, 0123456789abcdef
    ISRADDR \i\j
.endr
.endr