/**
 * Program Loader Header
 * ELF32 program loader for userspace applications
 */

#ifndef LOADER_H
#define LOADER_H

#include "fs.h"
#include "stdint.h"

/* Program load address (4MB mark, above kernel) */
#define PROGRAM_LOAD_ADDR   0x400000

/* Maximum program size (64KB) */
#define PROGRAM_MAX_SIZE    0x10000

/* ELF Magic Number */
#define ELF_MAGIC 0x464C457F  /* "\x7FELF" in little endian */

/* ELF Class */
#define ELFCLASS32      1     /* 32-bit objects */

/* ELF Data Encoding */
#define ELFDATA2LSB     1     /* Little endian */

/* ELF Type */
#define ET_EXEC         2     /* Executable file */

/* ELF Machine */
#define EM_386          3     /* Intel 80386 */

/* Program Header Types */
#define PT_NULL         0     /* Unused entry */
#define PT_LOAD         1     /* Loadable segment */

/* Program Header Flags */
#define PF_X            0x1   /* Execute */
#define PF_W            0x2   /* Write */
#define PF_R            0x4   /* Read */

/**
 * ELF32 Header
 */
typedef struct {
    uint8_t  e_ident[16];     /* ELF identification */
    uint16_t e_type;          /* Object file type */
    uint16_t e_machine;       /* Machine type */
    uint32_t e_version;       /* Object file version */
    uint32_t e_entry;         /* Entry point address */
    uint32_t e_phoff;         /* Program header offset */
    uint32_t e_shoff;         /* Section header offset */
    uint32_t e_flags;         /* Processor-specific flags */
    uint16_t e_ehsize;        /* ELF header size */
    uint16_t e_phentsize;     /* Size of program header entry */
    uint16_t e_phnum;         /* Number of program header entries */
    uint16_t e_shentsize;     /* Size of section header entry */
    uint16_t e_shnum;         /* Number of section header entries */
    uint16_t e_shstrndx;      /* Section name string table index */
} __attribute__((packed)) elf32_ehdr_t;

/**
 * ELF32 Program Header
 */
typedef struct {
    uint32_t p_type;          /* Type of segment */
    uint32_t p_offset;        /* Offset in file */
    uint32_t p_vaddr;         /* Virtual address in memory */
    uint32_t p_paddr;         /* Physical address (unused) */
    uint32_t p_filesz;        /* Size of segment in file */
    uint32_t p_memsz;         /* Size of segment in memory */
    uint32_t p_flags;         /* Segment attributes */
    uint32_t p_align;         /* Alignment of segment */
} __attribute__((packed)) elf32_phdr_t;

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
