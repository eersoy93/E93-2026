/**
 * Kernel Header
 * Core kernel definitions and declarations
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"
#include "stddef.h"
#include "ports.h"

/* Kernel entry point */
void kernel_main(unsigned int magic, unsigned int *mboot_info);

/* Memory information structure */
typedef struct {
    uint32_t mem_lower;     /* Lower memory in KB (below 1MB) */
    uint32_t mem_upper;     /* Upper memory in KB (above 1MB) */
    uint32_t total_kb;      /* Total usable memory in KB */
} mem_info_t;

/* Get memory information */
void kernel_get_mem_info(mem_info_t *info);

/* Utility macros */
#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif /* KERNEL_H */
