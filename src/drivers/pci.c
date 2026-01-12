/**
 * PCI (Peripheral Component Interconnect) Driver
 * PCI bus enumeration and device access
 */

#include <pci.h>
#include <ports.h>
#include <vga.h>

/* Detected PCI devices */
static pci_device_t pci_devices[PCI_MAX_DEVICES];

/* Number of detected devices */
static uint8_t pci_device_count = 0;

/**
 * Build PCI configuration address
 */
static uint32_t pci_make_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (uint32_t)((1 << 31) |                   /* Enable bit */
                      ((uint32_t)bus << 16) |       /* Bus number */
                      ((uint32_t)device << 11) |    /* Device number */
                      ((uint32_t)function << 8) |   /* Function number */
                      (offset & 0xFC));             /* Register offset (aligned) */
}

/**
 * Read a 32-bit value from PCI configuration space
 */
uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_make_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

/**
 * Read a 16-bit value from PCI configuration space
 */
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t value = pci_config_read32(bus, device, function, offset & ~3);
    return (uint16_t)((value >> ((offset & 2) * 8)) & 0xFFFF);
}

/**
 * Read an 8-bit value from PCI configuration space
 */
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t value = pci_config_read32(bus, device, function, offset & ~3);
    return (uint8_t)((value >> ((offset & 3) * 8)) & 0xFF);
}

/**
 * Write a 32-bit value to PCI configuration space
 */
void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = pci_make_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/**
 * Check if a device/function exists and add it to our list
 */
static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
    
    if (vendor_id == PCI_VENDOR_NONE || vendor_id == 0) {
        return;
    }
    
    if (pci_device_count >= PCI_MAX_DEVICES) {
        return;
    }
    
    pci_device_t *dev = &pci_devices[pci_device_count];
    
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->present = 1;
    dev->vendor_id = vendor_id;
    dev->device_id = pci_config_read16(bus, device, function, PCI_DEVICE_ID);
    dev->class_code = pci_config_read8(bus, device, function, PCI_CLASS);
    dev->subclass = pci_config_read8(bus, device, function, PCI_SUBCLASS);
    dev->prog_if = pci_config_read8(bus, device, function, PCI_PROG_IF);
    dev->revision = pci_config_read8(bus, device, function, PCI_REVISION_ID);
    dev->header_type = pci_config_read8(bus, device, function, PCI_HEADER_TYPE) & 0x7F;
    dev->irq = pci_config_read8(bus, device, function, PCI_INTERRUPT_LINE);
    
    /* Read BARs for standard header type */
    if (dev->header_type == PCI_HEADER_TYPE_NORMAL) {
        for (int i = 0; i < 6; i++) {
            dev->bar[i] = pci_config_read32(bus, device, function, PCI_BAR0 + (i * 4));
        }
    } else {
        for (int i = 0; i < 6; i++) {
            dev->bar[i] = 0;
        }
    }
    
    pci_device_count++;
}

/**
 * Check a device and all its functions
 */
static void pci_check_device(uint8_t bus, uint8_t device) {
    uint16_t vendor_id = pci_config_read16(bus, device, 0, PCI_VENDOR_ID);
    
    if (vendor_id == PCI_VENDOR_NONE || vendor_id == 0) {
        return;
    }
    
    /* Check function 0 */
    pci_check_function(bus, device, 0);
    
    /* Check if multi-function device */
    uint8_t header_type = pci_config_read8(bus, device, 0, PCI_HEADER_TYPE);
    if (header_type & PCI_HEADER_TYPE_MULTI_FUNC) {
        /* Check remaining functions */
        for (uint8_t function = 1; function < 8; function++) {
            vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
            if (vendor_id != PCI_VENDOR_NONE && vendor_id != 0) {
                pci_check_function(bus, device, function);
            }
        }
    }
}

/**
 * Scan a PCI bus
 */
static void pci_scan_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

/**
 * Initialize PCI bus and enumerate devices
 */
void pci_init(void) {
    pci_device_count = 0;
    
    /* Clear device list */
    for (int i = 0; i < PCI_MAX_DEVICES; i++) {
        pci_devices[i].present = 0;
    }
    
    /* Check if PCI is supported by reading from bus 0, device 0 */
    uint32_t tmp = pci_config_read32(0, 0, 0, 0);
    if (tmp == 0xFFFFFFFF) {
        /* No PCI bus present */
        return;
    }
    
    /* Check header type of host controller to determine scanning method */
    uint8_t header_type = pci_config_read8(0, 0, 0, PCI_HEADER_TYPE);
    
    if ((header_type & PCI_HEADER_TYPE_MULTI_FUNC) == 0) {
        /* Single PCI host controller - scan bus 0 only for direct devices,
         * but also scan other buses as devices might be on different buses */
        for (uint16_t bus = 0; bus < 256; bus++) {
            pci_scan_bus((uint8_t)bus);
        }
    } else {
        /* Multiple PCI host controllers */
        for (uint8_t function = 0; function < 8; function++) {
            uint16_t vendor_id = pci_config_read16(0, 0, function, PCI_VENDOR_ID);
            if (vendor_id == PCI_VENDOR_NONE) {
                break;
            }
            pci_scan_bus(function);
        }
    }
}

/**
 * Get PCI device information
 */
pci_device_t *pci_get_device(uint8_t index) {
    if (index >= pci_device_count) {
        return (pci_device_t *)0;
    }
    return &pci_devices[index];
}

/**
 * Get number of detected PCI devices
 */
uint8_t pci_get_device_count(void) {
    return pci_device_count;
}

/**
 * Find a PCI device by vendor and device ID
 */
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id && 
            pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return (pci_device_t *)0;
}

/**
 * Find a PCI device by class code
 */
pci_device_t *pci_find_class(uint8_t class_code, uint8_t subclass) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].class_code == class_code) {
            if (subclass == 0xFF || pci_devices[i].subclass == subclass) {
                return &pci_devices[i];
            }
        }
    }
    return (pci_device_t *)0;
}

/**
 * Get class name string
 */
const char *pci_class_name(uint8_t class_code) {
    switch (class_code) {
        case PCI_CLASS_UNCLASSIFIED:      return "Unclassified";
        case PCI_CLASS_STORAGE:           return "Storage";
        case PCI_CLASS_NETWORK:           return "Network";
        case PCI_CLASS_DISPLAY:           return "Display";
        case PCI_CLASS_MULTIMEDIA:        return "Multimedia";
        case PCI_CLASS_MEMORY:            return "Memory";
        case PCI_CLASS_BRIDGE:            return "Bridge";
        case PCI_CLASS_COMMUNICATION:     return "Communication";
        case PCI_CLASS_SYSTEM:            return "System";
        case PCI_CLASS_INPUT:             return "Input";
        case PCI_CLASS_DOCKING:           return "Docking";
        case PCI_CLASS_PROCESSOR:         return "Processor";
        case PCI_CLASS_SERIAL_BUS:        return "Serial Bus";
        case PCI_CLASS_WIRELESS:          return "Wireless";
        case PCI_CLASS_INTELLIGENT_IO:    return "Intelligent I/O";
        case PCI_CLASS_SATELLITE:         return "Satellite";
        case PCI_CLASS_ENCRYPTION:        return "Encryption";
        case PCI_CLASS_SIGNAL_PROCESSING: return "Signal Processing";
        default:                          return "Unknown";
    }
}

/**
 * Print a hex digit
 */
static void print_hex_digit(uint8_t val) {
    if (val < 10) {
        vga_putchar('0' + val);
    } else {
        vga_putchar('a' + val - 10);
    }
}

/**
 * Print a 16-bit hex value
 */
static void print_hex16(uint16_t val) {
    print_hex_digit((val >> 12) & 0xF);
    print_hex_digit((val >> 8) & 0xF);
    print_hex_digit((val >> 4) & 0xF);
    print_hex_digit(val & 0xF);
}

/**
 * Print information about detected PCI devices
 */
void pci_print_info(void) {
    for (int i = 0; i < pci_device_count; i++) {
        pci_device_t *dev = &pci_devices[i];
        
        /* Print bus:device.function */
        vga_print("  ");
        vga_putchar('0' + (dev->bus / 10));
        vga_putchar('0' + (dev->bus % 10));
        vga_print(":");
        vga_putchar('0' + (dev->device / 10));
        vga_putchar('0' + (dev->device % 10));
        vga_print(".");
        vga_putchar('0' + dev->function);
        vga_print(" ");
        
        /* Print vendor:device IDs */
        print_hex16(dev->vendor_id);
        vga_print(":");
        print_hex16(dev->device_id);
        vga_print(" ");
        
        /* Print class */
        vga_print("[");
        vga_print(pci_class_name(dev->class_code));
        vga_print("]\n");
    }
}
