/**
 * VGA Mode Y Demo
 * Demonstrates VGA mode Y (320x200, 256 colors, planar)
 * Mode Y is like Mode X but with 200 lines, allowing 4 full pages
 * 
 * Compiled as ELF32 executable by the build system.
 * Entry point: _start at virtual address 0x400000
 */

#include <io.h>
#include <user.h>
#include <vga_gfx.h>

/**
 * Set up a colorful gradient palette
 */
static void setup_gradient_palette(void) {
    int i;
    
    /* Colors 0-63: Red gradient */
    for (i = 0; i < 64; i++) {
        gfx_set_palette(i, i, 0, 0);
    }
    
    /* Colors 64-127: Green gradient */
    for (i = 0; i < 64; i++) {
        gfx_set_palette(64 + i, 0, i, 0);
    }
    
    /* Colors 128-191: Blue gradient */
    for (i = 0; i < 64; i++) {
        gfx_set_palette(128 + i, 0, 0, i);
    }
    
    /* Colors 192-255: Grayscale */
    for (i = 0; i < 64; i++) {
        gfx_set_palette(192 + i, i, i, i);
    }
}

/**
 * Set up a rainbow palette
 */
static void setup_rainbow_palette(void) {
    int i;
    
    /* Create a smooth rainbow across all 256 colors */
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
static void draw_plasma(void) {
    int x, y;
    
    for (y = 0; y < GFX_HEIGHT_Y; y++) {
        for (x = 0; x < GFX_WIDTH_Y; x++) {
            /* Simple plasma-like calculation */
            int color = (x + y + (x * y / 64)) & 0xFF;
            gfx_pixel(x, y, color);
        }
    }
}

/**
 * Draw concentric circles
 */
static void draw_circles(void) {
    int cx = GFX_WIDTH_Y / 2;
    int cy = GFX_HEIGHT_Y / 2;
    int x, y;
    
    for (y = 0; y < GFX_HEIGHT_Y; y++) {
        for (x = 0; x < GFX_WIDTH_Y; x++) {
            int dx = x - cx;
            int dy = y - cy;
            /* Approximate distance (no sqrt needed) */
            int dist = dx * dx + dy * dy;
            /* Use distance to determine color */
            int color = (dist / 32) & 0xFF;
            gfx_pixel(x, y, color);
        }
    }
}

/**
 * Draw color bars
 */
static void draw_color_bars(void) {
    int x, y;
    int bar_width = GFX_WIDTH_Y / 16;
    
    for (y = 0; y < GFX_HEIGHT_Y; y++) {
        for (x = 0; x < GFX_WIDTH_Y; x++) {
            int color = (x / bar_width) * 16;  /* 16 color bands */
            gfx_pixel(x, y, color);
        }
    }
}

/**
 * Draw a gradient fill
 */
static void draw_gradient(void) {
    int x, y;
    
    for (y = 0; y < GFX_HEIGHT_Y; y++) {
        for (x = 0; x < GFX_WIDTH_Y; x++) {
            /* Horizontal gradient using x position */
            int color = (x * 256) / GFX_WIDTH_Y;
            gfx_pixel(x, y, color);
        }
    }
}

/*
 * The program entry point
 */
void _start(void) {
    print("=== VGA Mode Y Demo ===\n\n");
    print("Resolution: 320x200, 256 colors (planar)\n");
    print("Press any key to switch patterns.\n");
    print("Press any key to start...\n");
    getchar();
    
    gfx_init_y();
    
    /* Set up rainbow palette */
    setup_rainbow_palette();
    
    /* Draw gradient */
    draw_gradient();
    
    /* Wait for keypress */
    getchar();
    
    /* Draw plasma pattern */
    draw_plasma();
    
    /* Wait for keypress */
    getchar();
    
    /* Draw concentric circles */
    draw_circles();
    
    /* Wait for keypress */
    getchar();
    
    /* Draw color bars with gradient palette */
    setup_gradient_palette();
    draw_color_bars();
    
    /* Wait for keypress */
    getchar();
    
    gfx_exit();
    
    print("\n=== Demo Complete ===\n");
    
    exit(0);
}
