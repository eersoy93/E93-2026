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
    ; Set up stack pointer
    mov esp, stack_top

    ; Push multiboot info pointer and magic number
    push ebx                            ; Multiboot info structure pointer
    push eax                            ; Multiboot magic number

    ; Call the kernel main function
    call kernel_main

    ; If kernel returns, halt the CPU
    cli
.hang:
    hlt
    jmp .hang

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
