/**
 * I/O Library Header
 * Provides general input/output functions for userspace programs
 */

#ifndef IO_H
#define IO_H

#include <string.h>

/* System call numbers for I/O */
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_FOPEN   3
#define SYS_FCLOSE  4
#define SYS_GETCHAR 7
#define SYS_READDIR 9
#define SYS_CLEAR   10
#define SYS_SETCOLOR 11
#define SYS_FREAD   12
#define SYS_FSIZE   13

/* File descriptors */
#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* VGA color palette */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

/* Color aliases */
#define COLOR_ERROR   COLOR_LIGHT_RED
#define COLOR_INFO    COLOR_LIGHT_CYAN
#define COLOR_NORMAL  COLOR_LIGHT_GREY
#define COLOR_SUCCESS COLOR_LIGHT_GREEN
#define COLOR_WARNING COLOR_YELLOW

/**
 * Make a system call (internal use)
 */
static inline int _io_syscall(int num, int arg1, int arg2, int arg3) {
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
 * Write to a file descriptor
 * @param fd: File descriptor (STDIN, STDOUT, STDERR)
 * @param buf: Buffer to write
 * @param len: Number of bytes to write
 * @return: Number of bytes written
 */
static inline int write(int fd, const char *buf, int len) {
    return _io_syscall(SYS_WRITE, fd, (int)buf, len);
}

/**
 * Print a string to stdout
 * @param str: Null-terminated string
 * @return: Number of bytes written
 */
static inline int print(const char *str) {
    return write(STDOUT, str, strlen(str));
}

/**
 * Print a character to stdout
 * @param c: Character to print
 */
static inline void putchar(char c) {
    write(STDOUT, &c, 1);
}

/**
 * Read a line from stdin (with echo and editing)
 * @param buf: Buffer to store input
 * @param max_len: Maximum length to read
 * @return: Number of characters read, -1 on Ctrl+C
 */
static inline int readline(char *buf, int max_len) {
    return _io_syscall(SYS_READ, STDIN, (int)buf, max_len);
}

/**
 * Get a single character from keyboard (blocking)
 * @return: ASCII code of pressed key
 */
static inline int getchar(void) {
    return _io_syscall(SYS_GETCHAR, 0, 0, 0);
}

/**
 * Read a directory entry
 * @param path: Directory path
 * @param index: Entry index (0-based)
 * @param buf: Buffer to store entry name (at least 256 bytes)
 * @return: 1 if entry found, 0 if no more entries, -1 on error
 */
static inline int readdir(const char *path, int index, char *buf) {
    return _io_syscall(SYS_READDIR, (int)path, index, (int)buf);
}

/**
 * Clear the screen
 */
static inline void clear(void) {
    _io_syscall(SYS_CLEAR, 0, 0, 0);
}

/**
 * Set text color
 * @param fg: Foreground color (0-15, use COLOR_* constants)
 * @param bg: Background color (0-15, use COLOR_* constants)
 */
static inline void setcolor(int fg, int bg) {
    _io_syscall(SYS_SETCOLOR, fg, bg, 0);
}

/**
 * Print a string with specified color
 * @param str: String to print
 * @param fg: Foreground color
 * @param bg: Background color
 * @return: Number of bytes written
 */
static inline int print_color(const char *str, int fg, int bg) {
    setcolor(fg, bg);
    int ret = print(str);
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);  /* Reset to default */
    return ret;
}

/**
 * Print a decimal number
 * @param n: Number to print
 */
static inline void print_int(int n) {
    char buf[16];
    int i = 0;
    int neg = 0;
    
    if (n < 0) {
        neg = 1;
        n = -n;
    }
    
    if (n == 0) {
        putchar('0');
        return;
    }
    
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    if (neg) putchar('-');
    while (i > 0) putchar(buf[--i]);
}

/**
 * Print an unsigned hexadecimal number
 * @param n: Number to print
 */
static inline void print_hex(unsigned int n) {
    const char *hex = "0123456789abcdef";
    char buf[8];
    int i = 0;
    
    if (n == 0) {
        print("0x0");
        return;
    }
    
    while (n > 0) {
        buf[i++] = hex[n & 0xF];
        n >>= 4;
    }
    
    print("0x");
    while (i > 0) putchar(buf[--i]);
}

/**
 * Print a hex nibble (single hex digit)
 * @param val: Value (0-15)
 */
static inline void print_hex_nibble(unsigned char val) {
    if (val < 10) {
        putchar('0' + val);
    } else {
        putchar('a' + val - 10);
    }
}

/**
 * Print a 16-bit hex value (4 digits, no prefix)
 * @param val: 16-bit value to print
 */
static inline void print_hex16(unsigned short val) {
    print_hex_nibble((val >> 12) & 0xF);
    print_hex_nibble((val >> 8) & 0xF);
    print_hex_nibble((val >> 4) & 0xF);
    print_hex_nibble(val & 0xF);
}

/**
 * Print an 8-bit hex value (2 digits, no prefix)
 * @param val: 8-bit value to print
 */
static inline void print_hex8(unsigned char val) {
    print_hex_nibble((val >> 4) & 0xF);
    print_hex_nibble(val & 0xF);
}

/**
 * Print a string followed by newline
 * @param str: String to print
 * @return: Number of bytes written
 */
static inline int println(const char *str) {
    int ret = print(str);
    putchar('\n');
    return ret + 1;
}

/**
 * Print a newline
 */
static inline void newline(void) {
    putchar('\n');
}

/**
 * Print an error message in red
 * @param str: Error message
 */
static inline void print_error(const char *str) {
    print_color(str, COLOR_ERROR, COLOR_BLACK);
}

/**
 * Print a success message in green
 * @param str: Success message
 */
static inline void print_success(const char *str) {
    print_color(str, COLOR_SUCCESS, COLOR_BLACK);
}

/**
 * Print a warning message in yellow
 * @param str: Warning message
 */
static inline void print_warning(const char *str) {
    print_color(str, COLOR_WARNING, COLOR_BLACK);
}

/**
 * Print an info message in cyan
 * @param str: Info message
 */
static inline void print_info(const char *str) {
    print_color(str, COLOR_INFO, COLOR_BLACK);
}

/* ============================================
 * File I/O Functions
 * ============================================ */

/**
 * Open a file for reading
 * @param path: Path to the file
 * @return: File descriptor (>= 3) on success, -1 on error
 */
static inline int fopen(const char *path) {
    return _io_syscall(SYS_FOPEN, (int)path, 0, 0);
}

/**
 * Close a file
 * @param fd: File descriptor
 * @return: 0 on success, -1 on error
 */
static inline int fclose(int fd) {
    return _io_syscall(SYS_FCLOSE, fd, 0, 0);
}

/**
 * Read from a file
 * @param fd: File descriptor
 * @param buf: Buffer to read into
 * @param size: Number of bytes to read
 * @return: Number of bytes read, or -1 on error
 */
static inline int fread(int fd, char *buf, int size) {
    return _io_syscall(SYS_FREAD, fd, (int)buf, size);
}

/**
 * Get file size
 * @param fd: File descriptor
 * @return: File size in bytes, or -1 on error
 */
static inline int fsize(int fd) {
    return _io_syscall(SYS_FSIZE, fd, 0, 0);
}

/**
 * Read entire file into buffer (convenience function)
 * Opens, reads, and closes the file
 * @param path: Path to file
 * @param buf: Buffer to read into
 * @param max_size: Maximum bytes to read
 * @return: Number of bytes read, or -1 on error
 */
static inline int read_file(const char *path, char *buf, int max_size) {
    int fd = fopen(path);
    if (fd < 0) {
        return -1;
    }
    
    int size = fsize(fd);
    if (size < 0) {
        fclose(fd);
        return -1;
    }
    
    if (size > max_size) {
        size = max_size;
    }
    
    int bytes_read = fread(fd, buf, size);
    fclose(fd);
    
    return bytes_read;
}

#endif /* IO_H */
