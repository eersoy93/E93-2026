/**
 * Kernel Header
 * Core kernel definitions and declarations
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"
#include "stddef.h"
#include "ports.h"

/* Kernel version */
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

/* Kernel entry point */
void kernel_main(unsigned int magic, unsigned int *mboot_info);

/* Utility macros */
#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif /* KERNEL_H */
