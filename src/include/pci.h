/**
 * PCI (Peripheral Component Interconnect) Driver Header
 * PCI bus enumeration and device access
 */

#ifndef PCI_H
#define PCI_H

#include "stdint.h"
#include "stdbool.h"

/* PCI Configuration Space Ports */
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

/* PCI Configuration Space Registers (offsets) */
#define PCI_VENDOR_ID       0x00    /* 16-bit */
#define PCI_DEVICE_ID       0x02    /* 16-bit */
#define PCI_COMMAND         0x04    /* 16-bit */
#define PCI_STATUS          0x06    /* 16-bit */
#define PCI_REVISION_ID     0x08    /* 8-bit */
#define PCI_PROG_IF         0x09    /* 8-bit */
#define PCI_SUBCLASS        0x0A    /* 8-bit */
#define PCI_CLASS           0x0B    /* 8-bit */
#define PCI_CACHE_LINE_SIZE 0x0C    /* 8-bit */
#define PCI_LATENCY_TIMER   0x0D    /* 8-bit */
#define PCI_HEADER_TYPE     0x0E    /* 8-bit */
#define PCI_BIST            0x0F    /* 8-bit */
#define PCI_BAR0            0x10    /* 32-bit */
#define PCI_BAR1            0x14    /* 32-bit */
#define PCI_BAR2            0x18    /* 32-bit */
#define PCI_BAR3            0x1C    /* 32-bit */
#define PCI_BAR4            0x20    /* 32-bit */
#define PCI_BAR5            0x24    /* 32-bit */
#define PCI_CARDBUS_CIS     0x28    /* 32-bit */
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C /* 16-bit */
#define PCI_SUBSYSTEM_ID    0x2E    /* 16-bit */
#define PCI_ROM_BASE        0x30    /* 32-bit */
#define PCI_CAPABILITIES    0x34    /* 8-bit */
#define PCI_INTERRUPT_LINE  0x3C    /* 8-bit */
#define PCI_INTERRUPT_PIN   0x3D    /* 8-bit */
#define PCI_MIN_GRANT       0x3E    /* 8-bit */
#define PCI_MAX_LATENCY     0x3F    /* 8-bit */

/* PCI Header Types */
#define PCI_HEADER_TYPE_NORMAL      0x00
#define PCI_HEADER_TYPE_BRIDGE      0x01
#define PCI_HEADER_TYPE_CARDBUS     0x02
#define PCI_HEADER_TYPE_MULTI_FUNC  0x80

/* Invalid Vendor ID (device not present) */
#define PCI_VENDOR_NONE     0xFFFF

/* Maximum PCI devices we can track */
#define PCI_MAX_DEVICES     64

/* PCI Class Codes */
#define PCI_CLASS_UNCLASSIFIED      0x00
#define PCI_CLASS_STORAGE           0x01
#define PCI_CLASS_NETWORK           0x02
#define PCI_CLASS_DISPLAY           0x03
#define PCI_CLASS_MULTIMEDIA        0x04
#define PCI_CLASS_MEMORY            0x05
#define PCI_CLASS_BRIDGE            0x06
#define PCI_CLASS_COMMUNICATION     0x07
#define PCI_CLASS_SYSTEM            0x08
#define PCI_CLASS_INPUT             0x09
#define PCI_CLASS_DOCKING           0x0A
#define PCI_CLASS_PROCESSOR         0x0B
#define PCI_CLASS_SERIAL_BUS        0x0C
#define PCI_CLASS_WIRELESS          0x0D
#define PCI_CLASS_INTELLIGENT_IO    0x0E
#define PCI_CLASS_SATELLITE         0x0F
#define PCI_CLASS_ENCRYPTION        0x10
#define PCI_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_OTHER             0xFF

/* PCI Device structure */
typedef struct {
    uint8_t     bus;            /* Bus number (0-255) */
    uint8_t     device;         /* Device number (0-31) */
    uint8_t     function;       /* Function number (0-7) */
    uint8_t     present;        /* Device is present */
    uint16_t    vendor_id;      /* Vendor ID */
    uint16_t    device_id;      /* Device ID */
    uint8_t     class_code;     /* Class code */
    uint8_t     subclass;       /* Subclass */
    uint8_t     prog_if;        /* Programming interface */
    uint8_t     revision;       /* Revision ID */
    uint8_t     header_type;    /* Header type */
    uint8_t     irq;            /* IRQ line */
    uint32_t    bar[6];         /* Base Address Registers */
} pci_device_t;

/* Function declarations */

/**
 * Initialize PCI bus and enumerate devices
 */
void pci_init(void);

/**
 * Read a 32-bit value from PCI configuration space
 * @param bus: Bus number
 * @param device: Device number
 * @param function: Function number
 * @param offset: Register offset (must be 4-byte aligned)
 * @return: 32-bit value read
 */
uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

/**
 * Read a 16-bit value from PCI configuration space
 * @param bus: Bus number
 * @param device: Device number
 * @param function: Function number
 * @param offset: Register offset
 * @return: 16-bit value read
 */
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

/**
 * Read an 8-bit value from PCI configuration space
 * @param bus: Bus number
 * @param device: Device number
 * @param function: Function number
 * @param offset: Register offset
 * @return: 8-bit value read
 */
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

/**
 * Write a 32-bit value to PCI configuration space
 * @param bus: Bus number
 * @param device: Device number
 * @param function: Function number
 * @param offset: Register offset (must be 4-byte aligned)
 * @param value: Value to write
 */
void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

/**
 * Get PCI device information
 * @param index: Device index (0 to pci_get_device_count()-1)
 * @return: Pointer to device structure or NULL if not found
 */
pci_device_t *pci_get_device(uint8_t index);

/**
 * Get number of detected PCI devices
 * @return: Number of devices found
 */
uint8_t pci_get_device_count(void);

/**
 * Find a PCI device by vendor and device ID
 * @param vendor_id: Vendor ID to search for
 * @param device_id: Device ID to search for
 * @return: Pointer to device structure or NULL if not found
 */
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);

/**
 * Find a PCI device by class code
 * @param class_code: Class code to search for
 * @param subclass: Subclass to search for (or 0xFF for any)
 * @return: Pointer to device structure or NULL if not found
 */
pci_device_t *pci_find_class(uint8_t class_code, uint8_t subclass);

/**
 * Get class name string
 * @param class_code: PCI class code
 * @return: Human-readable class name
 */
const char *pci_class_name(uint8_t class_code);

/**
 * Print information about detected PCI devices
 */
void pci_print_info(void);

#endif /* PCI_H */
