# VGA Graphics Programming Guide

This guide covers VGA graphics programming for E93-2026 user programs.

## Available Graphics Modes

| Mode | Resolution | Colors | Type | Best For |
|------|------------|--------|------|----------|
| 12h | 640×480 | 16 | Planar | High-res line art, text |
| 13h | 320×200 | 256 | Linear | Simple games, images |
| X | 320×240 | 256 | Planar | Square pixels, animation |
| Y | 320×200 | 256 | Planar | Multi-page animation |

## Getting Started

Include the graphics header and initialize your desired mode:

```c
#include <user.h>
#include <io.h>
#include <vga_gfx.h>

void _start(void) {
    /* Initialize graphics mode */
    gfx_init_13h();  /* or gfx_init(), gfx_init_x(), gfx_init_y() */
    
    /* Draw something */
    gfx_clear(0);
    gfx_pixel(160, 100, 15);
    
    /* Wait for keypress */
    getchar();
    
    /* Return to text mode */
    gfx_exit();
    exit(0);
}
```

## Mode Initialization

```c
void gfx_init(void);      /* Mode 12h: 640×480, 16 colors */
void gfx_init_13h(void);  /* Mode 13h: 320×200, 256 colors */
void gfx_init_x(void);    /* Mode X:   320×240, 256 colors */
void gfx_init_y(void);    /* Mode Y:   320×200, 256 colors (planar) */
void gfx_exit(void);      /* Return to text mode */
```

## Screen Dimensions

Use the appropriate constants for each mode:

```c
/* Mode 12h */
#define GFX_WIDTH_12H   640
#define GFX_HEIGHT_12H  480

/* Mode 13h */
#define GFX_WIDTH_13H   320
#define GFX_HEIGHT_13H  200

/* Mode X */
#define GFX_WIDTH_X     320
#define GFX_HEIGHT_X    240

/* Mode Y */
#define GFX_WIDTH_Y     320
#define GFX_HEIGHT_Y    200

/* Legacy (Mode 12h) */
#define GFX_WIDTH       640
#define GFX_HEIGHT      480
```

## Drawing Functions

### Clear Screen

```c
void gfx_clear(int color);
```

Fill the entire screen with a color.

### Pixel

```c
void gfx_pixel(int x, int y, int color);
```

Draw a single pixel at (x, y).

### Line

```c
void gfx_line(int x1, int y1, int x2, int y2, int color);
void gfx_hline(int x1, int x2, int y, int color);  /* Horizontal */
void gfx_vline(int x, int y1, int y2, int color);  /* Vertical */
```

Draw lines between points.

### Rectangle

```c
void gfx_rect(int x, int y, int w, int h, int color);      /* Outline */
void gfx_fill_rect(int x, int y, int w, int h, int color); /* Filled */
```

Draw rectangles starting at (x, y) with dimensions w×h.

### Circle

```c
void gfx_circle(int cx, int cy, int r, int color);      /* Outline */
void gfx_fill_circle(int cx, int cy, int r, int color); /* Filled */
```

Draw circles centered at (cx, cy) with radius r.

### Text (Mode 12h only)

```c
void gfx_char(int x, int y, char c, int color);
void gfx_string(int x, int y, const char *str, int color);
```

Draw text using the built-in 8×8 font.

## Palette Programming (256-color modes)

In 256-color modes, you can customize the color palette:

```c
void gfx_set_palette(int index, int r, int g, int b);
```

- `index`: Color index (0-255)
- `r`, `g`, `b`: Color components (0-63 each)

### Example: Rainbow Palette

```c
static void setup_rainbow_palette(void) {
    int i;
    for (i = 0; i < 256; i++) {
        int r, g, b;
        if (i < 43) {
            r = 63; g = (i * 63) / 42; b = 0;
        } else if (i < 85) {
            r = 63 - ((i - 43) * 63) / 42; g = 63; b = 0;
        } else if (i < 128) {
            r = 0; g = 63; b = ((i - 85) * 63) / 42;
        } else if (i < 170) {
            r = 0; g = 63 - ((i - 128) * 63) / 42; b = 63;
        } else if (i < 213) {
            r = ((i - 170) * 63) / 42; g = 0; b = 63;
        } else {
            r = 63; g = 0; b = 63 - ((i - 213) * 63) / 42;
        }
        gfx_set_palette(i, r, g, b);
    }
}
```

### Example: Gradient Palette

```c
static void setup_gradient_palette(void) {
    int i;
    /* Red gradient (0-63) */
    for (i = 0; i < 64; i++)
        gfx_set_palette(i, i, 0, 0);
    /* Green gradient (64-127) */
    for (i = 0; i < 64; i++)
        gfx_set_palette(64 + i, 0, i, 0);
    /* Blue gradient (128-191) */
    for (i = 0; i < 64; i++)
        gfx_set_palette(128 + i, 0, 0, i);
    /* Grayscale (192-255) */
    for (i = 0; i < 64; i++)
        gfx_set_palette(192 + i, i, i, i);
}
```

## 16-Color Palette (Mode 12h)

Mode 12h uses a fixed 16-color palette:

```c
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
```

## Common Patterns

### Plasma Effect

```c
static void draw_plasma(int width, int height) {
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = (x + y + (x * y / 64)) & 0xFF;
            gfx_pixel(x, y, color);
        }
    }
}
```

### Concentric Circles

```c
static void draw_circles(int width, int height) {
    int cx = width / 2;
    int cy = height / 2;
    int x, y;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int dist = dx * dx + dy * dy;
            int color = (dist / 32) & 0xFF;
            gfx_pixel(x, y, color);
        }
    }
}
```

### Horizontal Gradient

```c
static void draw_gradient(int width, int height) {
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = (x * 256) / width;
            gfx_pixel(x, y, color);
        }
    }
}
```

### Color Bars

```c
static void draw_color_bars(int width, int height) {
    int x, y;
    int bar_width = width / 16;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = (x / bar_width) * 16;
            gfx_pixel(x, y, color);
        }
    }
}
```

## Best Practices

1. **Always call `gfx_exit()`** before exiting to restore text mode
2. **Wait for keypress** between screens so users can see the output
3. **Use appropriate dimension constants** for your chosen mode
4. **Set up palette early** in 256-color modes before drawing
5. **Bounds check** your coordinates to avoid undefined behavior
