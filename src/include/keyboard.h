/**
 * Keyboard Driver Header
 * PS/2 keyboard driver for reading user input
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"

/* Keyboard I/O ports */
#define KBD_DATA_PORT       0x60
#define KBD_STATUS_PORT     0x64
#define KBD_CMD_PORT        0x64

/* Keyboard buffer size */
#define KBD_BUFFER_SIZE     256

/* Special key codes */
#define KEY_BACKSPACE       0x08
#define KEY_TAB             0x09
#define KEY_ENTER           0x0A
#define KEY_ESCAPE          0x1B

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void);

/**
 * Keyboard interrupt handler
 */
void keyboard_handler(void);

/**
 * Check if a character is available in the buffer
 * @return 1 if character available, 0 otherwise
 */
int keyboard_has_char(void);

/**
 * Get a character from the keyboard buffer (blocking)
 * @return The character read
 */
char keyboard_getchar(void);

/**
 * Get a character from the keyboard buffer (non-blocking)
 * @return The character read, or 0 if no character available
 */
char keyboard_getchar_nonblock(void);

/**
 * Read a line from keyboard into buffer
 * @param buf: Buffer to store the line
 * @param max_len: Maximum length to read
 * @return Number of characters read
 */
int keyboard_readline(char *buf, int max_len);

#endif /* KEYBOARD_H */
