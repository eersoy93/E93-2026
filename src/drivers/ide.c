/**
 * IDE (ATA/ATAPI) Driver
 * Integrated Drive Electronics - Hard disk and CD-ROM controller
 * Supports PIO mode for ATA and ATAPI devices
 */

#include <ide.h>
#include <idt.h>
#include <kernel.h>
#include <pit.h>
#include <ports.h>
#include <string.h>
#include <vga.h>

/* IDE channels */
static ide_channel_t ide_channels[2] = {
    { ATA_PRIMARY_DATA,   ATA_PRIMARY_CONTROL,   0, 0 },
    { ATA_SECONDARY_DATA, ATA_SECONDARY_CONTROL, 0, 0 }
};

/* Detected IDE devices */
static ide_device_t ide_devices[IDE_MAX_DRIVES];

/* Number of detected drives */
static uint8_t ide_drive_count = 0;

/* Identification buffer */
static uint16_t ide_buf[256];

/* IRQ flags for waiting */
static volatile uint8_t ide_irq_invoked = 0;

/**
 * Wait for ~400ns by reading alternate status port 4 times
 */
static void ide_400ns_delay(uint8_t channel) {
    inb(ide_channels[channel].ctrl);
    inb(ide_channels[channel].ctrl);
    inb(ide_channels[channel].ctrl);
    inb(ide_channels[channel].ctrl);
}

/**
 * Wait for BSY flag to clear
 * @return 0 on success, error code on timeout
 */
static int ide_wait_bsy(uint8_t channel) {
    uint32_t timeout = ATA_TIMEOUT;
    
    while ((inb(ide_channels[channel].base + 7) & ATA_SR_BSY) && timeout > 0) {
        timeout--;
    }
    
    return (timeout == 0) ? IDE_ERR_TIMEOUT : IDE_OK;
}

/**
 * Wait for DRQ flag (data ready)
 * @return 0 on success, error code on timeout or error
 */
static int ide_wait_drq(uint8_t channel) {
    uint32_t timeout = ATA_TIMEOUT;
    uint8_t status;
    
    while (timeout > 0) {
        status = inb(ide_channels[channel].base + 7);
        
        if (status & ATA_SR_ERR) {
            return IDE_ERR_READ;
        }
        if (status & ATA_SR_DF) {
            return IDE_ERR_DRIVE_FAULT;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            return IDE_OK;
        }
        
        timeout--;
    }
    
    return IDE_ERR_TIMEOUT;
}

/**
 * Poll for operation completion
 * @return 0 on success, error code on failure
 */
static int ide_poll(uint8_t channel, bool check_error) {
    /* Wait 400ns */
    ide_400ns_delay(channel);
    
    /* Wait for BSY to clear */
    int err = ide_wait_bsy(channel);
    if (err != IDE_OK) {
        return err;
    }
    
    if (check_error) {
        uint8_t status = inb(ide_channels[channel].base + 7);
        
        if (status & ATA_SR_ERR) {
            return IDE_ERR_READ;
        }
        if (status & ATA_SR_DF) {
            return IDE_ERR_DRIVE_FAULT;
        }
        if (!(status & ATA_SR_DRQ)) {
            return IDE_ERR_READ;
        }
    }
    
    return IDE_OK;
}

/**
 * Select a drive
 */
static void ide_select_drive(uint8_t channel, uint8_t drive) {
    uint8_t select = (drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    outb(ide_channels[channel].base + 6, select);
    ide_400ns_delay(channel);
}

/**
 * Soft reset an IDE channel
 */
__attribute__((unused))
static void ide_soft_reset(uint8_t channel) {
    /* Send reset command */
    outb(ide_channels[channel].ctrl, ATA_CTRL_SRST);
    ide_400ns_delay(channel);
    
    /* Clear reset */
    outb(ide_channels[channel].ctrl, 0);
    ide_400ns_delay(channel);
    
    /* Wait for BSY to clear */
    ide_wait_bsy(channel);
}

/**
 * Read identification data from a device
 * @return Device type (IDE_TYPE_ATA, IDE_TYPE_ATAPI, or IDE_TYPE_NONE)
 */
static uint8_t ide_identify(uint8_t channel, uint8_t drive) {
    uint8_t type = IDE_TYPE_ATA;
    uint8_t status;
    uint16_t base = ide_channels[channel].base;
    
    /* Select drive */
    ide_select_drive(channel, drive);
    
    /* Send IDENTIFY command */
    outb(base + 2, 0);  /* Sector count */
    outb(base + 3, 0);  /* LBA low */
    outb(base + 4, 0);  /* LBA mid */
    outb(base + 5, 0);  /* LBA high */
    outb(base + 7, ATA_CMD_IDENTIFY);
    
    /* Wait a bit */
    ide_400ns_delay(channel);
    
    /* Check if device exists */
    status = inb(base + 7);
    if (status == 0) {
        return IDE_TYPE_NONE;
    }
    
    /* Wait for BSY to clear */
    uint32_t timeout = ATA_TIMEOUT;
    while ((status & ATA_SR_BSY) && timeout > 0) {
        status = inb(base + 7);
        timeout--;
    }
    
    if (timeout == 0) {
        return IDE_TYPE_NONE;
    }
    
    /* Check for ATAPI device */
    uint8_t lba_mid = inb(base + 4);
    uint8_t lba_hi = inb(base + 5);
    
    if (lba_mid == 0x14 && lba_hi == 0xEB) {
        /* ATAPI device */
        type = IDE_TYPE_ATAPI;
        outb(base + 7, ATA_CMD_IDENTIFY_PACKET);
        ide_400ns_delay(channel);
    } else if (lba_mid == 0x69 && lba_hi == 0x96) {
        /* ATAPI device (alternate signature) */
        type = IDE_TYPE_ATAPI;
        outb(base + 7, ATA_CMD_IDENTIFY_PACKET);
        ide_400ns_delay(channel);
    } else if (lba_mid != 0 || lba_hi != 0) {
        /* Unknown device type */
        return IDE_TYPE_NONE;
    }
    
    /* Wait for data ready */
    if (ide_poll(channel, true) != IDE_OK) {
        return IDE_TYPE_NONE;
    }
    
    /* Read identification data */
    for (int i = 0; i < 256; i++) {
        ide_buf[i] = inw(base);
    }
    
    return type;
}

/**
 * Read ATAPI device capacity using SCSI READ CAPACITY(10) command
 * @param drive: Drive number (0-3)
 * @param size: Pointer to store capacity in sectors
 * @return 0 on success, error code on failure
 */
static int ide_atapi_read_capacity(uint8_t drive, uint32_t *size) {
    ide_device_t *dev = &ide_devices[drive];
    uint16_t base = ide_channels[dev->channel].base;
    uint8_t select;
    int err;
    uint8_t packet[12];
    uint8_t capacity_data[8];

    /* Wait for drive ready */
    err = ide_wait_bsy(dev->channel);
    if (err != IDE_OK) return err;

    /* Select drive */
    select = (dev->drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    outb(base + 6, select);
    ide_400ns_delay(dev->channel);

    /* Set up ATAPI command */
    outb(base + 1, 0);      /* Features = 0 (PIO) */
    outb(base + 4, 8);      /* Byte count low (8 bytes response) */
    outb(base + 5, 0);      /* Byte count high */

    /* Send PACKET command */
    outb(base + 7, ATA_CMD_PACKET);

    /* Wait for DRQ */
    err = ide_wait_drq(dev->channel);
    if (err != IDE_OK) return err;

    /* Build SCSI READ CAPACITY(10) command */
    memset(packet, 0, sizeof(packet));
    packet[0] = 0x25;  /* READ CAPACITY(10) opcode */

    /* Send packet */
    for (int i = 0; i < 6; i++) {
        outw(base, ((uint16_t *)packet)[i]);
    }

    /* Wait for data ready */
    err = ide_poll(dev->channel, true);
    if (err != IDE_OK) return err;

    /* Read 8 bytes of capacity data (4 words) */
    uint16_t *buf = (uint16_t *)capacity_data;
    for (int i = 0; i < 4; i++) {
        buf[i] = inw(base);
    }

    /* Extract last LBA (big-endian format) */
    uint32_t last_lba = ((uint32_t)capacity_data[0] << 24) |
                        ((uint32_t)capacity_data[1] << 16) |
                        ((uint32_t)capacity_data[2] << 8) |
                        capacity_data[3];

    /* Size in sectors = last_lba + 1 */
    *size = last_lba + 1;

    return IDE_OK;
}

/**
 * Extract a string from identification data
 * ATA strings are stored as big-endian words
 */
static void ide_extract_string(uint16_t *src, char *dst, int words) {
    int i;
    for (i = 0; i < words; i++) {
        dst[i * 2] = (src[i] >> 8) & 0xFF;
        dst[i * 2 + 1] = src[i] & 0xFF;
    }
    dst[words * 2] = '\0';
    
    /* Trim trailing spaces */
    for (i = words * 2 - 1; i >= 0 && dst[i] == ' '; i--) {
        dst[i] = '\0';
    }
}

/**
 * Primary IDE interrupt handler (IRQ14)
 */
static void ide_primary_handler(interrupt_frame_t *frame) {
    UNUSED(frame);
    ide_irq_invoked = 1;
}

/**
 * Secondary IDE interrupt handler (IRQ15)
 */
static void ide_secondary_handler(interrupt_frame_t *frame) {
    UNUSED(frame);
    ide_irq_invoked = 1;
}

/**
 * Initialize IDE controller and detect drives
 */
void ide_init(void) {
    int channel, drive;
    uint8_t type;
    
    /* Clear device array */
    memset(ide_devices, 0, sizeof(ide_devices));
    ide_drive_count = 0;
    
    /* Install IRQ handlers */
    irq_install_handler(14, ide_primary_handler);
    irq_install_handler(15, ide_secondary_handler);
    
    /* Disable interrupts on both channels */
    outb(ATA_PRIMARY_CONTROL, ATA_CTRL_NIEN);
    outb(ATA_SECONDARY_CONTROL, ATA_CTRL_NIEN);
    
    /* Scan for devices */
    for (channel = 0; channel < 2; channel++) {
        for (drive = 0; drive < 2; drive++) {
            int dev_num = channel * 2 + drive;
            
            /* Try to identify the device */
            type = ide_identify(channel, drive);
            
            if (type == IDE_TYPE_NONE) {
                continue;
            }
            
            /* Fill in device information */
            ide_devices[dev_num].present = 1;
            ide_devices[dev_num].channel = channel;
            ide_devices[dev_num].drive = drive;
            ide_devices[dev_num].type = type;
            
            /* Extract device information from identification data */
            ide_devices[dev_num].signature = ide_buf[0];
            ide_devices[dev_num].capabilities = ide_buf[49];
            ide_devices[dev_num].command_sets = (ide_buf[83] << 16) | ide_buf[82];
            
            /* Get device size */
            if (ide_devices[dev_num].command_sets & (1 << 26)) {
                /* 48-bit LBA supported */
                ide_devices[dev_num].size = (ide_buf[103] << 16) | ide_buf[102];
            } else {
                /* 28-bit LBA */
                ide_devices[dev_num].size = (ide_buf[61] << 16) | ide_buf[60];
            }
            
            /* Extract strings */
            ide_extract_string(&ide_buf[27], ide_devices[dev_num].model, 20);
            ide_extract_string(&ide_buf[10], ide_devices[dev_num].serial, 10);
            ide_extract_string(&ide_buf[23], ide_devices[dev_num].firmware, 4);

            /* Get ATAPI capacity using READ CAPACITY command */
            if (type == IDE_TYPE_ATAPI) {
                uint32_t capacity = 0;
                if (ide_atapi_read_capacity(dev_num, &capacity) == IDE_OK) {
                    ide_devices[dev_num].size = capacity;
                }
            }
            
            ide_drive_count++;
        }
    }
}

/**
 * Get IDE device information
 */
ide_device_t *ide_get_device(uint8_t drive) {
    if (drive >= IDE_MAX_DRIVES || !ide_devices[drive].present) {
        return NULL;
    }
    return &ide_devices[drive];
}

/**
 * Read sectors from ATA device using PIO mode (28-bit LBA)
 */
int ide_read_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, void *buffer) {
    ide_device_t *dev;
    uint16_t base;
    uint8_t select;
    uint16_t *buf = (uint16_t *)buffer;
    int err;
    
    /* Validate parameters */
    if (drive >= IDE_MAX_DRIVES) {
        return IDE_ERR_INVALID;
    }
    
    dev = &ide_devices[drive];
    if (!dev->present) {
        return IDE_ERR_NO_DEVICE;
    }
    
    if (dev->type != IDE_TYPE_ATA) {
        return IDE_ERR_INVALID;
    }
    
    base = ide_channels[dev->channel].base;
    
    /* Wait for drive to be ready */
    err = ide_wait_bsy(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Select drive and set LBA mode */
    select = (dev->drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    select |= ATA_DRIVE_LBA;
    select |= ((lba >> 24) & 0x0F);  /* LBA bits 24-27 */
    outb(base + 6, select);
    
    ide_400ns_delay(dev->channel);
    
    /* Set sector count and LBA */
    outb(base + 2, sectors);
    outb(base + 3, lba & 0xFF);
    outb(base + 4, (lba >> 8) & 0xFF);
    outb(base + 5, (lba >> 16) & 0xFF);
    
    /* Send read command */
    outb(base + 7, ATA_CMD_READ_PIO);
    
    /* Read sectors */
    for (int s = 0; s < sectors; s++) {
        /* Wait for data ready */
        err = ide_poll(dev->channel, true);
        if (err != IDE_OK) {
            return err;
        }
        
        /* Read 256 words (512 bytes) */
        for (int i = 0; i < 256; i++) {
            *buf++ = inw(base);
        }
    }
    
    return IDE_OK;
}

/**
 * Write sectors to ATA device using PIO mode (28-bit LBA)
 */
int ide_write_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, const void *buffer) {
    ide_device_t *dev;
    uint16_t base;
    uint8_t select;
    const uint16_t *buf = (const uint16_t *)buffer;
    int err;
    
    /* Validate parameters */
    if (drive >= IDE_MAX_DRIVES) {
        return IDE_ERR_INVALID;
    }
    
    dev = &ide_devices[drive];
    if (!dev->present) {
        return IDE_ERR_NO_DEVICE;
    }
    
    if (dev->type != IDE_TYPE_ATA) {
        return IDE_ERR_INVALID;
    }
    
    base = ide_channels[dev->channel].base;
    
    /* Wait for drive to be ready */
    err = ide_wait_bsy(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Select drive and set LBA mode */
    select = (dev->drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    select |= ATA_DRIVE_LBA;
    select |= ((lba >> 24) & 0x0F);  /* LBA bits 24-27 */
    outb(base + 6, select);
    
    ide_400ns_delay(dev->channel);
    
    /* Set sector count and LBA */
    outb(base + 2, sectors);
    outb(base + 3, lba & 0xFF);
    outb(base + 4, (lba >> 8) & 0xFF);
    outb(base + 5, (lba >> 16) & 0xFF);
    
    /* Send write command */
    outb(base + 7, ATA_CMD_WRITE_PIO);
    
    /* Write sectors */
    for (int s = 0; s < sectors; s++) {
        /* Wait for drive ready */
        err = ide_poll(dev->channel, false);
        if (err != IDE_OK) {
            return err;
        }
        
        /* Wait for DRQ */
        err = ide_wait_drq(dev->channel);
        if (err != IDE_OK) {
            return err;
        }
        
        /* Write 256 words (512 bytes) */
        for (int i = 0; i < 256; i++) {
            outw(base, *buf++);
        }
    }
    
    /* Flush cache */
    outb(base + 7, ATA_CMD_CACHE_FLUSH);
    err = ide_wait_bsy(dev->channel);
    
    return err;
}

/**
 * Read sectors from ATAPI device (CD-ROM) using PIO mode
 */
int ide_atapi_read(uint8_t drive, uint32_t lba, uint8_t sectors, void *buffer) {
    ide_device_t *dev;
    uint16_t base;
    uint8_t select;
    uint16_t *buf = (uint16_t *)buffer;
    int err;
    uint8_t packet[12];
    
    /* Validate parameters */
    if (drive >= IDE_MAX_DRIVES) {
        return IDE_ERR_INVALID;
    }
    
    dev = &ide_devices[drive];
    if (!dev->present) {
        return IDE_ERR_NO_DEVICE;
    }
    
    if (dev->type != IDE_TYPE_ATAPI) {
        return IDE_ERR_INVALID;
    }
    
    base = ide_channels[dev->channel].base;
    
    /* Wait for drive to be ready */
    err = ide_wait_bsy(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Select drive */
    select = (dev->drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    outb(base + 6, select);
    ide_400ns_delay(dev->channel);
    
    /* Set up ATAPI command */
    outb(base + 1, 0);                          /* Features = 0 (PIO) */
    outb(base + 4, ATAPI_SECTOR_SIZE & 0xFF);   /* Byte count low */
    outb(base + 5, ATAPI_SECTOR_SIZE >> 8);     /* Byte count high */
    
    /* Send PACKET command */
    outb(base + 7, ATA_CMD_PACKET);
    
    /* Wait for DRQ */
    err = ide_wait_drq(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Build SCSI READ(12) command packet */
    memset(packet, 0, sizeof(packet));
    packet[0] = ATAPI_CMD_READ;         /* READ command */
    packet[2] = (lba >> 24) & 0xFF;     /* LBA byte 3 */
    packet[3] = (lba >> 16) & 0xFF;     /* LBA byte 2 */
    packet[4] = (lba >> 8) & 0xFF;      /* LBA byte 1 */
    packet[5] = lba & 0xFF;             /* LBA byte 0 */
    packet[6] = 0;                      /* Transfer length (MSB) */
    packet[7] = 0;
    packet[8] = 0;
    packet[9] = sectors;                /* Transfer length (LSB) */
    
    /* Send packet */
    for (int i = 0; i < 6; i++) {
        outw(base, ((uint16_t *)packet)[i]);
    }
    
    /* Read sectors */
    for (int s = 0; s < sectors; s++) {
        /* Wait for data ready */
        err = ide_poll(dev->channel, true);
        if (err != IDE_OK) {
            return err;
        }
        
        /* Read 1024 words (2048 bytes) */
        for (int i = 0; i < 1024; i++) {
            *buf++ = inw(base);
        }
    }
    
    return IDE_OK;
}

/**
 * Eject ATAPI device media
 */
int ide_atapi_eject(uint8_t drive) {
    ide_device_t *dev;
    uint16_t base;
    uint8_t select;
    int err;
    uint8_t packet[12];
    
    /* Validate parameters */
    if (drive >= IDE_MAX_DRIVES) {
        return IDE_ERR_INVALID;
    }
    
    dev = &ide_devices[drive];
    if (!dev->present) {
        return IDE_ERR_NO_DEVICE;
    }
    
    if (dev->type != IDE_TYPE_ATAPI) {
        return IDE_ERR_INVALID;
    }
    
    base = ide_channels[dev->channel].base;
    
    /* Wait for drive to be ready */
    err = ide_wait_bsy(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Select drive */
    select = (dev->drive == IDE_SLAVE) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    outb(base + 6, select);
    ide_400ns_delay(dev->channel);
    
    /* Set up ATAPI command */
    outb(base + 1, 0);      /* Features = 0 */
    outb(base + 4, 0);      /* Byte count low */
    outb(base + 5, 0);      /* Byte count high */
    
    /* Send PACKET command */
    outb(base + 7, ATA_CMD_PACKET);
    
    /* Wait for DRQ */
    err = ide_wait_drq(dev->channel);
    if (err != IDE_OK) {
        return err;
    }
    
    /* Build SCSI START/STOP UNIT command packet (eject) */
    memset(packet, 0, sizeof(packet));
    packet[0] = ATAPI_CMD_EJECT;    /* START/STOP UNIT command */
    packet[4] = 0x02;               /* Eject (LoEj=1, Start=0) */
    
    /* Send packet */
    for (int i = 0; i < 6; i++) {
        outw(base, ((uint16_t *)packet)[i]);
    }
    
    /* Wait for completion */
    err = ide_wait_bsy(dev->channel);
    
    return err;
}

/**
 * Get number of detected drives
 */
uint8_t ide_get_drive_count(void) {
    return ide_drive_count;
}

/**
 * Print information about detected drives
 */
void ide_print_info(void) {
    for (int i = 0; i < IDE_MAX_DRIVES; i++) {
        vga_print("  Drive ");
        vga_putchar('0' + i);
        vga_print(": ");
        
        if (!ide_devices[i].present) {
            vga_print("None\n");
            continue;
        }
        
        if (ide_devices[i].type == IDE_TYPE_ATA) {
            vga_print("[ATA]   ");
        } else if (ide_devices[i].type == IDE_TYPE_ATAPI) {
            vga_print("[ATAPI] ");
        }
        
        vga_print(ide_devices[i].model);

        /* Print size in MB for both ATA and ATAPI */
        if (ide_devices[i].size > 0) {
            uint32_t size_mb;
            if (ide_devices[i].type == IDE_TYPE_ATA) {
                /* ATA: 512-byte sectors */
                size_mb = ide_devices[i].size / 2048;
            } else {
                /* ATAPI: 2048-byte sectors */
                size_mb = ide_devices[i].size / 512;
            }

            vga_print(" (");

            /* Simple integer to string */
            char size_str[16];
            int idx = 0;
            uint32_t n = size_mb;
            if (n == 0) {
                size_str[idx++] = '0';
            } else {
                char temp[16];
                int temp_idx = 0;
                while (n > 0) {
                    temp[temp_idx++] = '0' + (n % 10);
                    n /= 10;
                }
                while (temp_idx > 0) {
                    size_str[idx++] = temp[--temp_idx];
                }
            }
            size_str[idx] = '\0';

            vga_print(size_str);
            vga_print(" MB)");
        }

        vga_print("\n");
    }
}
