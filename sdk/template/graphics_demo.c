/**
 * Graphics Demo Template
 * 
 * Template for programs using VGA graphics.
 * Copy this file to src/user/programs/ and rename it.
 * 
 * Build with: make iso
 * Run with:   make run
 */

#include <syscall.h>
#include <io.h>
#include <vga_gfx.h>

/**
 * Set up a rainbow palette for 256-color modes
 */
static void setup_rainbow_palette(void) {
    int i;
    
    for (i = 0; i < 256; i++) {
        int r, g, b;
        
        if (i < 43) {
            /* Red to Yellow */
            r = 63;
            g = (i * 63) / 42;
            b = 0;
        } else if (i < 85) {
            /* Yellow to Green */
            r = 63 - ((i - 43) * 63) / 42;
            g = 63;
            b = 0;
        } else if (i < 128) {
            /* Green to Cyan */
            r = 0;
            g = 63;
            b = ((i - 85) * 63) / 42;
        } else if (i < 170) {
            /* Cyan to Blue */
            r = 0;
            g = 63 - ((i - 128) * 63) / 42;
            b = 63;
        } else if (i < 213) {
            /* Blue to Magenta */
            r = ((i - 170) * 63) / 42;
            g = 0;
            b = 63;
        } else {
            /* Magenta to Red */
            r = 63;
            g = 0;
            b = 63 - ((i - 213) * 63) / 42;
        }
        
        gfx_set_palette(i, r, g, b);
    }
}

/**
 * Draw a plasma-like pattern
 */
static void draw_plasma(int width, int height) {
    int x, y;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = (x + y + (x * y / 64)) & 0xFF;
            gfx_pixel(x, y, color);
        }
    }
}

/**
 * Draw concentric circles
 */
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

/**
 * Draw a horizontal gradient
 */
static void draw_gradient(int width, int height) {
    int x, y;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = (x * 256) / width;
            gfx_pixel(x, y, color);
        }
    }
}

/**
 * Draw vertical color bars
 */
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

/**
 * Program entry point
 */
void _start(void) {
    /* Show intro in text mode */
    print("=== Graphics Demo ===\n\n");
    print("This template demonstrates VGA graphics.\n");
    print("Press any key to start...\n");
    getchar();
    
    /* Initialize 256-color graphics mode */
    /* Options: gfx_init_13h(), gfx_init_x(), gfx_init_y(), gfx_init() */
    gfx_init_13h();
    
    /* Set up color palette */
    setup_rainbow_palette();
    
    /* Demo 1: Gradient */
    draw_gradient(GFX_WIDTH_13H, GFX_HEIGHT_13H);
    getchar();
    
    /* Demo 2: Plasma */
    draw_plasma(GFX_WIDTH_13H, GFX_HEIGHT_13H);
    getchar();
    
    /* Demo 3: Circles */
    draw_circles(GFX_WIDTH_13H, GFX_HEIGHT_13H);
    getchar();
    
    /* Demo 4: Color bars */
    draw_color_bars(GFX_WIDTH_13H, GFX_HEIGHT_13H);
    getchar();
    
    /* Return to text mode */
    gfx_exit();
    
    print("\n=== Demo Complete ===\n");
    
    exit(0);
}
