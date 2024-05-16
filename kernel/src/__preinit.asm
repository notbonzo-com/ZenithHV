section .text

; int printf_(const char* format, ...);
extern printf_
; extern kprintf
extern kpanic
extern kmain

extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors

; extern __attribute__((noreturn)) void __preinit(void);
global __preinit

__preinit:
    ; Save registers
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rbp
    push r8
    push r9
    push r10
    push r11

    ; Print debug message: Disabling interrupts
    lea rdi, [msgDisableInterrupts]
    call printf_
    cli

    ; Print debug message: Calling constructors
    lea rdi, [msgCallingCtors]
    call printf_
    ; Call constructors
.callCtors:
    lea rbx, [rel start_ctors]
    lea rdx, [rel end_ctors]
.loopCtors:
    cmp rbx, rdx       ; Compare the current constructor address with end_ctors
    je .callKmain      ; If all constructors are called, jump to call kmain
    mov rax, [rbx]     ; Load the function pointer of the current constructor
    call rax           ; Call the constructor
    add rbx, 8         ; Move to the next function pointer
    jmp .loopCtors     ; Repeat the loop

.callKmain:
    ; Restore registers
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi

    ; Call kernel main
    call kmain

.kRet:
    cmp rax, 0
    jne .kFail

.kSuccess:
    lea rdi, [msgSuccess]     ; First argument is a pointer to a string
    call printf_
    cli
    jmp .nmiSafe

.kFail:
    lea rdi, [msgFail]        ; First argument is a pointer to a string
    mov rsi, rax              ; Variable argument, just an integer of return code
    call printf_
    xor rdi, rdi
    call kpanic
    cli

.nmiSafe:
    hlt
    jmp .nmiSafe

section .rodata
msgFail: db '<<src/__preinit.asm>> Line 79 in __preinit -> !!! Kernel Main has returned non-zero code: %d !!!', 10, 0
msgSuccess: db '<<src/__preinit.asm>> Line 72 in __preinit -> -> Kernel Main finished correctly!', 10, 0
msgDisableInterrupts: db '<<src/__preinit.asm>> Line 32 in __preinit -> -> Disabling interrupts', 10, 0
msgCallingCtors: db '<<src/__preinit.asm>> Line 37 in __preinit -> -> Calling constructors', 10, 0


; Lets also put the memory functions here cuz why not
section .text

section .text
global memcpy
memcpy:
    push rbp
    mov rbp, rsp

    mov r8, rdi
    mov r9, rsi

.copyLoop:
    test rcx, rcx
    je .done
    mov al, [r9]
    mov [r8], al
    inc r8
    inc r9
    dec rcx
    jmp .copyLoop

.done:
    mov rax, rdi
    mov rsp, rbp
    pop rbp
    ret

global memset
memset:
    push rbp
    mov rbp, rsp
    mov rcx, rdx

    mov r8, rdi

.setLoop:
    test rcx, rcx
    je .done
    mov [r8], sil
    inc r8
    dec rcx
    jmp .setLoop

.done:
    mov rax, rdi
    mov rsp, rbp
    pop rbp
    ret

global memmove
memmove:
    push rbp
    mov rbp, rsp
    mov rcx, rdx

    mov r8, rdi
    mov r9, rsi

    cmp rsi, rdi
    ja .copyForward

    add r8, rcx
    add r9, rcx
    dec r8
    dec r9

.copyBackwardLoop:
    test rcx, rcx
    je .done
    mov al, [r9]
    mov [r8], al
    dec r8
    dec r9
    dec rcx
    jmp .copyBackwardLoop

.copyForward:
.copyForwardLoop:
    test rcx, rcx
    je .done
    mov al, [r9]
    mov [r8], al
    inc r8
    inc r9
    dec rcx
    jmp .copyForwardLoop

.done:
    mov rax, rdi
    mov rsp, rbp
    pop rbp
    ret

global memcmp
memcmp:
    push rbp
    mov rbp, rsp
    mov rcx, rdx

    mov r8, rdi
    mov r9, rsi

.compareLoop:
    test rcx, rcx
    je .equal
    mov al, [r8]
    mov bl, [r9]
    cmp al, bl
    jne .notEqual
    inc r8
    inc r9
    dec rcx
    jmp .compareLoop

.equal:
    xor eax, eax
    jmp .done

.notEqual:
    mov al, [r8]
    mov bl, [r9]
    sub eax, ebx

.done:
    mov rsp, rbp
    pop rbp
    ret
