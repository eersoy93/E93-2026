/**
 * System Call Interface Header
 * Defines system call numbers and functions
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdint.h"

/* System call numbers */
#define SYS_EXIT        0   /* Exit program */
#define SYS_WRITE       1   /* Write to screen */
#define SYS_READ        2   /* Read from keyboard */
#define SYS_OPEN        3   /* Open file (reserved) */
#define SYS_CLOSE       4   /* Close file (reserved) */
#define SYS_SLEEP       5   /* Sleep for milliseconds */
#define SYS_BEEP        6   /* PC speaker beep */
#define SYS_GETCHAR     7   /* Get single character */
#define SYS_EXEC        8   /* Execute program */
#define SYS_READDIR     9   /* Read directory entry */
#define SYS_CLEAR       10  /* Clear screen */
#define SYS_SETCOLOR    11  /* Set text color */
#define SYS_CHDIR       12  /* Change directory (reserved) */
#define SYS_GETCWD      13  /* Get current directory (reserved) */
#define SYS_VGA_INIT    14  /* Enter VGA graphics mode 12h (640x480x16) */
#define SYS_VGA_EXIT    15  /* Exit VGA graphics mode */
#define SYS_VGA_CLEAR   16  /* Clear graphics screen */
#define SYS_VGA_PIXEL   17  /* Set pixel */
#define SYS_VGA_LINE    18  /* Draw line */
#define SYS_VGA_RECT    19  /* Draw rectangle */
#define SYS_VGA_CIRCLE  20  /* Draw circle */
#define SYS_VGA_INIT_13H  21  /* Enter VGA graphics mode 13h (320x200x256) */
#define SYS_VGA_INIT_X    22  /* Enter VGA graphics mode X (320x240x256) */
#define SYS_VGA_PALETTE   23  /* Set palette entry (r, g, b packed) */

/* System call interrupt number */
#define SYSCALL_INT     0x80

/* Maximum number of system calls */
#define NUM_SYSCALLS    28

/**
 * Initialize the system call interface
 */
void syscall_init(void);

/**
 * System call handler (called from assembly)
 * @param eax: System call number
 * @param ebx: First argument
 * @param ecx: Second argument
 * @param edx: Third argument
 * @return Return value from system call
 */
int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif /* SYSCALL_H */
