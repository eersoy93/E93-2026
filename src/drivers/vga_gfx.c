/**
 * VGA Graphics Mode Driver
 * Implements VGA mode 12h (640x480, 16 colors)
 */

#include <vga_gfx.h>
#include <vga.h>
#include <vga_font.h>
#include <ports.h>
#include <string.h>

/* Graphics mode state */
static int gfx_mode_active = 0;

/* VGA video memory pointer */
static volatile uint8_t *vga_mem = (volatile uint8_t *)VGA_GFX_MEMORY;

/* Mode 12h register values */
static const uint8_t mode12h_misc = 0xE3;

static const uint8_t mode12h_seq[] = {
    0x03, /* Reset */
    0x01, /* Clock Mode */
    0x0F, /* Plane Write - all planes enabled */
    0x00, /* Char Map */
    0x06  /* Memory Mode */
};

static const uint8_t mode12h_crtc[] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0xFF
};

static const uint8_t mode12h_gc[] = {
    0x00, /* Set/Reset */
    0x00, /* Enable Set/Reset */
    0x00, /* Color Compare */
    0x00, /* Data Rotate */
    0x00, /* Read Map Select */
    0x00, /* Mode */
    0x05, /* Misc */
    0x0F, /* Color Don't Care */
    0xFF  /* Bit Mask */
};

static const uint8_t mode12h_attr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x01, 0x00, 0x0F, 0x00, 0x00
};

/* Text mode register values (mode 3) for restoration */
static const uint8_t mode3_misc = 0x67;

static const uint8_t mode3_seq[] = {
    0x03, 0x00, 0x03, 0x00, 0x02
};

static const uint8_t mode3_crtc[] = {
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF
};

static const uint8_t mode3_gc[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF
};

static const uint8_t mode3_attr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

/**
 * Write VGA registers
 */
static void write_regs(const uint8_t *regs_seq, const uint8_t *regs_crtc,
                       const uint8_t *regs_gc, const uint8_t *regs_attr,
                       uint8_t misc) {
    unsigned i;
    
    /* Write Miscellaneous Output Register */
    outb(VGA_MISC_WRITE, misc);
    
    /* Write Sequencer registers */
    for (i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, regs_seq[i]);
    }
    
    /* Unlock CRTC registers */
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
    
    /* Write CRTC registers */
    for (i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, regs_crtc[i]);
    }
    
    /* Write Graphics Controller registers */
    for (i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, regs_gc[i]);
    }
    
    /* Write Attribute Controller registers */
    inb(VGA_INPUT_STATUS);  /* Reset flip-flop */
    for (i = 0; i < 21; i++) {
        outb(VGA_ATTR_INDEX, i);
        outb(VGA_ATTR_INDEX, regs_attr[i]);
    }
    
    /* Enable video */
    inb(VGA_INPUT_STATUS);
    outb(VGA_ATTR_INDEX, 0x20);
}

/**
 * Initialize VGA graphics mode 12h
 */
void vga_gfx_init(void) {
    if (gfx_mode_active) return;
    
    /* Set mode 12h (640x480x16) */
    write_regs(mode12h_seq, mode12h_crtc, mode12h_gc, mode12h_attr, mode12h_misc);
    
    gfx_mode_active = 1;
    
    /* Clear screen to black */
    vga_gfx_clear(GFX_BLACK);
}

/**
 * Load VGA 8x16 font into plane 2
 */
static void load_font(void) {
    volatile uint8_t *font_mem = (volatile uint8_t *)0xA0000;
    
    /* Set up sequencer for font loading */
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x01);  /* Synchronous reset */
    
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, 0x04);  /* Write to plane 2 only */
    
    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x07);  /* Sequential addressing, extended memory */
    
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x03);  /* Clear synchronous reset */
    
    /* Set up graphics controller for font loading */
    outb(VGA_GC_INDEX, 0x04);
    outb(VGA_GC_DATA, 0x02);   /* Read from plane 2 */
    
    outb(VGA_GC_INDEX, 0x05);
    outb(VGA_GC_DATA, 0x00);   /* Write mode 0 */
    
    outb(VGA_GC_INDEX, 0x06);
    outb(VGA_GC_DATA, 0x00);   /* Map at A0000, not odd/even */
    
    /* Copy font data - 256 characters, 16 bytes each, but VGA uses 32-byte slots */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            font_mem[i * 32 + j] = vga_font_8x16[i * 16 + j];
        }
        /* Clear remaining bytes in the 32-byte slot */
        for (int j = 16; j < 32; j++) {
            font_mem[i * 32 + j] = 0;
        }
    }
    
    /* Restore sequencer for text mode */
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x01);  /* Synchronous reset */
    
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, 0x03);  /* Write to planes 0 and 1 */
    
    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x03);  /* Odd/even addressing */
    
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x03);  /* Clear synchronous reset */
    
    /* Restore graphics controller for text mode */
    outb(VGA_GC_INDEX, 0x04);
    outb(VGA_GC_DATA, 0x00);   /* Read from plane 0 */
    
    outb(VGA_GC_INDEX, 0x05);
    outb(VGA_GC_DATA, 0x10);   /* Odd/even mode */
    
    outb(VGA_GC_INDEX, 0x06);
    outb(VGA_GC_DATA, 0x0E);   /* Map at B8000, odd/even */
}

/**
 * Return to VGA text mode
 */
void vga_gfx_exit(void) {
    if (!gfx_mode_active) return;
    
    /* Set mode 3 (80x25 text) */
    write_regs(mode3_seq, mode3_crtc, mode3_gc, mode3_attr, mode3_misc);
    
    /* Reload the font into plane 2 */
    load_font();
    
    gfx_mode_active = 0;
    
    /* Reinitialize VGA text mode and clear screen */
    vga_init();
    vga_clear();
}

/**
 * Clear the graphics screen
 */
void vga_gfx_clear(uint8_t color) {
    if (!gfx_mode_active) return;
    
    /* Set write mode 0, enable all planes */
    outb(VGA_GC_INDEX, VGA_GC_MODE);
    outb(VGA_GC_DATA, 0x00);
    
    outb(VGA_SEQ_INDEX, VGA_SEQ_PLANE_WRITE);
    outb(VGA_SEQ_DATA, 0x0F);
    
    /* Set up for fill */
    outb(VGA_GC_INDEX, VGA_GC_ENABLE_SET_RESET);
    outb(VGA_GC_DATA, 0x0F);
    
    outb(VGA_GC_INDEX, VGA_GC_SET_RESET);
    outb(VGA_GC_DATA, color);
    
    outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
    outb(VGA_GC_DATA, 0xFF);
    
    /* Fill all video memory */
    uint32_t size = (VGA_GFX_WIDTH * VGA_GFX_HEIGHT) / 8;
    for (uint32_t i = 0; i < size; i++) {
        volatile uint8_t dummy = vga_mem[i];
        (void)dummy;
        vga_mem[i] = 0xFF;
    }
    
    /* Reset enable set/reset */
    outb(VGA_GC_INDEX, VGA_GC_ENABLE_SET_RESET);
    outb(VGA_GC_DATA, 0x00);
}

/**
 * Set a single pixel using write mode 2
 */
void vga_gfx_set_pixel(int x, int y, uint8_t color) {
    if (!gfx_mode_active) return;
    if (x < 0 || x >= VGA_GFX_WIDTH || y < 0 || y >= VGA_GFX_HEIGHT) return;
    
    uint32_t offset = (y * VGA_GFX_WIDTH + x) / 8;
    uint8_t bit = 0x80 >> (x & 7);
    
    /* Set write mode 2 */
    outb(VGA_GC_INDEX, VGA_GC_MODE);
    outb(VGA_GC_DATA, 0x02);
    
    /* Set bit mask for the specific pixel */
    outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
    outb(VGA_GC_DATA, bit);
    
    /* Read-modify-write to set the pixel */
    volatile uint8_t dummy = vga_mem[offset];
    (void)dummy;
    vga_mem[offset] = color;
    
    /* Reset to write mode 0 */
    outb(VGA_GC_INDEX, VGA_GC_MODE);
    outb(VGA_GC_DATA, 0x00);
    
    outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
    outb(VGA_GC_DATA, 0xFF);
}

/**
 * Get a pixel color
 */
uint8_t vga_gfx_get_pixel(int x, int y) {
    if (!gfx_mode_active) return 0;
    if (x < 0 || x >= VGA_GFX_WIDTH || y < 0 || y >= VGA_GFX_HEIGHT) return 0;
    
    uint32_t offset = (y * VGA_GFX_WIDTH + x) / 8;
    uint8_t bit = 0x80 >> (x & 7);
    uint8_t color = 0;
    
    /* Read from each plane */
    for (int plane = 0; plane < 4; plane++) {
        outb(VGA_GC_INDEX, VGA_GC_READ_MAP_SELECT);
        outb(VGA_GC_DATA, plane);
        
        if (vga_mem[offset] & bit) {
            color |= (1 << plane);
        }
    }
    
    return color;
}

/**
 * Draw a horizontal line (optimized)
 */
void vga_gfx_hline(int x1, int x2, int y, uint8_t color) {
    if (!gfx_mode_active) return;
    if (y < 0 || y >= VGA_GFX_HEIGHT) return;
    
    /* Ensure x1 <= x2 */
    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    
    /* Clip to screen */
    if (x1 < 0) x1 = 0;
    if (x2 >= VGA_GFX_WIDTH) x2 = VGA_GFX_WIDTH - 1;
    if (x1 > x2) return;
    
    /* Set write mode 2 */
    outb(VGA_GC_INDEX, VGA_GC_MODE);
    outb(VGA_GC_DATA, 0x02);
    
    uint32_t start_byte = (y * VGA_GFX_WIDTH + x1) / 8;
    uint32_t end_byte = (y * VGA_GFX_WIDTH + x2) / 8;
    uint8_t start_mask = 0xFF >> (x1 & 7);
    uint8_t end_mask = 0xFF << (7 - (x2 & 7));
    
    if (start_byte == end_byte) {
        /* Single byte */
        uint8_t mask = start_mask & end_mask;
        outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
        outb(VGA_GC_DATA, mask);
        volatile uint8_t dummy = vga_mem[start_byte];
        (void)dummy;
        vga_mem[start_byte] = color;
    } else {
        /* First byte */
        outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
        outb(VGA_GC_DATA, start_mask);
        volatile uint8_t dummy = vga_mem[start_byte];
        (void)dummy;
        vga_mem[start_byte] = color;
        
        /* Middle bytes */
        outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
        outb(VGA_GC_DATA, 0xFF);
        for (uint32_t i = start_byte + 1; i < end_byte; i++) {
            dummy = vga_mem[i];
            vga_mem[i] = color;
        }
        
        /* Last byte */
        outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
        outb(VGA_GC_DATA, end_mask);
        dummy = vga_mem[end_byte];
        vga_mem[end_byte] = color;
    }
    
    /* Reset */
    outb(VGA_GC_INDEX, VGA_GC_MODE);
    outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX, VGA_GC_BIT_MASK);
    outb(VGA_GC_DATA, 0xFF);
}

/**
 * Draw a vertical line
 */
void vga_gfx_vline(int x, int y1, int y2, uint8_t color) {
    if (!gfx_mode_active) return;
    if (x < 0 || x >= VGA_GFX_WIDTH) return;
    
    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    
    if (y1 < 0) y1 = 0;
    if (y2 >= VGA_GFX_HEIGHT) y2 = VGA_GFX_HEIGHT - 1;
    
    for (int y = y1; y <= y2; y++) {
        vga_gfx_set_pixel(x, y, color);
    }
}

/**
 * Draw a line using Bresenham's algorithm
 */
void vga_gfx_line(int x1, int y1, int x2, int y2, uint8_t color) {
    if (!gfx_mode_active) return;
    
    /* Optimize for horizontal and vertical lines */
    if (y1 == y2) {
        vga_gfx_hline(x1, x2, y1, color);
        return;
    }
    if (x1 == x2) {
        vga_gfx_vline(x1, y1, y2, color);
        return;
    }
    
    int dx = x2 - x1;
    int dy = y2 - y1;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    int err = dx - dy;
    
    while (1) {
        vga_gfx_set_pixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * Draw a rectangle outline
 */
void vga_gfx_rect(int x, int y, int w, int h, uint8_t color) {
    if (!gfx_mode_active) return;
    if (w <= 0 || h <= 0) return;
    
    vga_gfx_hline(x, x + w - 1, y, color);
    vga_gfx_hline(x, x + w - 1, y + h - 1, color);
    vga_gfx_vline(x, y, y + h - 1, color);
    vga_gfx_vline(x + w - 1, y, y + h - 1, color);
}

/**
 * Draw a filled rectangle
 */
void vga_gfx_fill_rect(int x, int y, int w, int h, uint8_t color) {
    if (!gfx_mode_active) return;
    if (w <= 0 || h <= 0) return;
    
    for (int i = 0; i < h; i++) {
        vga_gfx_hline(x, x + w - 1, y + i, color);
    }
}

/**
 * Draw a circle outline using midpoint algorithm
 */
void vga_gfx_circle(int cx, int cy, int r, uint8_t color) {
    if (!gfx_mode_active) return;
    if (r <= 0) return;
    
    int x = 0;
    int y = r;
    int d = 1 - r;
    
    while (x <= y) {
        vga_gfx_set_pixel(cx + x, cy + y, color);
        vga_gfx_set_pixel(cx - x, cy + y, color);
        vga_gfx_set_pixel(cx + x, cy - y, color);
        vga_gfx_set_pixel(cx - x, cy - y, color);
        vga_gfx_set_pixel(cx + y, cy + x, color);
        vga_gfx_set_pixel(cx - y, cy + x, color);
        vga_gfx_set_pixel(cx + y, cy - x, color);
        vga_gfx_set_pixel(cx - y, cy - x, color);
        
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/**
 * Draw a filled circle
 */
void vga_gfx_fill_circle(int cx, int cy, int r, uint8_t color) {
    if (!gfx_mode_active) return;
    if (r <= 0) return;
    
    int x = 0;
    int y = r;
    int d = 1 - r;
    
    while (x <= y) {
        vga_gfx_hline(cx - x, cx + x, cy + y, color);
        vga_gfx_hline(cx - x, cx + x, cy - y, color);
        vga_gfx_hline(cx - y, cx + y, cy + x, color);
        vga_gfx_hline(cx - y, cx + y, cy - x, color);
        
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/**
 * Check if graphics mode is active
 */
int vga_gfx_is_active(void) {
    return gfx_mode_active;
}
