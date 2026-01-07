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

/* File descriptors */
#define STDIN   0
#define STDOUT  1
#define STDERR  2

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

#endif /* USER_H */
