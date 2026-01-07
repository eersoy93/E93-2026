/**
 * 32-bit OS Kernel
 * Main kernel entry point and core functionality
 */

#include "kernel.h"
#include "string.h"
#include "vga.h"

/* Multiboot magic number */
#define MULTIBOOT_MAGIC 0x2BADB002

/**
 * Kernel main entry point
 * Called from boot.asm after setting up the stack
 */
void kernel_main(unsigned int magic, unsigned int *mboot_info) {
    /* Initialize VGA text mode */
    vga_init();

    /* Clear the screen */
    vga_clear();

    /* Check multiboot magic number */
    if (magic != MULTIBOOT_MAGIC) {
        vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_print("Error: Invalid Multiboot magic number!\n");
        return;
    }

    /* Print welcome message */
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Loading E93-2026...\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Kernel loaded successfully!\n");

    /* Halt the CPU */
    while (1) {
        __asm__ volatile("hlt");
    }

    (void) mboot_info; /* Suppress unused variable warning */
}
