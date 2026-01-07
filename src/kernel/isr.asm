;
; Interrupt Service Routine Stubs
; Assembly stubs for ISRs and IRQs
;

[bits 32]

; External C handlers
extern isr_handler
extern irq_handler

; ISR common stub - saves state and calls C handler
isr_common_stub:
    pusha               ; Push all general purpose registers

    mov ax, ds
    push eax            ; Save data segment

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Push pointer to interrupt frame
    call isr_handler
    add esp, 4          ; Clean up pushed parameter

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore general purpose registers
    add esp, 8          ; Clean up error code and ISR number
    iret                ; Return from interrupt

; IRQ common stub - saves state and calls C handler
irq_common_stub:
    pusha               ; Push all general purpose registers

    mov ax, ds
    push eax            ; Save data segment

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Push pointer to interrupt frame
    call irq_handler
    add esp, 4          ; Clean up pushed parameter

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore general purpose registers
    add esp, 8          ; Clean up error code and IRQ number
    iret                ; Return from interrupt

; ISR stubs - exceptions 0-31
; Some exceptions push an error code, some don't

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0        ; Push dummy error code
    push dword %1       ; Push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push dword %1       ; Push interrupt number (error code already pushed by CPU)
    jmp isr_common_stub
%endmacro

; IRQ stubs - hardware interrupts 0-15 (mapped to INT 32-47)
%macro IRQ 2
global irq%1
irq%1:
    push dword 0        ; Push dummy error code
    push dword %2       ; Push interrupt number
    jmp irq_common_stub
%endmacro

; ISRs 0-31 (CPU exceptions)
ISR_NOERRCODE 0     ; Divide by zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non-maskable interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound range exceeded
ISR_NOERRCODE 6     ; Invalid opcode
ISR_NOERRCODE 7     ; Device not available
ISR_ERRCODE   8     ; Double fault
ISR_NOERRCODE 9     ; Coprocessor segment overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment not present
ISR_ERRCODE   12    ; Stack-segment fault
ISR_ERRCODE   13    ; General protection fault
ISR_ERRCODE   14    ; Page fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 FPU error
ISR_ERRCODE   17    ; Alignment check
ISR_NOERRCODE 18    ; Machine check
ISR_NOERRCODE 19    ; SIMD floating-point
ISR_NOERRCODE 20    ; Virtualization
ISR_ERRCODE   21    ; Control protection
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Hypervisor injection
ISR_NOERRCODE 29    ; VMM communication
ISR_ERRCODE   30    ; Security exception
ISR_NOERRCODE 31    ; Reserved

; IRQs 0-15 (hardware interrupts)
IRQ 0, 32           ; Timer
IRQ 1, 33           ; Keyboard
IRQ 2, 34           ; Cascade
IRQ 3, 35           ; COM2
IRQ 4, 36           ; COM1
IRQ 5, 37           ; LPT2
IRQ 6, 38           ; Floppy
IRQ 7, 39           ; LPT1 / Spurious
IRQ 8, 40           ; RTC
IRQ 9, 41           ; Free
IRQ 10, 42          ; Free
IRQ 11, 43          ; Free
IRQ 12, 44          ; Mouse
IRQ 13, 45          ; FPU
IRQ 14, 46          ; Primary ATA
IRQ 15, 47          ; Secondary ATA
