/**
 * IDT (Interrupt Descriptor Table) Header
 * Defines structures and functions for interrupt handling
 */

#ifndef IDT_H
#define IDT_H

#include "stdint.h"

/* Number of IDT entries (256 possible interrupts) */
#define IDT_ENTRIES 256

/* IDT gate types */
#define IDT_GATE_TASK       0x05
#define IDT_GATE_INT16      0x06
#define IDT_GATE_TRAP16     0x07
#define IDT_GATE_INT32      0x0E
#define IDT_GATE_TRAP32     0x0F

/* IDT flags */
#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_RING1      0x20
#define IDT_FLAG_RING2      0x40
#define IDT_FLAG_RING3      0x60

/* PIC (Programmable Interrupt Controller) ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* PIC commands */
#define PIC_EOI         0x20    /* End of interrupt */

/* IRQ numbers */
#define IRQ0    32  /* Timer */
#define IRQ1    33  /* Keyboard */
#define IRQ2    34  /* Cascade */
#define IRQ3    35  /* COM2 */
#define IRQ4    36  /* COM1 */
#define IRQ5    37  /* LPT2 */
#define IRQ6    38  /* Floppy */
#define IRQ7    39  /* LPT1 */
#define IRQ8    40  /* RTC */
#define IRQ9    41  /* Free */
#define IRQ10   42  /* Free */
#define IRQ11   43  /* Free */
#define IRQ12   44  /* Mouse */
#define IRQ13   45  /* FPU */
#define IRQ14   46  /* Primary ATA */
#define IRQ15   47  /* Secondary ATA */

/* IDT entry structure (8 bytes) */
typedef struct {
    uint16_t base_low;      /* Lower 16 bits of handler address */
    uint16_t selector;      /* Kernel code segment selector */
    uint8_t  zero;          /* Always zero */
    uint8_t  flags;         /* Type and attributes */
    uint16_t base_high;     /* Upper 16 bits of handler address */
} __attribute__((packed)) idt_entry_t;

/* IDT pointer structure */
typedef struct {
    uint16_t limit;         /* Size of IDT - 1 */
    uint32_t base;          /* Base address of IDT */
} __attribute__((packed)) idt_ptr_t;

/* Interrupt frame pushed by CPU */
typedef struct {
    uint32_t ds;                                        /* Data segment */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;   /* Pushed by pusha */
    uint32_t int_no, err_code;                          /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;             /* Pushed by CPU */
} __attribute__((packed)) interrupt_frame_t;

/* IRQ handler function type */
typedef void (*irq_handler_t)(interrupt_frame_t *frame);

/* Function declarations */
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void irq_install_handler(uint8_t irq, irq_handler_t handler);
void irq_uninstall_handler(uint8_t irq);

/* PIC functions */
void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

/* External ISR stubs (defined in assembly) */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* External IRQ stubs */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/* Syscall interrupt stub */
extern void isr128(void);

#endif /* IDT_H */
