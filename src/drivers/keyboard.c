/**
 * Keyboard Driver Implementation
 * PS/2 keyboard driver for reading user input
 */

#include "keyboard.h"
#include "idt.h"
#include "ports.h"
#include "vga.h"

/* Keyboard buffer */
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile int kbd_read_index = 0;
static volatile int kbd_write_index = 0;

/* Shift state */
static volatile int shift_pressed = 0;
static volatile int caps_lock = 0;
static volatile int ctrl_pressed = 0;

/* US keyboard scancode to ASCII mapping (lowercase) */
static const char scancode_to_ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0
};

/* US keyboard scancode to ASCII mapping (uppercase/shifted) */
static const char scancode_to_ascii_shift[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0
};

/* Scancode definitions */
#define SCAN_LSHIFT     0x2A
#define SCAN_RSHIFT     0x36
#define SCAN_LCTRL      0x1D
#define SCAN_CAPS       0x3A
#define SCAN_RELEASE    0x80

/**
 * Add a character to the keyboard buffer
 */
static void kbd_buffer_put(char c) {
    int next_write = (kbd_write_index + 1) % KBD_BUFFER_SIZE;
    if (next_write != kbd_read_index) {
        kbd_buffer[kbd_write_index] = c;
        kbd_write_index = next_write;
    }
}

/**
 * Get a character from the keyboard buffer (non-blocking)
 */
static char kbd_buffer_get(void) {
    if (kbd_read_index == kbd_write_index) {
        return 0;  /* Buffer empty */
    }
    char c = kbd_buffer[kbd_read_index];
    kbd_read_index = (kbd_read_index + 1) % KBD_BUFFER_SIZE;
    return c;
}

/**
 * Keyboard interrupt handler (called by IRQ1)
 */
static void keyboard_irq_handler(interrupt_frame_t *frame) {
    (void)frame;
    
    uint8_t scancode = inb(KBD_DATA_PORT);
    
    /* Check for key release */
    if (scancode & SCAN_RELEASE) {
        uint8_t released = scancode & ~SCAN_RELEASE;
        if (released == SCAN_LSHIFT || released == SCAN_RSHIFT) {
            shift_pressed = 0;
        } else if (released == SCAN_LCTRL) {
            ctrl_pressed = 0;
        }
        return;
    }
    
    /* Handle modifier keys */
    if (scancode == SCAN_LSHIFT || scancode == SCAN_RSHIFT) {
        shift_pressed = 1;
        return;
    }
    if (scancode == SCAN_LCTRL) {
        ctrl_pressed = 1;
        return;
    }
    if (scancode == SCAN_CAPS) {
        caps_lock = !caps_lock;
        return;
    }
    
    /* Convert scancode to ASCII */
    if (scancode < 128) {
        char c;
        int use_shift = shift_pressed;
        
        /* Caps lock affects only letters */
        if (caps_lock) {
            char lower = scancode_to_ascii[scancode];
            if (lower >= 'a' && lower <= 'z') {
                use_shift = !use_shift;
            }
        }
        
        if (use_shift) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        /* Handle Ctrl+C */
        if (ctrl_pressed && (c == 'c' || c == 'C')) {
            c = 3;  /* ETX - End of Text */
        }
        
        if (c != 0) {
            kbd_buffer_put(c);
        }
    }
}

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void) {
    kbd_read_index = 0;
    kbd_write_index = 0;
    shift_pressed = 0;
    caps_lock = 0;
    ctrl_pressed = 0;
    
    /* Clear any pending data */
    while (inb(KBD_STATUS_PORT) & 1) {
        inb(KBD_DATA_PORT);
    }
    
    /* Install keyboard interrupt handler (IRQ1) */
    irq_install_handler(1, keyboard_irq_handler);
}

/**
 * Check if a character is available
 */
int keyboard_has_char(void) {
    return kbd_read_index != kbd_write_index;
}

/**
 * Get a character (blocking)
 */
char keyboard_getchar(void) {
    while (!keyboard_has_char()) {
        /* Enable interrupts and halt until next interrupt */
        __asm__ volatile("sti; hlt");
    }
    return kbd_buffer_get();
}

/**
 * Get a character (non-blocking)
 */
char keyboard_getchar_nonblock(void) {
    return kbd_buffer_get();
}

/**
 * Read a line from keyboard with echo
 */
int keyboard_readline(char *buf, int max_len) {
    int i = 0;
    
    while (i < max_len - 1) {
        char c = keyboard_getchar();
        
        if (c == '\n' || c == '\r') {
            buf[i] = '\0';
            vga_putchar('\n');
            return i;
        } else if (c == '\b' || c == 127) {
            /* Backspace */
            if (i > 0) {
                i--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
        } else if (c == 3) {
            /* Ctrl+C */
            buf[0] = '\0';
            vga_putchar('^');
            vga_putchar('C');
            vga_putchar('\n');
            return -1;
        } else if (c >= 32 && c < 127) {
            /* Printable character */
            buf[i++] = c;
            vga_putchar(c);
        }
    }
    
    buf[i] = '\0';
    return i;
}
