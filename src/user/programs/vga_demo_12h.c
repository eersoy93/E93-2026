/**
 * VGA Graphics Demo
 * Demonstrates VGA mode 12h (640x480, 16 colors) graphics
 * 
 * Shows various drawing primitives and animations
 * 
 * Compiled as ELF32 executable by the build system.
 * Entry point: _start at virtual address 0x400000
 */

#include <io.h>
#include <syscall.h>
#include <vga_gfx.h>

/* Simple pseudo-random number generator */
static unsigned int seed = 12345;

static int rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/**
 * Draw a demo pattern showing all 16 colors
 */
static void demo_colors(void) {
    int bar_width = GFX_WIDTH / 16;
    int bar_height = 60;
    int y_offset = 20;
    
    /* Draw color bars */
    for (int i = 0; i < 16; i++) {
        gfx_fill_rect(i * bar_width, y_offset, bar_width, bar_height, i);
    }
}

/**
 * Draw random lines
 */
static void demo_lines(void) {
    for (int i = 0; i < 50; i++) {
        int x1 = rand() % GFX_WIDTH;
        int y1 = 100 + (rand() % 150);
        int x2 = rand() % GFX_WIDTH;
        int y2 = 100 + (rand() % 150);
        int color = 1 + (rand() % 15);  /* Avoid black */
        gfx_line(x1, y1, x2, y2, color);
    }
}

/**
 * Draw rectangles
 */
static void demo_rectangles(void) {
    /* Filled rectangles */
    for (int i = 0; i < 8; i++) {
        int x = 20 + i * 75;
        int y = 270;
        int color = 1 + (rand() % 15);
        gfx_fill_rect(x, y, 60, 40, color);
    }
    
    /* Outlined rectangles */
    for (int i = 0; i < 8; i++) {
        int x = 20 + i * 75;
        int y = 320;
        int color = 1 + (rand() % 15);
        gfx_rect(x, y, 60, 40, color);
    }
}

/**
 * Draw circles
 */
static void demo_circles(void) {
    /* Filled circles */
    for (int i = 0; i < 6; i++) {
        int cx = 70 + i * 100;
        int cy = 400;
        int r = 25 + (rand() % 15);
        int color = 1 + (rand() % 15);
        gfx_fill_circle(cx, cy, r, color);
    }
    
    /* Outlined circles */
    for (int i = 0; i < 6; i++) {
        int cx = 70 + i * 100;
        int cy = 455;
        int r = 15 + (rand() % 10);
        int color = 1 + (rand() % 15);
        gfx_circle(cx, cy, r, color);
    }
}

/**
 * Draw a starfield pattern
 */
static void demo_starfield(void) {
    gfx_clear(GFX_BLACK);
    
    /* Draw random stars */
    for (int i = 0; i < 200; i++) {
        int x = rand() % GFX_WIDTH;
        int y = rand() % GFX_HEIGHT;
        int brightness = rand() % 3;
        int color;
        
        switch (brightness) {
            case 0: color = GFX_DARK_GREY; break;
            case 1: color = GFX_LIGHT_GREY; break;
            default: color = GFX_WHITE; break;
        }
        
        gfx_pixel(x, y, color);
    }
    
    /* Draw a few bright "stars" */
    for (int i = 0; i < 10; i++) {
        int x = rand() % (GFX_WIDTH - 4) + 2;
        int y = rand() % (GFX_HEIGHT - 4) + 2;
        
        /* Draw a small cross for bright stars */
        gfx_pixel(x, y, GFX_WHITE);
        gfx_pixel(x-1, y, GFX_LIGHT_GREY);
        gfx_pixel(x+1, y, GFX_LIGHT_GREY);
        gfx_pixel(x, y-1, GFX_LIGHT_GREY);
        gfx_pixel(x, y+1, GFX_LIGHT_GREY);
    }
}

/**
 * Draw bouncing ball animation
 */
static void demo_bounce(void) {
    int ball_x = GFX_WIDTH / 2;
    int ball_y = GFX_HEIGHT / 2;
    int ball_r = 20;
    int dx = 5;
    int dy = 3;
    
    for (int frame = 0; frame < 150; frame++) {
        /* Clear old position */
        gfx_fill_circle(ball_x, ball_y, ball_r + 1, GFX_BLACK);
        
        /* Update position */
        ball_x += dx;
        ball_y += dy;
        
        /* Bounce off walls */
        if (ball_x - ball_r <= 0 || ball_x + ball_r >= GFX_WIDTH) {
            dx = -dx;
            ball_x += dx * 2;
        }
        if (ball_y - ball_r <= 0 || ball_y + ball_r >= GFX_HEIGHT) {
            dy = -dy;
            ball_y += dy * 2;
        }
        
        /* Draw ball */
        gfx_fill_circle(ball_x, ball_y, ball_r, GFX_LIGHT_RED);
        gfx_circle(ball_x, ball_y, ball_r, GFX_RED);
        
        /* Small delay */
        sleep(20);
    }
}

/**
 * Draw concentric circles animation
 */
static void demo_concentric(void) {
    gfx_clear(GFX_BLACK);
    
    int cx = GFX_WIDTH / 2;
    int cy = GFX_HEIGHT / 2;
    
    for (int r = 10; r < 230; r += 10) {
        int color = (r / 10) % 15 + 1;
        gfx_circle(cx, cy, r, color);
        sleep(50);
    }
}

/**
 * Draw a simple landscape
 */
static void demo_landscape(void) {
    gfx_clear(GFX_LIGHT_BLUE);
    
    /* Sun */
    gfx_fill_circle(100, 80, 40, GFX_YELLOW);
    
    /* Mountains */
    for (int x = 0; x < GFX_WIDTH; x++) {
        int h1 = 200 - (x < 200 ? x : (400 - x)) / 2;
        int h2 = 250 - ((x + 150) % 300 < 150 ? (x + 150) % 300 : (300 - (x + 150) % 300)) / 2;
        int h3 = 280 - ((x + 250) % 350 < 175 ? (x + 250) % 350 : (350 - (x + 250) % 350)) / 3;
        
        if (h1 < GFX_HEIGHT) gfx_vline(x, h1, 300, GFX_DARK_GREY);
        if (h2 < GFX_HEIGHT && h2 < h1) gfx_vline(x, h2, h1, GFX_LIGHT_GREY);
        if (h3 < GFX_HEIGHT && h3 < h2) gfx_vline(x, h3, h2, GFX_BROWN);
    }
    
    /* Ground */
    gfx_fill_rect(0, 300, GFX_WIDTH, 180, GFX_GREEN);
    
    /* Trees */
    for (int i = 0; i < 8; i++) {
        int tx = 50 + i * 80 + (rand() % 30);
        int ty = 320 + (rand() % 100);
        int th = 40 + (rand() % 30);
        
        /* Trunk */
        gfx_fill_rect(tx - 3, ty, 6, th / 2, GFX_BROWN);
        
        /* Foliage (triangle approximation) */
        for (int j = 0; j < th; j++) {
            int w = (th - j) / 2;
            gfx_hline(tx - w, tx + w, ty - j, GFX_LIGHT_GREEN);
        }
    }
    
    /* House */
    gfx_fill_rect(450, 360, 80, 60, GFX_RED);
    /* Roof */
    for (int j = 0; j < 40; j++) {
        int w = 50 - j;
        if (w > 0) gfx_hline(490 - w, 490 + w, 360 - j, GFX_BROWN);
    }
    /* Door */
    gfx_fill_rect(475, 385, 20, 35, GFX_BROWN);
    /* Window */
    gfx_fill_rect(505, 375, 15, 15, GFX_LIGHT_CYAN);
}

/**
 * Main demo entry point
 */
void _start(void) {
    print("Starting VGA Demo (12h mode)...\n");
    print("Resolution: 640x480, 16 colors (planar)\n");
    print("Press any key to begin...\n");
    getchar();

    /* Enter graphics mode */
    gfx_init();
    
    /* Demo 1: Color palette and primitives */
    gfx_clear(GFX_BLACK);
    demo_colors();
    demo_lines();
    demo_rectangles();
    demo_circles();
    sleep(3000);
    
    /* Demo 2: Starfield */
    demo_starfield();
    sleep(2000);
    
    /* Demo 3: Bouncing ball */
    gfx_clear(GFX_BLACK);
    demo_bounce();
    
    /* Demo 4: Concentric circles */
    demo_concentric();
    sleep(1000);
    
    /* Demo 5: Landscape scene */
    demo_landscape();
    sleep(4000);
    
    /* Return to text mode */
    gfx_exit();
    
    print("VGA Demo (12h mode) completed!\n");
    print("Press any key to exit...\n");
    getchar();
    
    exit(0);
}
