/**
 * Hello World - C Version
 * A simple userspace program written in C
 * 
 * Compile with:
 *   i686-elf-gcc -m32 -ffreestanding -nostdlib -nostdinc -fno-pic -c hello.c -o hello.o
 *   i686-elf-ld -m elf_i386 -Ttext=0x400000 --oformat binary -o hello.bin hello.o
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
