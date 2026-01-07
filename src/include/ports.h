/**
 * Port I/O Header
 * Low-level port I/O functions for x86
 */

#ifndef PORTS_H
#define PORTS_H

#include "stdint.h"

/* Output a byte to a port */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Input a byte from a port */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Output a word (16-bit) to a port */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Input a word (16-bit) from a port */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Output a dword (32-bit) to a port */
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

/* Input a dword (32-bit) from a port */
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* I/O wait - small delay for slow devices */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif /* PORTS_H */
