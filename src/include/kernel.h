/**
 * Kernel Header
 * Core kernel definitions and declarations
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"
#include "stddef.h"

/* Kernel version */
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

/* Kernel entry point */
void kernel_main(unsigned int magic, unsigned int *mboot_info);

/* Utility macros */
#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Memory operations */
void *memset(void *dest, int val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *s1, const void *s2, size_t n);

/* String operations */
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);

/* Port I/O */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

#endif /* KERNEL_H */
