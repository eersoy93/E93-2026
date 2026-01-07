; Multiboot header constants
MBALIGN     equ 1 << 0                  ; Align loaded modules on page boundaries
MEMINFO     equ 1 << 1                  ; Provide memory map
FLAGS       equ MBALIGN | MEMINFO       ; Multiboot flag field
MAGIC       equ 0x1BADB002              ; Magic number for bootloader
CHECKSUM    equ -(MAGIC + FLAGS)        ; Checksum (magic + flags + checksum = 0)

; Multiboot header section
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack section - reserve 16KB for stack
section .bss
align 16
stack_bottom:
    resb 16384                          ; 16 KB stack
stack_top:

; Text section - entry point
section .text
global _start
extern kernel_main

_start:
    ; Save multiboot parameters before we clobber EAX
    mov ecx, eax                        ; Save magic number in ECX
    mov edx, ebx                        ; Save multiboot info in EDX

    ; Set up stack pointer
    mov esp, stack_top

    ; Set up our own GDT
    lgdt [gdt_descriptor]

    ; Reload segment registers (this clobbers EAX)
    mov ax, 0x10                        ; Data segment selector (GDT entry 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.reload_cs                 ; Far jump to reload CS (code segment)

.reload_cs:
    ; Push multiboot info pointer and magic number (from saved registers)
    push edx                            ; Multiboot info structure pointer
    push ecx                            ; Multiboot magic number

    ; Call the kernel main function
    call kernel_main

    ; If kernel returns, halt the CPU
    cli
.hang:
    hlt
    jmp .hang

; ============================================
; Global Descriptor Table (GDT)
; ============================================
section .data
align 16
gdt_start:
    ; Null descriptor (required)
    dq 0x0000000000000000

    ; Code segment descriptor (selector 0x08)
    ; Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0xCF
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access: Present, Ring 0, Code, Executable, Readable
    db 11001111b    ; Flags: 4KB granularity, 32-bit + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

    ; Data segment descriptor (selector 0x10)
    ; Base=0, Limit=0xFFFFF, Access=0x92, Flags=0xCF
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access: Present, Ring 0, Data, Writable
    db 11001111b    ; Flags: 4KB granularity, 32-bit + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size - 1
    dd gdt_start                ; GDT base address

section .text

; Global Descriptor Table (GDT) setup
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]                  ; Get GDT pointer from stack
    lgdt [eax]                          ; Load GDT
    mov ax, 0x10                        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush                     ; Far jump to code segment
.flush:
    ret

; Interrupt Descriptor Table (IDT) setup
global idt_flush
idt_flush:
    mov eax, [esp + 4]                  ; Get IDT pointer from stack
    lidt [eax]                          ; Load IDT
    ret
