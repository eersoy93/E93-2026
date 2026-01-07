/**
 * IDE (ATA/ATAPI) Driver Header
 * Integrated Drive Electronics - Hard disk and CD-ROM controller
 */

#ifndef IDE_H
#define IDE_H

#include "stdint.h"
#include "stdbool.h"

/* ATA I/O Ports - Primary Bus */
#define ATA_PRIMARY_DATA        0x1F0   /* Data register (R/W) */
#define ATA_PRIMARY_ERROR       0x1F1   /* Error register (R) / Features (W) */
#define ATA_PRIMARY_FEATURES    0x1F1   /* Features register (W) */
#define ATA_PRIMARY_SECCOUNT    0x1F2   /* Sector count register */
#define ATA_PRIMARY_LBA_LO      0x1F3   /* LBA low byte / Sector number */
#define ATA_PRIMARY_LBA_MID     0x1F4   /* LBA mid byte / Cylinder low */
#define ATA_PRIMARY_LBA_HI      0x1F5   /* LBA high byte / Cylinder high */
#define ATA_PRIMARY_DRIVE       0x1F6   /* Drive/Head register */
#define ATA_PRIMARY_STATUS      0x1F7   /* Status register (R) */
#define ATA_PRIMARY_COMMAND     0x1F7   /* Command register (W) */
#define ATA_PRIMARY_CONTROL     0x3F6   /* Device control register */
#define ATA_PRIMARY_ALTSTATUS   0x3F6   /* Alternate status register */

/* ATA I/O Ports - Secondary Bus */
#define ATA_SECONDARY_DATA      0x170   /* Data register (R/W) */
#define ATA_SECONDARY_ERROR     0x171   /* Error register (R) / Features (W) */
#define ATA_SECONDARY_FEATURES  0x171   /* Features register (W) */
#define ATA_SECONDARY_SECCOUNT  0x172   /* Sector count register */
#define ATA_SECONDARY_LBA_LO    0x173   /* LBA low byte / Sector number */
#define ATA_SECONDARY_LBA_MID   0x174   /* LBA mid byte / Cylinder low */
#define ATA_SECONDARY_LBA_HI    0x175   /* LBA high byte / Cylinder high */
#define ATA_SECONDARY_DRIVE     0x176   /* Drive/Head register */
#define ATA_SECONDARY_STATUS    0x177   /* Status register (R) */
#define ATA_SECONDARY_COMMAND   0x177   /* Command register (W) */
#define ATA_SECONDARY_CONTROL   0x376   /* Device control register */
#define ATA_SECONDARY_ALTSTATUS 0x376   /* Alternate status register */

/* ATA Status Register Bits */
#define ATA_SR_BSY      0x80    /* Busy */
#define ATA_SR_DRDY     0x40    /* Drive ready */
#define ATA_SR_DF       0x20    /* Drive fault */
#define ATA_SR_DSC      0x10    /* Drive seek complete */
#define ATA_SR_DRQ      0x08    /* Data request ready */
#define ATA_SR_CORR     0x04    /* Corrected data */
#define ATA_SR_IDX      0x02    /* Index */
#define ATA_SR_ERR      0x01    /* Error */

/* ATA Error Register Bits */
#define ATA_ER_BBK      0x80    /* Bad block */
#define ATA_ER_UNC      0x40    /* Uncorrectable data */
#define ATA_ER_MC       0x20    /* Media changed */
#define ATA_ER_IDNF     0x10    /* ID mark not found */
#define ATA_ER_MCR      0x08    /* Media change request */
#define ATA_ER_ABRT     0x04    /* Command aborted */
#define ATA_ER_TK0NF    0x02    /* Track 0 not found */
#define ATA_ER_AMNF     0x01    /* No address mark */

/* ATA Commands */
#define ATA_CMD_READ_PIO        0x20    /* Read sectors (PIO) */
#define ATA_CMD_READ_PIO_EXT    0x24    /* Read sectors (PIO, 48-bit LBA) */
#define ATA_CMD_READ_DMA        0xC8    /* Read sectors (DMA) */
#define ATA_CMD_READ_DMA_EXT    0x25    /* Read sectors (DMA, 48-bit LBA) */
#define ATA_CMD_WRITE_PIO       0x30    /* Write sectors (PIO) */
#define ATA_CMD_WRITE_PIO_EXT   0x34    /* Write sectors (PIO, 48-bit LBA) */
#define ATA_CMD_WRITE_DMA       0xCA    /* Write sectors (DMA) */
#define ATA_CMD_WRITE_DMA_EXT   0x35    /* Write sectors (DMA, 48-bit LBA) */
#define ATA_CMD_CACHE_FLUSH     0xE7    /* Flush cache */
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA    /* Flush cache (48-bit LBA) */
#define ATA_CMD_PACKET          0xA0    /* ATAPI packet command */
#define ATA_CMD_IDENTIFY_PACKET 0xA1    /* Identify ATAPI device */
#define ATA_CMD_IDENTIFY        0xEC    /* Identify ATA device */

/* ATAPI Commands */
#define ATAPI_CMD_READ          0xA8    /* Read sectors */
#define ATAPI_CMD_EJECT         0x1B    /* Eject media */

/* Device Control Register Bits */
#define ATA_CTRL_SRST   0x04    /* Software reset */
#define ATA_CTRL_NIEN   0x02    /* Disable interrupts */

/* Drive Select Values */
#define ATA_DRIVE_MASTER    0xA0    /* Select master drive */
#define ATA_DRIVE_SLAVE     0xB0    /* Select slave drive */
#define ATA_DRIVE_LBA       0x40    /* Use LBA addressing */

/* IDE Channel definitions */
#define IDE_PRIMARY     0
#define IDE_SECONDARY   1

/* IDE Drive definitions */
#define IDE_MASTER      0
#define IDE_SLAVE       1

/* Device Types */
#define IDE_TYPE_NONE       0   /* No device */
#define IDE_TYPE_ATA        1   /* ATA hard disk */
#define IDE_TYPE_ATAPI      2   /* ATAPI CD-ROM/DVD */

/* Sector size */
#define ATA_SECTOR_SIZE     512
#define ATAPI_SECTOR_SIZE   2048

/* Timeout values (in ticks) */
#define ATA_TIMEOUT         5000    /* 5 second timeout */

/* Maximum drives */
#define IDE_MAX_DRIVES      4

/* IDE Device structure */
typedef struct {
    uint8_t     present;        /* Device is present */
    uint8_t     channel;        /* Primary (0) or Secondary (1) */
    uint8_t     drive;          /* Master (0) or Slave (1) */
    uint8_t     type;           /* ATA or ATAPI */
    uint16_t    signature;      /* Drive signature */
    uint16_t    capabilities;   /* Device capabilities */
    uint32_t    command_sets;   /* Supported command sets */
    uint32_t    size;           /* Size in sectors */
    char        model[41];      /* Model string (40 chars + null) */
    char        serial[21];     /* Serial number (20 chars + null) */
    char        firmware[9];    /* Firmware revision (8 chars + null) */
} ide_device_t;

/* IDE Channel structure */
typedef struct {
    uint16_t    base;           /* I/O base port */
    uint16_t    ctrl;           /* Control port */
    uint16_t    bmide;          /* Bus master IDE port */
    uint8_t     nien;           /* Interrupts disabled flag */
} ide_channel_t;

/* Function declarations */

/**
 * Initialize IDE controller and detect drives
 */
void ide_init(void);

/**
 * Get IDE device information
 * @param drive: Drive number (0-3)
 * @return Pointer to device structure or NULL if not present
 */
ide_device_t *ide_get_device(uint8_t drive);

/**
 * Read sectors from ATA device using PIO mode
 * @param drive: Drive number (0-3)
 * @param lba: Logical Block Address to read from
 * @param sectors: Number of sectors to read
 * @param buffer: Buffer to store data
 * @return 0 on success, error code on failure
 */
int ide_read_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, void *buffer);

/**
 * Write sectors to ATA device using PIO mode
 * @param drive: Drive number (0-3)
 * @param lba: Logical Block Address to write to
 * @param sectors: Number of sectors to write
 * @param buffer: Buffer containing data to write
 * @return 0 on success, error code on failure
 */
int ide_write_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, const void *buffer);

/**
 * Read sectors from ATAPI device (CD-ROM)
 * @param drive: Drive number (0-3)
 * @param lba: Logical Block Address to read from
 * @param sectors: Number of sectors to read
 * @param buffer: Buffer to store data
 * @return 0 on success, error code on failure
 */
int ide_atapi_read(uint8_t drive, uint32_t lba, uint8_t sectors, void *buffer);

/**
 * Eject ATAPI device media
 * @param drive: Drive number (0-3)
 * @return 0 on success, error code on failure
 */
int ide_atapi_eject(uint8_t drive);

/**
 * Get number of detected drives
 * @return Number of drives found
 */
uint8_t ide_get_drive_count(void);

/**
 * Print information about detected drives
 */
void ide_print_info(void);

/* Error codes */
#define IDE_OK              0       /* Success */
#define IDE_ERR_NO_DEVICE   -1      /* Device not present */
#define IDE_ERR_TIMEOUT     -2      /* Operation timed out */
#define IDE_ERR_DRIVE_FAULT -3      /* Drive fault */
#define IDE_ERR_READ        -4      /* Read error */
#define IDE_ERR_WRITE       -5      /* Write error */
#define IDE_ERR_INVALID     -6      /* Invalid parameter */

#endif /* IDE_H */
