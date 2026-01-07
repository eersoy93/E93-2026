/**
 * VGA Text Mode Driver Header
 * VGA text mode definitions and function declarations
 */

#ifndef VGA_H
#define VGA_H

#include "stdint.h"
#include "stddef.h"

/* VGA text mode dimensions */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* VGA text mode memory address */
#define VGA_MEMORY 0xB8000

/* VGA color palette */
enum vga_color {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
};

/* Predefined color schemes */
#define VGA_COLOR_ERROR   VGA_COLOR_LIGHT_RED
#define VGA_COLOR_INFO    VGA_COLOR_LIGHT_CYAN
#define VGA_COLOR_NORMAL  VGA_COLOR_LIGHT_GREY
#define VGA_COLOR_SUCCESS VGA_COLOR_LIGHT_GREEN
#define VGA_COLOR_WARNING VGA_COLOR_YELLOW

/* VGA I/O ports */
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5

/* VGA cursor registers */
#define VGA_CURSOR_START  0x0A
#define VGA_CURSOR_END    0x0B
#define VGA_CURSOR_HIGH   0x0E
#define VGA_CURSOR_LOW    0x0F

/* Function declarations */
void vga_init(void);
void vga_clear(void);
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_hex(uint32_t num);
void vga_print_dec(uint32_t num);
void vga_set_cursor(size_t row, size_t col);
size_t vga_get_row(void);
size_t vga_get_col(void);

/* Cursor control */
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void vga_disable_cursor(void);
void vga_update_cursor(void);

#endif /* VGA_H */
