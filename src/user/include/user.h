/**
 * Userspace Library Header
 * Provides general system call wrappers for userspace programs
 */

#ifndef USER_H
#define USER_H

/* System call numbers */
#define SYS_EXIT    0
#define SYS_SLEEP   5
#define SYS_BEEP    6
#define SYS_EXEC    8

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
 * @param code: Exit code
 */
static inline void exit(int code) {
    syscall(SYS_EXIT, code, 0, 0);
    /* Never returns */
    while (1);
}

/**
 * Sleep for milliseconds
 * @param ms: Milliseconds to sleep
 */
static inline void sleep(int ms) {
    syscall(SYS_SLEEP, ms, 0, 0);
}

/**
 * Play a beep sound
 * @param freq: Frequency in Hz
 * @param duration: Duration in milliseconds
 */
static inline void beep(int freq, int duration) {
    syscall(SYS_BEEP, freq, duration, 0);
}

/**
 * Execute a program
 * @param path: Path to program
 * @return: -1 on error, does not return on success
 */
static inline int exec(const char *path) {
    return syscall(SYS_EXEC, (int)path, 0, 0);
}

#endif /* USER_H */
