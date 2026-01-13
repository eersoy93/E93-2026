/**
 * General System Call Library Header
 * Provides general system call wrappers for userspace programs
 */

#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

/* System call numbers */
#define SYS_EXIT    0
#define SYS_SLEEP   5
#define SYS_BEEP    6
#define SYS_EXEC    8
#define SYS_MEMINFO 27

/* Memory information structure */
typedef struct {
    unsigned int mem_lower;     /* Lower memory in KB (below 1MB) */
    unsigned int mem_upper;     /* Upper memory in KB (above 1MB) */
    unsigned int total_kb;      /* Total usable memory in KB */
} mem_info_t;

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

/**
 * Get memory information
 * @param info: Pointer to mem_info_t structure to fill
 * @return: 0 on success, -1 on error
 */
static inline int get_mem_info(mem_info_t *info) {
    return syscall(SYS_MEMINFO, (int)info, 0, 0);
}

#endif /* USER_SYSCALL_H */
