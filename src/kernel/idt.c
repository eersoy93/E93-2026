/**
 * IDT (Interrupt Descriptor Table) Implementation
 * Sets up interrupt handling for the kernel
 */

#include "idt.h"
#include "ports.h"
#include "vga.h"

/* IDT and IDT pointer */
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

/* IRQ handlers array */
static irq_handler_t irq_handlers[16] = { 0 };

/* Exception messages */
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

/**
 * Load IDT using lidt instruction
 */
static inline void idt_load(void) {
    __asm__ volatile ("lidt (%0)" : : "r"(&idt_ptr));
}

/**
 * Set an IDT gate entry
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

/**
 * Initialize the PIC (Programmable Interrupt Controller)
 * Remaps IRQs 0-15 to interrupts 32-47
 */
void pic_init(void) {
    /* Start initialization sequence (ICW1) */
    outb(PIC1_COMMAND, 0x11);  /* Init + ICW4 needed */
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    /* Set vector offsets (ICW2) */
    outb(PIC1_DATA, 0x20);  /* IRQ 0-7  -> INT 32-39 */
    io_wait();
    outb(PIC2_DATA, 0x28);  /* IRQ 8-15 -> INT 40-47 */
    io_wait();

    /* Set up cascading (ICW3) */
    outb(PIC1_DATA, 0x04);  /* IRQ2 has slave */
    io_wait();
    outb(PIC2_DATA, 0x02);  /* Slave ID 2 */
    io_wait();

    /* Set 8086 mode (ICW4) */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Mask all interrupts */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

/**
 * Send End of Interrupt to PIC
 */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Set IRQ mask (disable an IRQ)
 */
void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

/**
 * Clear IRQ mask (enable an IRQ)
 */
void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

/**
 * Install an IRQ handler
 */
void irq_install_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
        pic_clear_mask(irq);  /* Enable this IRQ */
    }
}

/**
 * Uninstall an IRQ handler
 */
void irq_uninstall_handler(uint8_t irq) {
    if (irq < 16) {
        irq_handlers[irq] = 0;
        pic_set_mask(irq);  /* Disable this IRQ */
    }
}

/**
 * ISR handler - called from assembly stub
 */
void isr_handler(interrupt_frame_t *frame) {
    if (frame->int_no < 32) {
        /* CPU exception */
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_print("\n*** KERNEL PANIC ***\n");
        vga_print("Exception: ");
        vga_print(exception_messages[frame->int_no]);
        vga_print("\n");

        vga_print("Error Code: 0x");
        vga_print_hex(frame->err_code);
        vga_print("\n");

        vga_print("EIP: 0x");
        vga_print_hex(frame->eip);
        vga_print("  CS: 0x");
        vga_print_hex(frame->cs);
        vga_print("\n");

        vga_print("EFLAGS: 0x");
        vga_print_hex(frame->eflags);
        vga_print("\n");

        /* Halt the system */
        __asm__ volatile ("cli; hlt");
    }
}

/**
 * IRQ handler - called from assembly stub
 */
void irq_handler(interrupt_frame_t *frame) {
    uint8_t irq = frame->int_no - 32;

    /* Call the registered handler if any */
    if (irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq](frame);
    }

    /* Send End of Interrupt */
    pic_send_eoi(irq);
}

/**
 * Initialize the IDT
 */
void idt_init(void) {
    /* Set up IDT pointer */
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    /* Clear IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Initialize PIC (also masks all IRQs) */
    pic_init();

    /* Set up ISR gates (exceptions 0-31) */
    idt_set_gate(0,  (uint32_t)isr0,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);

    /* Set up IRQ gates (interrupts 32-47) */
    idt_set_gate(32, (uint32_t)irq0,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(33, (uint32_t)irq1,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(34, (uint32_t)irq2,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(35, (uint32_t)irq3,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(36, (uint32_t)irq4,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(37, (uint32_t)irq5,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(38, (uint32_t)irq6,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(39, (uint32_t)irq7,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(40, (uint32_t)irq8,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(41, (uint32_t)irq9,  0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_FLAG_PRESENT | IDT_GATE_INT32);

    /* Load the IDT */
    idt_load();
}
