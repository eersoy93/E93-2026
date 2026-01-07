/**
 * User Linker Script
 * Links userspace programs as flat binaries
 */

OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    /* Virtual address where program is loaded */
    . = 0x400000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
    }
}
