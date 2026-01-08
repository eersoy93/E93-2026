/**
 * Program Loader Implementation
 * ELF32 program loader for userspace applications
 */

#include <loader.h>
#include <vga.h>
#include <string.h>

/* Current program state */
static program_t current_program;
static int program_running = 0;
static int program_exit_code = 0;

/* Parent program path for returning after child exits */
static char parent_path[64];
static int has_parent = 0;

/* Temporary buffer for reading ELF file */
static uint8_t elf_buffer[PROGRAM_MAX_SIZE];

/* External symbol from linker script */
extern uint32_t __kernel_end;

/**
 * Initialize the program loader
 */
void loader_init(void) {
    program_running = 0;
    program_exit_code = 0;
    has_parent = 0;
    parent_path[0] = '\0';
    memset(&current_program, 0, sizeof(program_t));
}

/**
 * Validate ELF32 header
 * @return 0 on success, negative error code on failure
 */
static int elf_validate(elf32_ehdr_t *ehdr) {
    /* Check ELF magic number */
    if (*(uint32_t *)ehdr->e_ident != ELF_MAGIC) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Invalid ELF magic number!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -1;
    }
    
    /* Check ELF class (must be 32-bit) */
    if (ehdr->e_ident[4] != ELFCLASS32) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Not a 32-bit ELF file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -2;
    }
    
    /* Check data encoding (must be little endian) */
    if (ehdr->e_ident[5] != ELFDATA2LSB) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Not a little-endian ELF file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -3;
    }
    
    /* Check file type (must be executable) */
    if (ehdr->e_type != ET_EXEC) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Not an executable ELF file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -4;
    }
    
    /* Check machine type (must be i386) */
    if (ehdr->e_machine != EM_386) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Not an i386 ELF file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -5;
    }
    
    /* Check for program headers */
    if (ehdr->e_phnum == 0) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: ELF file has no program headers!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -6;
    }
    
    return 0;
}

/**
 * Load ELF segments into memory
 * @return 0 on success, negative error code on failure
 */
static int elf_load_segments(uint8_t *elf_data, elf32_ehdr_t *ehdr) {
    elf32_phdr_t *phdr = (elf32_phdr_t *)(elf_data + ehdr->e_phoff);
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            /* Get segment info */
            uint32_t vaddr = phdr[i].p_vaddr;
            uint32_t filesz = phdr[i].p_filesz;
            uint32_t memsz = phdr[i].p_memsz;
            uint32_t offset = phdr[i].p_offset;
            
            /* Copy segment data from file to memory */
            if (filesz > 0) {
                memcpy((void *)vaddr, elf_data + offset, filesz);
            }
            
            /* Zero out the BSS section (memsz > filesz) */
            if (memsz > filesz) {
                memset((void *)(vaddr + filesz), 0, memsz - filesz);
            }
        }
    }
    
    return 0;
}

/**
 * Load a program from a memory buffer (ELF format)
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
    
    if (size < sizeof(elf32_ehdr_t)) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: File too small for ELF header!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -3;
    }
    
    /* Copy to temporary buffer for parsing */
    memcpy(elf_buffer, data, size);
    
    /* Validate ELF header */
    elf32_ehdr_t *ehdr = (elf32_ehdr_t *)elf_buffer;
    int ret = elf_validate(ehdr);
    if (ret < 0) {
        return ret;
    }
    
    /* Load ELF segments */
    ret = elf_load_segments(elf_buffer, ehdr);
    if (ret < 0) {
        return ret;
    }
    
    /* Fill program structure */
    prog->entry = ehdr->e_entry;
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
 * Load a program from filesystem (ELF format)
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
        vga_print("\n");
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
    
    if (node->length < sizeof(elf32_ehdr_t)) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: File too small for ELF header!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -3;
    }
    
    /* Read entire ELF file into buffer */
    int bytes_read = fs_read(node, 0, node->length, elf_buffer);
    
    if (bytes_read < 0) {
        vga_set_color(VGA_COLOR_ERROR, VGA_COLOR_BLACK);
        vga_print("Error: Failed to read file!\n");
        vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
        return -4;
    }
    
    /* Validate ELF header */
    elf32_ehdr_t *ehdr = (elf32_ehdr_t *)elf_buffer;
    int ret = elf_validate(ehdr);
    if (ret < 0) {
        return ret;
    }
    
    /* Load ELF segments */
    ret = elf_load_segments(elf_buffer, ehdr);
    if (ret < 0) {
        return ret;
    }
    
    /* Fill program structure */
    prog->entry = ehdr->e_entry;
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
    vga_print("Executing ELF: ");
    vga_print(prog->name);
    vga_print(" (");
    vga_print_dec(prog->size);
    vga_print(" bytes, entry=0x");
    vga_print_hex(prog->entry);
    vga_print(")\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
    
    /* Call the program entry point */
    typedef void (*program_entry_t)(void);
    program_entry_t entry = (program_entry_t)prog->entry;
    
    /* Jump to program */
    entry();
    
    /* If we get here, program returned without calling exit */
    program_running = 0;
    
    vga_set_color(VGA_COLOR_INFO, VGA_COLOR_BLACK);
    vga_print("Program returned without exit\n");
    vga_set_color(VGA_COLOR_NORMAL, VGA_COLOR_BLACK);
    
    return 0;
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
 * Signal program exit and restart parent if exists
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
    
    /* If there's a parent program, reload and restart it */
    if (has_parent && parent_path[0] != '\0') {
        program_t parent;
        has_parent = 0;  /* Clear before reloading to avoid infinite loop */
        
        if (loader_load(parent_path, &parent) == 0) {
            parent_path[0] = '\0';
            loader_exec(&parent);
            /* If exec returns, fall through to halt */
        }
    }
    
    /* Fallback: halt the machine */
    vga_print("System halted!\n");
    __asm__ volatile ("cli");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/**
 * Set the parent program path (called before exec'ing a child)
 */
void loader_set_parent(const char *path) {
    if (path) {
        strncpy(parent_path, path, sizeof(parent_path) - 1);
        parent_path[sizeof(parent_path) - 1] = '\0';
        has_parent = 1;
    }
}
