/**
 * System Call Implementation
 * Provides system call interface for userspace programs
 */

#include "idt.h"
#include "loader.h"
#include "pit.h"
#include "speaker.h"
#include "string.h"
#include "syscall.h"
#include "vga.h"

/* System call function type */
typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t);

/* Forward declarations */
static int sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2);
static int sys_write(uint32_t fd, uint32_t buf, uint32_t len);
static int sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2);
static int sys_beep(uint32_t freq, uint32_t duration, uint32_t unused);

/* System call table */
static syscall_fn syscall_table[NUM_SYSCALLS] = {
    [SYS_EXIT]  = sys_exit,
    [SYS_WRITE] = sys_write,
    [SYS_READ]  = NULL,     /* Reserved */
    [SYS_OPEN]  = NULL,     /* Reserved */
    [SYS_CLOSE] = NULL,     /* Reserved */
    [SYS_SLEEP] = sys_sleep,
    [SYS_BEEP]  = sys_beep,
};

/**
 * SYS_EXIT - Exit the current program
 */
static int sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    loader_exit((int)code);
    return 0;  /* Never reached */
}

/**
 * SYS_WRITE - Write to output
 * fd 1 = stdout (VGA screen)
 */
static int sys_write(uint32_t fd, uint32_t buf, uint32_t len) {
    if (fd != 1) {
        return -1;  /* Only stdout supported */
    }
    
    const char *str = (const char *)buf;
    
    /* Write each character */
    for (uint32_t i = 0; i < len; i++) {
        if (str[i] == '\0') {
            break;
        }
        vga_putchar(str[i]);
    }
    
    return (int)len;
}

/**
 * SYS_SLEEP - Sleep for milliseconds
 */
static int sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    pit_sleep(ms);
    return 0;
}

/**
 * SYS_BEEP - Play a beep
 */
static int sys_beep(uint32_t freq, uint32_t duration, uint32_t unused) {
    (void)unused;
    
    speaker_beep((uint16_t)freq, (uint32_t)duration);
    return 0;
}

/**
 * Main system call handler
 * Called from the INT 0x80 handler
 */
int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    /* Validate system call number */
    if (eax >= NUM_SYSCALLS || syscall_table[eax] == NULL) {
        vga_set_color(VGA_COLOR_WARNING, VGA_COLOR_BLACK);
        vga_print("Unknown syscall: ");
        vga_print_dec(eax);
        vga_print("\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -1;
    }
    
    /* Call the system call function */
    return syscall_table[eax](ebx, ecx, edx);
}

/**
 * Syscall interrupt handler
 * Called from assembly stub
 */
void syscall_isr_handler(interrupt_frame_t *frame) {
    /* Get arguments from registers */
    uint32_t eax = frame->eax;  /* Syscall number */
    uint32_t ebx = frame->ebx;  /* Arg 1 */
    uint32_t ecx = frame->ecx;  /* Arg 2 */
    uint32_t edx = frame->edx;  /* Arg 3 */
    
    /* Call the handler and store result in eax */
    frame->eax = (uint32_t)syscall_handler(eax, ebx, ecx, edx);
}

/**
 * Initialize the system call interface
 */
void syscall_init(void) {
    /* Register INT 0x80 as the syscall interrupt */
    /* Use ring 3 accessible gate so userspace can call it */
    extern void isr128(void);  /* Defined in isr.asm */
    idt_set_gate(SYSCALL_INT, (uint32_t)isr128, 0x08, 
                 IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_GATE_INT32);
}
