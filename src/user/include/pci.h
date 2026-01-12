/**
 * PCI Devices Library Header
 * Provides PCI device information for userspace programs
 */

#ifndef USER_PCI_H
#define USER_PCI_H

#include <io.h>
#include <utils.h>

/* System call number for PCI info */
#define SYS_PCIINFO 26

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

/* PCI device info structure (matches kernel layout) */
typedef struct {
    unsigned char bus;          /* Bus number (0-255) */
    unsigned char device;       /* Device number (0-31) */
    unsigned char function;     /* Function number (0-7) */
    unsigned char present;      /* Device is present */
    unsigned short vendor_id;   /* Vendor ID */
    unsigned short device_id;   /* Device ID */
    unsigned char class_code;   /* Class code */
    unsigned char subclass;     /* Subclass */
    unsigned char prog_if;      /* Programming interface */
    unsigned char revision;     /* Revision ID */
    unsigned char header_type;  /* Header type */
    unsigned char irq;          /* IRQ line */
} pci_device_info_t;

/**
 * Get number of PCI devices
 * @return: Number of devices detected
 */
static inline int pci_get_device_count(void) {
    return _io_syscall(SYS_PCIINFO, 0xFF, 0, 0);
}

/**
 * Get PCI device information
 * @param index: Device index (0 to pci_get_device_count()-1)
 * @param info: Pointer to pci_device_info_t structure to fill
 * @return: 0 on success, -1 if device not found
 */
static inline int pci_get_device_info(int index, pci_device_info_t *info) {
    return _io_syscall(SYS_PCIINFO, index, (int)info, 0);
}

/**
 * Get class name string
 * @param class_code: PCI class code
 * @return: Human-readable class name
 */
static inline const char *pci_class_name(unsigned char class_code) {
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

/* ============================================
 * PCI IDs Database Lookup Functions
 * ============================================ */

#define PCI_IDS_PATH "/media/pci.ids"
#define PCI_IDS_BUF_SIZE 8192
#define PCI_LINE_BUF_SIZE 256

/* Static buffers for PCI IDs parsing */
static char _pci_ids_buf[PCI_IDS_BUF_SIZE];
static char _pci_line_buf[PCI_LINE_BUF_SIZE];

/**
 * Look up a vendor name from pci.ids database
 * @param vendor_id: Vendor ID to search for
 * @param name_buf: Buffer to store the name (at least 64 bytes)
 * @return: 1 if found, 0 if not found
 */
static inline int pci_lookup_vendor(unsigned short vendor_id, char *name_buf) {
    int fd = fopen(PCI_IDS_PATH);
    if (fd < 0) {
        return 0;
    }
    
    int found = 0;
    int line_pos = 0;
    int bytes_read;
    
    while ((bytes_read = fread(fd, _pci_ids_buf, PCI_IDS_BUF_SIZE - 1)) > 0) {
        _pci_ids_buf[bytes_read] = '\0';
        
        for (int i = 0; i < bytes_read; i++) {
            char c = _pci_ids_buf[i];
            
            if (c == '\n' || c == '\r') {
                /* End of line - check if this is our vendor */
                _pci_line_buf[line_pos] = '\0';
                
                /* Vendor lines start with 4 hex digits followed by 2 spaces */
                if (line_pos >= 6 && 
                    match_hex4(_pci_line_buf, vendor_id) &&
                    _pci_line_buf[4] == ' ' && _pci_line_buf[5] == ' ') {
                    
                    /* Found! Copy the name */
                    int name_idx = 0;
                    int j = 6;
                    while (_pci_line_buf[j] && name_idx < 63) {
                        name_buf[name_idx++] = _pci_line_buf[j++];
                    }
                    name_buf[name_idx] = '\0';
                    found = 1;
                    break;
                }
                
                line_pos = 0;
            } else if (line_pos < PCI_LINE_BUF_SIZE - 1) {
                _pci_line_buf[line_pos++] = c;
            }
        }
        
        if (found) break;
    }
    
    fclose(fd);
    return found;
}

/**
 * Look up a device name from pci.ids database
 * @param vendor_id: Vendor ID
 * @param device_id: Device ID to search for
 * @param name_buf: Buffer to store the name (at least 64 bytes)
 * @return: 1 if found, 0 if not found
 */
static inline int pci_lookup_device(unsigned short vendor_id, unsigned short device_id, char *name_buf) {
    int fd = fopen(PCI_IDS_PATH);
    if (fd < 0) {
        return 0;
    }
    
    int found = 0;
    int in_vendor_section = 0;
    int found_vendor = 0;
    int line_pos = 0;
    int bytes_read;
    
    while ((bytes_read = fread(fd, _pci_ids_buf, PCI_IDS_BUF_SIZE - 1)) > 0) {
        _pci_ids_buf[bytes_read] = '\0';
        
        for (int i = 0; i < bytes_read; i++) {
            char c = _pci_ids_buf[i];
            
            if (c == '\n' || c == '\r') {
                /* End of line - process it */
                _pci_line_buf[line_pos] = '\0';
                
                if (line_pos > 0) {
                    /* Check if this is our vendor line (starts with vendor ID) */
                    if (_pci_line_buf[0] != '\t' && _pci_line_buf[0] != '#' &&
                        line_pos >= 6 &&
                        match_hex4(_pci_line_buf, vendor_id) &&
                        _pci_line_buf[4] == ' ' && _pci_line_buf[5] == ' ') {
                        in_vendor_section = 1;
                        found_vendor = 1;
                    }
                    /* Check if we've hit another vendor (line starts with hex, not tab) */
                    else if (in_vendor_section && 
                             _pci_line_buf[0] != '\t' && 
                             _pci_line_buf[0] != '#' &&
                             isxdigit(_pci_line_buf[0])) {
                        /* New vendor section started, stop searching */
                        in_vendor_section = 0;
                        break;
                    }
                    /* Check for device line (starts with single tab + hex) */
                    else if (in_vendor_section && 
                             _pci_line_buf[0] == '\t' && 
                             _pci_line_buf[1] != '\t' &&  /* Not subsystem (double tab) */
                             line_pos >= 7 &&
                             match_hex4(_pci_line_buf + 1, device_id) &&
                             _pci_line_buf[5] == ' ' && _pci_line_buf[6] == ' ') {
                        
                        /* Found! Copy the name */
                        int name_idx = 0;
                        int j = 7;
                        while (_pci_line_buf[j] && name_idx < 63) {
                            name_buf[name_idx++] = _pci_line_buf[j++];
                        }
                        name_buf[name_idx] = '\0';
                        found = 1;
                        break;
                    }
                }
                
                line_pos = 0;
            } else if (line_pos < PCI_LINE_BUF_SIZE - 1) {
                _pci_line_buf[line_pos++] = c;
            }
        }
        
        /* Stop if found, or if we were in vendor section and left it */
        if (found) break;
        if (found_vendor && !in_vendor_section) break;
    }
    
    fclose(fd);
    return found;
}

#endif /* USER_PCI_H */
