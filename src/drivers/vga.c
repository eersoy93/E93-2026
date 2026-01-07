/**
 * VGA Text Mode Driver
 * Handles text output to VGA text mode (80x25)
 */

#include "vga.h"

/* I/O port access */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* VGA text buffer address */
static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;

/* Current cursor position */
static size_t vga_row = 0;
static size_t vga_col = 0;

/* Current color attribute */
static uint8_t vga_color = 0;

/**
 * Create a VGA color attribute byte
 */
static inline uint8_t vga_make_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

/**
 * Create a VGA entry (character + color)
 */
static inline uint16_t vga_make_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/**
 * Initialize VGA text mode
 */
void vga_init(void) {
    vga_row = 0;
    vga_col = 0;
    vga_color = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_enable_cursor(14, 15);  /* Enable cursor with default scanlines */
    vga_update_cursor();
}

/**
 * Set the current text color
 */
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_make_color(fg, bg);
}

/**
 * Clear the screen
 */
void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_make_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
    vga_update_cursor();
}

/**
 * Scroll the screen up by one line
 */
static void vga_scroll(void) {
    /* Move all lines up by one */
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t dst = y * VGA_WIDTH + x;
            const size_t src = (y + 1) * VGA_WIDTH + x;
            vga_buffer[dst] = vga_buffer[src];
        }
    }

    /* Clear the last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buffer[index] = vga_make_entry(' ', vga_color);
    }

    vga_row = VGA_HEIGHT - 1;
}

/**
 * Put a character at the current cursor position
 */
void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
        }
    } else {
        const size_t index = vga_row * VGA_WIDTH + vga_col;
        vga_buffer[index] = vga_make_entry(c, vga_color);
        vga_col++;
    }

    /* Handle line wrap */
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    /* Handle scrolling */
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }

    /* Update hardware cursor position */
    vga_update_cursor();
}

/**
 * Print a string to the screen
 */
void vga_print(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/**
 * Print a hexadecimal number
 */
void vga_print_hex(uint32_t num) {
    static const char hex_chars[] = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[num & 0xF];
        num >>= 4;
    }

    vga_print(buffer);
}

/**
 * Print a decimal number
 */
void vga_print_dec(uint32_t num) {
    if (num == 0) {
        vga_putchar('0');
        return;
    }

    char buffer[12];
    int i = 0;

    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    /* Print in reverse order */
    while (i > 0) {
        vga_putchar(buffer[--i]);
    }
}

/**
 * Set cursor position
 */
void vga_set_cursor(size_t row, size_t col) {
    if (row < VGA_HEIGHT && col < VGA_WIDTH) {
        vga_row = row;
        vga_col = col;
        vga_update_cursor();
    }
}

/**
 * Get current cursor row
 */
size_t vga_get_row(void) {
    return vga_row;
}

/**
 * Get current cursor column
 */
size_t vga_get_col(void) {
    return vga_col;
}

/**
 * Enable the hardware cursor
 * @param cursor_start: Scanline where cursor starts (0-15)
 * @param cursor_end: Scanline where cursor ends (0-15)
 */
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_START);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xC0) | cursor_start);

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_END);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xE0) | cursor_end);
}

/**
 * Disable the hardware cursor
 */
void vga_disable_cursor(void) {
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_START);
    outb(VGA_DATA_REGISTER, 0x20);  /* Bit 5 set = cursor disabled */
}

/**
 * Update the hardware cursor position to match the software cursor
 */
void vga_update_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_col;

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}
