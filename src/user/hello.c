/**
 * Hello World - C Version
 * A simple userspace program written in C
 * 
 * Compiled as ELF32 executable by the build system.
 * Entry point: _start at virtual address 0x400000
 */

#include "user.h"

/* Program entry point */
void _start(void) {
    print("Hello from userspace!\n");

    /* Play a success beep */
    beep(1000, 100);
    
    /* Exit cleanly */
    exit(0);
}
