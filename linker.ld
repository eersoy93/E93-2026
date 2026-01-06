/**
 * Linker Script for 32-bit OS Kernel
 * Defines memory layout for the kernel image
 */

/* Entry point */
ENTRY(_start)

/* Memory sections */
SECTIONS
{
    /* Load kernel at 1MB mark (standard for protected mode) */
    . = 1M;

    /* Multiboot header must be within first 8KB of file */
    .multiboot :
    {
        *(.multiboot)
    }

    /* Text section (code) */
    .text ALIGN(4) :
    {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* Kernel end marker */
    __kernel_end = .;

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
    }
}
