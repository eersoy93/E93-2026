/**
 * Program Loader Implementation
 * Simple flat binary program loader
 */

#include "loader.h"
#include "vga.h"
#include "string.h"

/* Current program state */
static program_t current_program;
static int program_running = 0;
static int program_exit_code = 0;

/* External symbol from linker script */
extern uint32_t __kernel_end;

/**
 * Initialize the program loader
 */
void loader_init(void) {
    program_running = 0;
    program_exit_code = 0;
    memset(&current_program, 0, sizeof(program_t));
}

/**
 * Load a program from a memory buffer
 */
int loader_load_from_memory(const uint8_t *data, uint32_t size, 
                            const char *name, program_t *prog) {
    if (!data || !prog) {
        return -1;
    }
    
    if (size > PROGRAM_MAX_SIZE) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Program too large!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -2;
    }
    
    /* Copy program to load address */
    uint8_t *dest = (uint8_t *)PROGRAM_LOAD_ADDR;
    memcpy(dest, data, size);
    
    /* Fill program structure */
    prog->entry = PROGRAM_LOAD_ADDR + PROGRAM_ENTRY_OFFSET;
    prog->size = size;
    prog->load_addr = PROGRAM_LOAD_ADDR;
    
    if (name) {
        strncpy(prog->name, name, sizeof(prog->name) - 1);
        prog->name[sizeof(prog->name) - 1] = '\0';
    } else {
        strcpy(prog->name, "unknown");
    }
    
    return 0;
}

/**
 * Load a program from filesystem
 */
int loader_load(const char *path, program_t *prog) {
    if (!path || !prog) {
        return -1;
    }
    
    /* Find the file */
    fs_node_t *node = fs_namei(path);
    if (!node) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: File not found: ");
        vga_print(path);
        vga_print("!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -1;
    }
    
    /* Check file size */
    if (node->length > PROGRAM_MAX_SIZE) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Program too large!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -2;
    }
    
    /* Read file directly to load address */
    uint8_t *dest = (uint8_t *)PROGRAM_LOAD_ADDR;
    int bytes_read = fs_read(node, 0, node->length, dest);
    
    if (bytes_read < 0) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Failed to read file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -3;
    }
    
    /* Fill program structure */
    prog->entry = PROGRAM_LOAD_ADDR + PROGRAM_ENTRY_OFFSET;
    prog->size = (uint32_t)bytes_read;
    prog->load_addr = PROGRAM_LOAD_ADDR;
    strncpy(prog->name, path, sizeof(prog->name) - 1);
    prog->name[sizeof(prog->name) - 1] = '\0';
    
    return 0;
}

/**
 * Execute a loaded program
 */
int loader_exec(program_t *prog) {
    if (!prog) {
        return -1;
    }
    
    /* Store current program info */
    memcpy(&current_program, prog, sizeof(program_t));
    program_running = 1;
    program_exit_code = 0;
    
    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("Executing: ");
    vga_print(prog->name);
    vga_print(" (");
    vga_print_dec(prog->size);
    vga_print(" bytes...)\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
    
    /* Call the program entry point */
    /* The program is a flat binary that uses INT 0x80 for syscalls */
    typedef void (*program_entry_t)(void);
    program_entry_t entry = (program_entry_t)prog->entry;
    
    /* Jump to program */
    entry();
    
    /* If we get here, program returned without calling exit */
    program_running = 0;
    
    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("Program exited with code: ");
    vga_print_dec((uint32_t)program_exit_code);
    vga_print("\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
    
    return program_exit_code;
}

/**
 * Check if a program is running
 */
int loader_is_running(void) {
    return program_running;
}

/**
 * Get current program
 */
program_t *loader_get_current(void) {
    if (program_running) {
        return &current_program;
    }
    return NULL;
}

/**
 * Signal program exit and halt the machine
 */
void loader_exit(int exit_code) {
    program_exit_code = exit_code;
    program_running = 0;
    
    /* Print exit message */
    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("Program exited with code: ");
    vga_print_dec((uint32_t)exit_code);
    vga_print("\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
    
    /* Halt the machine */
    vga_print("System halted!\n");
    __asm__ volatile ("cli");  /* Disable interrupts */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
