/**
 * 32-bit OS Kernel
 * Main kernel entry point and core functionality
 */

#include "kernel.h"
#include "idt.h"
#include "pit.h"
#include "speaker.h"
#include "string.h"
#include "vga.h"

/* Multiboot magic number */
#define MULTIBOOT_MAGIC 0x2BADB002

/**
 * Kernel main entry point
 * Called from boot.asm after setting up the stack
 */
void kernel_main(unsigned int magic, unsigned int *mboot_info) {
    /* Initialize VGA text mode and clear the screen*/
    vga_init();
    vga_clear();

    /* Check multiboot magic number */
    if (magic != MULTIBOOT_MAGIC) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Invalid Multiboot magic number!\n");
        return;
    }

    /* Initialize IDT (Interrupt Descriptor Table) */
    vga_print("Initializing IDT...\n");
    idt_init();

    /* Initialize PIT timer (1000 Hz = 1ms resolution) */
    vga_print("Initializing PIT...\n");
    pit_init(1000);

    /* Enable interrupts */
    vga_print("Enabling interrupts...\n");
    __asm__ volatile ("sti");

    /* Initialize PC speaker */
    vga_print("Initializing PC Speaker...\n");
    speaker_init();

    /* Print welcome message */
    vga_set_color(VGA_COLOR_SUCCESS, VGA_COLOR_BLACK);
    vga_print("Welcome to E93-2026!\n");

    /* Play startup beep */
    speaker_beep(NOTE_SYSTEM, 100);

    /* Halt the CPU */
    while (1) {
        __asm__ volatile("hlt");
    }

    UNUSED(mboot_info); /* Suppress unused variable warning */
}
