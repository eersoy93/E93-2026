/**
 * Userspace Library Header
 * Provides system call wrappers for userspace programs
 * 
 * This header can be used by C userspace programs
 */

#ifndef USER_H
#define USER_H

/* Type definitions for userspace */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

/* System call numbers */
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_OPEN    3
#define SYS_CLOSE   4
#define SYS_SLEEP   5
#define SYS_BEEP    6
#define SYS_GETCHAR 7
#define SYS_EXEC    8
#define SYS_READDIR 9
#define SYS_CLEAR   10
#define SYS_SETCOLOR 11

/* VGA color palette for userspace */
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

#define COLOR_ERROR   COLOR_LIGHT_RED
#define COLOR_INFO    COLOR_LIGHT_CYAN
#define COLOR_NORMAL  COLOR_LIGHT_GREY
#define COLOR_SUCCESS COLOR_LIGHT_GREEN
#define COLOR_WARNING COLOR_YELLOW

/* File descriptors */
#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* Boolean */
#define true    1
#define false   0

/**
 * Make a system call with up to 3 arguments
 */
static inline int syscall(int num, int arg1, int arg2, int arg3) {
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
 * Exit the program
 */
static inline void exit(int code) {
    syscall(SYS_EXIT, code, 0, 0);
    /* Never returns */
    while (1);
}

/**
 * Write to a file descriptor
 */
static inline int write(int fd, const char *buf, int len) {
    return syscall(SYS_WRITE, fd, (int)buf, len);
}

/**
 * Print a string to stdout
 */
static inline int print(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return write(STDOUT, str, len);
}

/**
 * Print a character
 */
static inline void putchar(char c) {
    write(STDOUT, &c, 1);
}

/**
 * Read a line from stdin
 */
static inline int readline(char *buf, int max_len) {
    return syscall(SYS_READ, STDIN, (int)buf, max_len);
}

/**
 * Get a single character from keyboard
 */
static inline int getchar(void) {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}

/**
 * Sleep for milliseconds
 */
static inline void sleep(int ms) {
    syscall(SYS_SLEEP, ms, 0, 0);
}

/**
 * Play a beep
 */
static inline void beep(int freq, int duration) {
    syscall(SYS_BEEP, freq, duration, 0);
}

/**
 * Execute a program
 * Returns -1 on error (program not found)
 * On success, does not return (replaces current program)
 */
static inline int exec(const char *path) {
    return syscall(SYS_EXEC, (int)path, 0, 0);
}

/**
 * Read a directory entry
 * @param path: Directory path
 * @param index: Entry index (0-based)
 * @param buf: Buffer to store entry name (at least 256 bytes)
 * @return: 1 if entry found, 0 if no more entries, -1 on error
 */
static inline int readdir(const char *path, int index, char *buf) {
    return syscall(SYS_READDIR, (int)path, index, (int)buf);
}

/**
 * Clear the screen
 */
static inline void clear(void) {
    syscall(SYS_CLEAR, 0, 0, 0);
}

/**
 * Set text color
 * @param fg: Foreground color (0-15, use COLOR_* constants)
 * @param bg: Background color (0-15, use COLOR_* constants)
 */
static inline void setcolor(int fg, int bg) {
    syscall(SYS_SETCOLOR, fg, bg, 0);
}

/**
 * Print a string with color
 * @param str: String to print
 * @param fg: Foreground color
 * @param bg: Background color
 */
static inline int print_color(const char *str, int fg, int bg) {
    setcolor(fg, bg);
    int ret = print(str);
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);  /* Reset to default */
    return ret;
}

/**
 * String length
 */
static inline int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

/**
 * String compare
 */
static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/**
 * String copy
 */
static inline char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

/**
 * String concatenate
 */
static inline char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

/**
 * Memory set
 */
static inline void *memset(void *s, int c, uint32_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

#endif /* USER_H */
