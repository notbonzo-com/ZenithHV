.section .text

.globl handlerTable
.extern handlerTable

commonISRHanlder:
    cmpq $24, 24(%rsp)
    je ready
    swapgs

ready:
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

    movq %rsp, %rdi
    ; callq default_exception_handler
    movq 168(%rsp), %rbx        # read n from the stack
    shlq $3, %rbx               # offset = n * 8
    leaq handlerTable(%rip), %rax   # load the pointers base address
    addq %rbx, %rax             # base + offset
    callq *(%rax)               # go to where rax value points and call this address

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

    cmpq $24, 24(%rsp)
    je ready2
    swapgs

ready2:
    sti

    iretq


.macro ISR index
    .global _isr\index                      # Declare ISR globally
    .type _isr\index, @function             # Specify the type of symbol as a function
_isr\index:
    .if 0x\index != 8 && 0x\index != 10 && 0x\index != 11 && 0x\index != 12 && 0x\index != 13 && 0x\index != 14 && 0x\index != 17 && 0x\index != 30
        pushq $0                            # Push 0 onto the stack if index doesn't match certain values
    .endif
    pushq $0x\index                         # Push the index onto the stack
    cli                                     # Disable interrupts
    jmp commonISRHanlder
.endm

.macro ISRADDR index
    .quad _isr\index                        # Define a quadword containing the address of the ISR
.endm

.irpc i, 0123456789abcdef
    .irpc j, 0123456789abcdef
        ISR \i\j                            # Invoke the ISR macro with the current combination of digits
    .endr
.endr

.section .data

.global commonHandlerJMPTable               # Declare commonHandlerJMPTable as a global symbol
.align 8                                    # Align the following data on an 8-byte boundary
commonHandlerJMPTable:
.irpc i, 0123456789abcdef
    .irpc j, 0123456789abcdef
        ISRADDR \i\j
    .endr
.endr
