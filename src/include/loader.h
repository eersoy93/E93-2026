/**
 * Program Loader Header
 * Simple flat binary program loader
 */

#ifndef LOADER_H
#define LOADER_H

#include "fs.h"
#include "stdint.h"

/* Program load address (4MB mark, above kernel) */
#define PROGRAM_LOAD_ADDR   0x400000

/* Maximum program size (64KB) */
#define PROGRAM_MAX_SIZE    0x10000

/* Program entry point offset in flat binary */
#define PROGRAM_ENTRY_OFFSET 0

/**
 * Program structure
 */
typedef struct {
    uint32_t entry;         /* Entry point address */
    uint32_t size;          /* Program size */
    uint32_t load_addr;     /* Load address */
    char name[64];          /* Program name */
} program_t;

/**
 * Initialize the program loader
 */
void loader_init(void);

/**
 * Load a flat binary program from filesystem
 * @param path: Path to the program file
 * @param prog: Program structure to fill
 * @return 0 on success, negative error code on failure
 */
int loader_load(const char *path, program_t *prog);

/**
 * Load a program from a memory buffer
 * @param data: Program binary data
 * @param size: Size of program data
 * @param name: Program name
 * @param prog: Program structure to fill
 * @return 0 on success, negative error code on failure
 */
int loader_load_from_memory(const uint8_t *data, uint32_t size, 
                            const char *name, program_t *prog);

/**
 * Execute a loaded program
 * @param prog: Program to execute
 * @return Program exit code
 */
int loader_exec(program_t *prog);

/**
 * Check if a program is currently running
 * @return 1 if program is running, 0 otherwise
 */
int loader_is_running(void);

/**
 * Get the currently running program
 * @return Pointer to current program, or NULL if none
 */
program_t *loader_get_current(void);

/**
 * Signal that the current program has exited
 * @param exit_code: Exit code from program
 */
void loader_exit(int exit_code);

#endif /* LOADER_H */
