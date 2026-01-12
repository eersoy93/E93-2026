/**
 * IDE Devices Library Header
 * Provides IDE device information for userspace programs
 */

#ifndef USER_IDE_H
#define USER_IDE_H

#include <io.h>

/* System call number for IDE info */
#define SYS_IDEINFO 25

/* IDE device types */
#define IDE_TYPE_NONE   0
#define IDE_TYPE_ATA    1
#define IDE_TYPE_ATAPI  2

/* IDE device info structure (matches kernel layout) */
typedef struct {
    unsigned char present;      /* Device is present */
    unsigned char channel;      /* Primary (0) or Secondary (1) */
    unsigned char drive;        /* Master (0) or Slave (1) */
    unsigned char type;         /* ATA or ATAPI */
    unsigned int size;          /* Size in sectors */
    char model[41];             /* Model string */
} ide_device_info_t;

/**
 * Get number of IDE drives
 * @return: Number of drives detected
 */
static inline int ide_get_drive_count(void) {
    return _io_syscall(SYS_IDEINFO, 0xFF, 0, 0);
}

/**
 * Get IDE device information
 * @param drive: Drive number (0-3)
 * @param info: Pointer to ide_device_info_t structure to fill
 * @return: 0 on success, -1 if device not present
 */
static inline int ide_get_device_info(int drive, ide_device_info_t *info) {
    return _io_syscall(SYS_IDEINFO, drive, (int)info, 0);
}

#endif /* USER_IDE_H */
