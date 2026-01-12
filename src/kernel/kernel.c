/**
 * 32-bit OS Kernel
 * Main kernel entry point and core functionality
 */

#include <fs.h>
#include <ide.h>
#include <idt.h>
#include <iso9660.h>
#include <kernel.h>
#include <keyboard.h>
#include <loader.h>
#include <pci.h>
#include <pit.h>
#include <speaker.h>
#include <string.h>
#include <syscall.h>
#include <vga.h>

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

    /* Initialize keyboard driver */
    vga_print("Initializing keyboard...\n");
    keyboard_init();

    /* Enable interrupts */
    vga_print("Enabling interrupts...\n");
    __asm__ volatile ("sti");

    /* Initialize system call interface */
    vga_print("Initializing syscall interface...\n");
    syscall_init();

    /* Initialize program loader */
    vga_print("Initializing program loader...\n");
    loader_init();

    /* Initialize PC speaker */
    vga_print("Initializing PC Speaker...\n");
    speaker_init();

    /* Initialize PCI bus */
    vga_print("Initializing PCI bus...\n");
    pci_init();

    /* Print detected PCI devices */
    if (pci_get_device_count() > 0) {
        vga_print("Detected PCI devices: ");
        vga_print_dec(pci_get_device_count());
        vga_print("\n");
    }

    /* Initialize IDE controller */
    vga_print("Initializing IDE controller...\n");
    ide_init();

    /* Print detected drives */
    if (ide_get_drive_count() > 0) {
        vga_print("Detected IDE drives:\n");
        ide_print_info();
    } else {
        vga_print("No IDE drives detected!\n");
    }

    /* Initialize Virtual Filesystem */
    vga_print("Initializing VFS...\n");
    fs_init();

    /* Initialize ISO9660 filesystem driver */
    vga_print("Initializing ISO9660...\n");
    iso9660_init();

    /* Try to mount CD-ROM filesystems from all ATAPI drives */
    vga_print("Mounting CD-ROM filesystems...\n");

    fs_node_t *cdrom_roots[4] = {NULL};
    int mounted_count = 0;
    
    for (int i = 0; i < 4; i++) {
        ide_device_t *dev = ide_get_device(i);
        if (dev && dev->type == IDE_TYPE_ATAPI) {
            cdrom_roots[i] = fs_mount(i, "iso9660");
            if (cdrom_roots[i]) {
                vga_print("Mounted ISO9660 filesystem from drive ");
                vga_putchar('0' + i);
                vga_putchar('.');
                vga_print("\n");
                mounted_count++;
            }
        }
    }
    if (mounted_count == 0) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: No CD-ROM filesystems mounted!\n");
        vga_print("Cannot continue without a filesystem!\n");
        vga_print("System halted!\n");
        asm volatile("hlt");
    }

    /* Run the shell from filesystem */
    vga_print("\n");
    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("Starting shell...\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);

    program_t shell;
    /* Try to load shell from the CD-ROM filesystem */
    if (loader_load("/user/shell", &shell) == 0) {
        loader_exec(&shell);
    } else {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Failed to load: /user/shell\n");
    }

    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("=== System Halted ===\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);

    /* Halt the CPU */
    while (1) {
        __asm__ volatile("hlt");
    }

    UNUSED(mboot_info); /* Suppress unused variable warning */
}
