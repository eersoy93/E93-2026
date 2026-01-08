/**
 * VGA Graphics Mode Driver Header
 * VGA mode 12h (640x480, 16 colors) definitions
 */

#ifndef VGA_GFX_H
#define VGA_GFX_H

#include "stdint.h"

/* VGA Mode 12h dimensions */
#define VGA_GFX_WIDTH   640
#define VGA_GFX_HEIGHT  480
#define VGA_GFX_COLORS  16

/* VGA graphics memory address */
#define VGA_GFX_MEMORY  0xA0000

/* VGA I/O ports for graphics mode */
#define VGA_GC_INDEX    0x3CE   /* Graphics Controller Index */
#define VGA_GC_DATA     0x3CF   /* Graphics Controller Data */
#define VGA_SEQ_INDEX   0x3C4   /* Sequencer Index */
#define VGA_SEQ_DATA    0x3C5   /* Sequencer Data */
#define VGA_MISC_READ   0x3CC   /* Misc Output Read */
#define VGA_MISC_WRITE  0x3C2   /* Misc Output Write */
#define VGA_CRTC_INDEX  0x3D4   /* CRTC Index */
#define VGA_CRTC_DATA   0x3D5   /* CRTC Data */
#define VGA_ATTR_INDEX  0x3C0   /* Attribute Controller Index */
#define VGA_ATTR_DATA   0x3C1   /* Attribute Controller Data */
#define VGA_INPUT_STATUS 0x3DA  /* Input Status Register */

/* Graphics Controller registers */
#define VGA_GC_SET_RESET        0x00
#define VGA_GC_ENABLE_SET_RESET 0x01
#define VGA_GC_COLOR_COMPARE    0x02
#define VGA_GC_DATA_ROTATE      0x03
#define VGA_GC_READ_MAP_SELECT  0x04
#define VGA_GC_MODE             0x05
#define VGA_GC_MISC             0x06
#define VGA_GC_COLOR_DONT_CARE  0x07
#define VGA_GC_BIT_MASK         0x08

/* Sequencer registers */
#define VGA_SEQ_RESET           0x00
#define VGA_SEQ_CLOCK_MODE      0x01
#define VGA_SEQ_PLANE_WRITE     0x02
#define VGA_SEQ_CHAR_MAP        0x03
#define VGA_SEQ_MEMORY_MODE     0x04

/* VGA colors (EGA/VGA 16-color palette) */
#define GFX_BLACK           0
#define GFX_BLUE            1
#define GFX_GREEN           2
#define GFX_CYAN            3
#define GFX_RED             4
#define GFX_MAGENTA         5
#define GFX_BROWN           6
#define GFX_LIGHT_GREY      7
#define GFX_DARK_GREY       8
#define GFX_LIGHT_BLUE      9
#define GFX_LIGHT_GREEN     10
#define GFX_LIGHT_CYAN      11
#define GFX_LIGHT_RED       12
#define GFX_LIGHT_MAGENTA   13
#define GFX_YELLOW          14
#define GFX_WHITE           15

/**
 * Initialize VGA graphics mode 12h (640x480x16)
 */
void vga_gfx_init(void);

/**
 * Return to VGA text mode (mode 3)
 */
void vga_gfx_exit(void);

/**
 * Clear the graphics screen with a color
 * @param color: Color (0-15)
 */
void vga_gfx_clear(uint8_t color);

/**
 * Set a single pixel
 * @param x: X coordinate (0-639)
 * @param y: Y coordinate (0-479)
 * @param color: Color (0-15)
 */
void vga_gfx_set_pixel(int x, int y, uint8_t color);

/**
 * Get a pixel color
 * @param x: X coordinate (0-639)
 * @param y: Y coordinate (0-479)
 * @return: Color (0-15)
 */
uint8_t vga_gfx_get_pixel(int x, int y);

/**
 * Draw a horizontal line
 * @param x1: Start X
 * @param x2: End X
 * @param y: Y coordinate
 * @param color: Color (0-15)
 */
void vga_gfx_hline(int x1, int x2, int y, uint8_t color);

/**
 * Draw a vertical line
 * @param x: X coordinate
 * @param y1: Start Y
 * @param y2: End Y
 * @param color: Color (0-15)
 */
void vga_gfx_vline(int x, int y1, int y2, uint8_t color);

/**
 * Draw a line using Bresenham's algorithm
 * @param x1, y1: Start point
 * @param x2, y2: End point
 * @param color: Color (0-15)
 */
void vga_gfx_line(int x1, int y1, int x2, int y2, uint8_t color);

/**
 * Draw a rectangle outline
 * @param x, y: Top-left corner
 * @param w, h: Width and height
 * @param color: Color (0-15)
 */
void vga_gfx_rect(int x, int y, int w, int h, uint8_t color);

/**
 * Draw a filled rectangle
 * @param x, y: Top-left corner
 * @param w, h: Width and height
 * @param color: Color (0-15)
 */
void vga_gfx_fill_rect(int x, int y, int w, int h, uint8_t color);

/**
 * Draw a circle outline
 * @param cx, cy: Center point
 * @param r: Radius
 * @param color: Color (0-15)
 */
void vga_gfx_circle(int cx, int cy, int r, uint8_t color);

/**
 * Draw a filled circle
 * @param cx, cy: Center point
 * @param r: Radius
 * @param color: Color (0-15)
 */
void vga_gfx_fill_circle(int cx, int cy, int r, uint8_t color);

/**
 * Check if graphics mode is active
 * @return: 1 if in graphics mode, 0 otherwise
 */
int vga_gfx_is_active(void);

#endif /* VGA_GFX_H */
