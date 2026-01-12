/**
 * Hello World - C Version
 * A simple userspace program written in C
 * 
 * Compiled as ELF32 executable by the build system.
 * Entry point: _start at virtual address 0x400000
 */

#include <user.h>
#include <io.h>

/* Program entry point */
void _start(void) {
    /* Print Hello message */
    println("Hello from userspace!");

    /* Play a success beep */
    beep(1000, 100);

    /* Wait for user input before exiting */
    println("Press any key to continue...");
    getchar();
    println("Exiting now...");

    /* Exit cleanly */
    exit(0);
}
