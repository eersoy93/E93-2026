/**
 * System Call Implementation
 * Provides system call interface for userspace programs
 */

#include <fs.h>
#include <idt.h>
#include <keyboard.h>
#include <loader.h>
#include <pit.h>
#include <speaker.h>
#include <string.h>
#include <syscall.h>
#include <vga.h>
#include <vga_gfx.h>

/* System call function type */
typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t);

/* Forward declarations */
static int sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2);
static int sys_write(uint32_t fd, uint32_t buf, uint32_t len);
static int sys_read(uint32_t fd, uint32_t buf, uint32_t len);
static int sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2);
static int sys_beep(uint32_t freq, uint32_t duration, uint32_t unused);
static int sys_getchar(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_exec(uint32_t path, uint32_t unused1, uint32_t unused2);
static int sys_readdir(uint32_t path, uint32_t index, uint32_t buf);
static int sys_clear(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_setcolor(uint32_t fg, uint32_t bg, uint32_t unused);
static int sys_vga_init(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_vga_exit(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_vga_clear(uint32_t color, uint32_t unused1, uint32_t unused2);
static int sys_vga_pixel(uint32_t x, uint32_t y, uint32_t color);
static int sys_vga_line(uint32_t packed1, uint32_t packed2, uint32_t color);
static int sys_vga_rect(uint32_t packed_xy, uint32_t packed_wh, uint32_t color_fill);
static int sys_vga_circle(uint32_t packed_xy, uint32_t r, uint32_t color_fill);
static int sys_vga_init_13h(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_vga_init_x(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int sys_vga_palette(uint32_t index, uint32_t rgb, uint32_t unused);

/* System call table */
static syscall_fn syscall_table[NUM_SYSCALLS] = {
    [SYS_EXIT]    = sys_exit,
    [SYS_WRITE]   = sys_write,
    [SYS_READ]    = sys_read,
    [SYS_OPEN]    = NULL,       /* Reserved */
    [SYS_CLOSE]   = NULL,       /* Reserved */
    [SYS_SLEEP]   = sys_sleep,
    [SYS_BEEP]    = sys_beep,
    [SYS_GETCHAR] = sys_getchar,
    [SYS_EXEC]    = sys_exec,
    [SYS_READDIR] = sys_readdir,
    [SYS_CLEAR]   = sys_clear,
    [SYS_SETCOLOR] = sys_setcolor,
    [SYS_VGA_INIT]    = sys_vga_init,
    [SYS_VGA_EXIT]    = sys_vga_exit,
    [SYS_VGA_CLEAR]   = sys_vga_clear,
    [SYS_VGA_PIXEL]   = sys_vga_pixel,
    [SYS_VGA_LINE]    = sys_vga_line,
    [SYS_VGA_RECT]    = sys_vga_rect,
    [SYS_VGA_CIRCLE]  = sys_vga_circle,
    [SYS_VGA_INIT_13H] = sys_vga_init_13h,
    [SYS_VGA_INIT_X]   = sys_vga_init_x,
    [SYS_VGA_PALETTE]  = sys_vga_palette,
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
 * SYS_READ - Read from stdin (keyboard)
 */
static int sys_read(uint32_t fd, uint32_t buf, uint32_t len) {
    if (fd != 0) {
        return -1;  /* Only stdin supported */
    }
    
    char *buffer = (char *)buf;
    return keyboard_readline(buffer, (int)len);
}

/**
 * SYS_GETCHAR - Get a single character from keyboard
 */
static int sys_getchar(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    return (int)keyboard_getchar();
}

/**
 * SYS_EXEC - Execute a program
 * Returns exit code on success, negative on error
 * The executed program runs and returns when it calls exit()
 */
static int sys_exec(uint32_t path, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    const char *prog_path = (const char *)path;
    program_t prog;
    
    /* Save current program as parent before loading child */
    program_t *current = loader_get_current();
    if (current != NULL) {
        loader_set_parent(current->name);
    }
    
    if (loader_load(prog_path, &prog) != 0) {
        return -1;
    }
    
    /* Execute the program - this will NOT return! 
     * When child exits, loader_exit will restart the parent */
    loader_exec(&prog);
    
    /* Never reached - parent restarts from beginning */
    return 0;
}

/**
 * SYS_READDIR - Read a directory entry
 * @param path: Directory path
 * @param index: Entry index (0-based)
 * @param buf: Buffer to store entry name (must be at least 256 bytes)
 * @return: 1 if entry found, 0 if no more entries, -1 on error
 */
static int sys_readdir(uint32_t path, uint32_t index, uint32_t buf) {
    const char *dir_path = (const char *)path;
    char *entry_buf = (char *)buf;
    
    fs_node_t *dir = fs_namei(dir_path);
    if (!dir || !(dir->flags & FS_DIRECTORY)) {
        return -1;
    }
    
    dirent_t *dirent = fs_readdir(dir, index);
    if (!dirent) {
        return 0;  /* No more entries */
    }
    
    /* Copy entry name to buffer */
    strncpy(entry_buf, dirent->name, 255);
    entry_buf[255] = '\0';
    
    return 1;
}

/**
 * SYS_CLEAR - Clear the screen
 */
static int sys_clear(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_clear();
    return 0;
}

/**
 * SYS_SETCOLOR - Set text color
 * @param fg: Foreground color (0-15)
 * @param bg: Background color (0-15)
 */
static int sys_setcolor(uint32_t fg, uint32_t bg, uint32_t unused) {
    (void)unused;
    
    /* Clamp colors to valid range */
    if (fg > 15) fg = 15;
    if (bg > 15) bg = 15;
    
    vga_set_color((enum vga_color)fg, (enum vga_color)bg);
    return 0;
}

/**
 * SYS_VGA_INIT - Enter VGA graphics mode 12h (640x480x16)
 */
static int sys_vga_init(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_gfx_init();
    return 0;
}

/**
 * SYS_VGA_EXIT - Exit VGA graphics mode, return to text mode
 */
static int sys_vga_exit(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_gfx_exit();
    return 0;
}

/**
 * SYS_VGA_CLEAR - Clear graphics screen with color
 * @param color: Fill color (depends on mode)
 * Mode-aware: works with 12h, 13h, and mode X
 */
static int sys_vga_clear(uint32_t color, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    int mode = vga_gfx_get_mode();
    switch (mode) {
        case VGA_MODE_12H:
            vga_gfx_clear((uint8_t)color);
            break;
        case VGA_MODE_13H:
            vga_13h_clear((uint8_t)color);
            break;
        case VGA_MODE_X:
            vga_x_clear((uint8_t)color);
            break;
        default:
            return -1;  /* Not in graphics mode */
    }
    return 0;
}

/**
 * SYS_VGA_PIXEL - Set a pixel
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param color: Color (depends on mode)
 * Mode-aware: works with 12h, 13h, and mode X
 */
static int sys_vga_pixel(uint32_t x, uint32_t y, uint32_t color) {
    int mode = vga_gfx_get_mode();
    switch (mode) {
        case VGA_MODE_12H:
            vga_gfx_set_pixel((int)x, (int)y, (uint8_t)color);
            break;
        case VGA_MODE_13H:
            vga_13h_set_pixel((int)x, (int)y, (uint8_t)color);
            break;
        case VGA_MODE_X:
            vga_x_set_pixel((int)x, (int)y, (uint8_t)color);
            break;
        default:
            return -1;  /* Not in graphics mode */
    }
    return 0;
}

/**
 * SYS_VGA_LINE - Draw a line
 * @param packed1: x1 | (y1 << 16)
 * @param packed2: x2 | (y2 << 16)
 * @param color: Color (0-15)
 */
static int sys_vga_line(uint32_t packed1, uint32_t packed2, uint32_t color) {
    int x1 = (int)(packed1 & 0xFFFF);
    int y1 = (int)(packed1 >> 16);
    int x2 = (int)(packed2 & 0xFFFF);
    int y2 = (int)(packed2 >> 16);
    
    vga_gfx_line(x1, y1, x2, y2, (uint8_t)color);
    return 0;
}

/**
 * SYS_VGA_RECT - Draw a rectangle
 * @param packed_xy: x | (y << 16)
 * @param packed_wh: w | (h << 16)
 * @param color_fill: color | (filled << 8)
 */
static int sys_vga_rect(uint32_t packed_xy, uint32_t packed_wh, uint32_t color_fill) {
    int x = (int)(packed_xy & 0xFFFF);
    int y = (int)(packed_xy >> 16);
    int w = (int)(packed_wh & 0xFFFF);
    int h = (int)(packed_wh >> 16);
    uint8_t color = (uint8_t)(color_fill & 0xFF);
    int filled = (color_fill >> 8) & 1;
    
    if (filled) {
        vga_gfx_fill_rect(x, y, w, h, color);
    } else {
        vga_gfx_rect(x, y, w, h, color);
    }
    return 0;
}

/**
 * SYS_VGA_CIRCLE - Draw a circle
 * @param packed_xy: cx | (cy << 16)
 * @param r: radius
 * @param color_fill: color | (filled << 8)
 */
static int sys_vga_circle(uint32_t packed_xy, uint32_t r, uint32_t color_fill) {
    int cx = (int)(packed_xy & 0xFFFF);
    int cy = (int)(packed_xy >> 16);
    uint8_t color = (uint8_t)(color_fill & 0xFF);
    int filled = (color_fill >> 8) & 1;
    
    if (filled) {
        vga_gfx_fill_circle(cx, cy, (int)r, color);
    } else {
        vga_gfx_circle(cx, cy, (int)r, color);
    }
    return 0;
}

/**
 * SYS_VGA_INIT_13H - Enter VGA graphics mode 13h (320x200x256)
 */
static int sys_vga_init_13h(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_gfx_init_13h();
    return 0;
}

/**
 * SYS_VGA_INIT_X - Enter VGA graphics mode X (320x240x256)
 */
static int sys_vga_init_x(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_gfx_init_x();
    return 0;
}

/**
 * SYS_VGA_PALETTE - Set a VGA palette entry
 * @param index: Palette index (0-255)
 * @param rgb: Color value (r | (g << 8) | (b << 16))
 */
static int sys_vga_palette(uint32_t index, uint32_t rgb, uint32_t unused) {
    (void)unused;
    
    uint8_t r = (uint8_t)(rgb & 0xFF);
    uint8_t g = (uint8_t)((rgb >> 8) & 0xFF);
    uint8_t b = (uint8_t)((rgb >> 16) & 0xFF);
    
    vga_set_palette((uint8_t)index, r, g, b);
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
