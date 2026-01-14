/**
 * Program Template
 * 
 * Copy this file to src/user/programs/ and rename it.
 * Then modify it to create your own program.
 * 
 * Build with: make iso
 * Run with:   make run
 * 
 * Entry point: _start at virtual address 0x400000
 */

#include <syscall.h>
#include <io.h>
#include <version.h>

/* Uncomment if using graphics */
/* #include <vga_gfx.h> */

/**
 * Program entry point
 * 
 * This function is called when the program starts.
 * It must call exit() when done to properly terminate.
 */
void _start(void) {
    /* Clear screen and set colors (optional) */
    clear();
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("=== My Program ===\n\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    
    /* Your code here */
    print("Hello from my program!\n");
    print("OS Version: ");
    print(VERSION);
    print("\n");
    
    /* Example: read user input */
    char input[128];
    print("\nEnter something: ");
    int len = readline(input, sizeof(input));
    
    if (len >= 0) {
        print("You entered: ");
        print(input);
        print("\n");
    } else {
        print("\nInput cancelled.\n");
    }
    
    /* Example: play a sound */
    beep(440, 200);
    
    /* Wait before exit (optional) */
    print("\nPress any key to exit...");
    getchar();
    
    /* Always exit with a status code */
    exit(0);
}
