/**
 * VGA Graphics Library Header
 * Provides VGA mode 12h (640x480x16) graphics for userspace programs
 */

#ifndef VGA_GFX_H
#define VGA_GFX_H

/* VGA Mode 12h dimensions */
#define GFX_WIDTH   640
#define GFX_HEIGHT  480

/* VGA graphics syscall numbers */
#define SYS_VGA_INIT    14
#define SYS_VGA_EXIT    15
#define SYS_VGA_CLEAR   16
#define SYS_VGA_PIXEL   17
#define SYS_VGA_LINE    18
#define SYS_VGA_RECT    19
#define SYS_VGA_CIRCLE  20

/* VGA 16-color palette */
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
 * Make a graphics system call
 */
static inline int _gfx_syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return ret;
}

/**
 * Initialize VGA graphics mode 12h (640x480, 16 colors)
 * Must be called before any drawing operations
 */
static inline void gfx_init(void) {
    _gfx_syscall(SYS_VGA_INIT, 0, 0, 0);
}

/**
 * Exit VGA graphics mode and return to text mode
 * Should be called when done with graphics
 */
static inline void gfx_exit(void) {
    _gfx_syscall(SYS_VGA_EXIT, 0, 0, 0);
}

/**
 * Clear the screen with a color
 * @param color: Fill color (0-15)
 */
static inline void gfx_clear(int color) {
    _gfx_syscall(SYS_VGA_CLEAR, color, 0, 0);
}

/**
 * Set a single pixel
 * @param x: X coordinate (0-639)
 * @param y: Y coordinate (0-479)
 * @param color: Color (0-15)
 */
static inline void gfx_pixel(int x, int y, int color) {
    _gfx_syscall(SYS_VGA_PIXEL, x, y, color);
}

/**
 * Draw a line
 * @param x1, y1: Start point
 * @param x2, y2: End point
 * @param color: Color (0-15)
 */
static inline void gfx_line(int x1, int y1, int x2, int y2, int color) {
    int packed1 = (x1 & 0xFFFF) | ((y1 & 0xFFFF) << 16);
    int packed2 = (x2 & 0xFFFF) | ((y2 & 0xFFFF) << 16);
    _gfx_syscall(SYS_VGA_LINE, packed1, packed2, color);
}

/**
 * Draw a rectangle outline
 * @param x, y: Top-left corner
 * @param w, h: Width and height
 * @param color: Color (0-15)
 */
static inline void gfx_rect(int x, int y, int w, int h, int color) {
    int packed_xy = (x & 0xFFFF) | ((y & 0xFFFF) << 16);
    int packed_wh = (w & 0xFFFF) | ((h & 0xFFFF) << 16);
    int color_fill = (color & 0xFF);  /* filled = 0 */
    _gfx_syscall(SYS_VGA_RECT, packed_xy, packed_wh, color_fill);
}

/**
 * Draw a filled rectangle
 * @param x, y: Top-left corner
 * @param w, h: Width and height
 * @param color: Color (0-15)
 */
static inline void gfx_fill_rect(int x, int y, int w, int h, int color) {
    int packed_xy = (x & 0xFFFF) | ((y & 0xFFFF) << 16);
    int packed_wh = (w & 0xFFFF) | ((h & 0xFFFF) << 16);
    int color_fill = (color & 0xFF) | 0x100;  /* filled = 1 */
    _gfx_syscall(SYS_VGA_RECT, packed_xy, packed_wh, color_fill);
}

/**
 * Draw a circle outline
 * @param cx, cy: Center point
 * @param r: Radius
 * @param color: Color (0-15)
 */
static inline void gfx_circle(int cx, int cy, int r, int color) {
    int packed_xy = (cx & 0xFFFF) | ((cy & 0xFFFF) << 16);
    int color_fill = (color & 0xFF);  /* filled = 0 */
    _gfx_syscall(SYS_VGA_CIRCLE, packed_xy, r, color_fill);
}

/**
 * Draw a filled circle
 * @param cx, cy: Center point
 * @param r: Radius
 * @param color: Color (0-15)
 */
static inline void gfx_fill_circle(int cx, int cy, int r, int color) {
    int packed_xy = (cx & 0xFFFF) | ((cy & 0xFFFF) << 16);
    int color_fill = (color & 0xFF) | 0x100;  /* filled = 1 */
    _gfx_syscall(SYS_VGA_CIRCLE, packed_xy, r, color_fill);
}

/**
 * Draw a horizontal line (convenience function)
 */
static inline void gfx_hline(int x1, int x2, int y, int color) {
    gfx_line(x1, y, x2, y, color);
}

/**
 * Draw a vertical line (convenience function)
 */
static inline void gfx_vline(int x, int y1, int y2, int color) {
    gfx_line(x, y1, x, y2, color);
}

#endif /* VGA_GFX_H */
