/**
 * ISO9660 Filesystem Driver
 * CD-ROM filesystem support (ECMA-119)
 */

#include <iso9660.h>
#include <ide.h>
#include <kernel.h>
#include <string.h>
#include <vga.h>

/* Maximum cached directory entries */
#define ISO9660_MAX_CACHED_ENTRIES 64

/* Sector buffer for reading */
static uint8_t iso9660_sector_buf[ISO9660_SECTOR_SIZE];

/* Static directory entry for readdir */
static dirent_t iso9660_dirent;

/* Node cache for finddir results */
static fs_node_t iso9660_node_cache[ISO9660_MAX_CACHED_ENTRIES];
static iso9660_file_t iso9660_file_cache[ISO9660_MAX_CACHED_ENTRIES];
static int iso9660_cache_index = 0;

/* Filesystem private data */
static iso9660_fs_t iso9660_fs_data;

/* Filesystem type for registration */
static filesystem_t iso9660_fstype = {
    .name = "iso9660",
    .mount = iso9660_mount,
    .unmount = iso9660_unmount
};

/* Forward declarations */
static int iso9660_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static dirent_t *iso9660_readdir(fs_node_t *node, uint32_t index);
static fs_node_t *iso9660_finddir(fs_node_t *node, const char *name);

/**
 * Read sectors from CD-ROM
 */
static int iso9660_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, void *buffer) {
    return ide_atapi_read(drive, lba, count, buffer);
}

/**
 * Convert ISO9660 filename to normal format
 * Removes version number (;1) and trailing dots
 */
static void iso9660_parse_filename(const char *src, int len, char *dst) {
    int i, j;
    
    /* Copy characters, stopping at semicolon (version separator) */
    for (i = 0, j = 0; i < len && src[i] != ';' && j < FS_MAX_NAME - 1; i++) {
        dst[j++] = src[i];
    }
    
    /* Remove trailing dot if present */
    if (j > 0 && dst[j - 1] == '.') {
        j--;
    }
    
    dst[j] = '\0';
    
    /* Convert to lowercase for easier matching */
    for (i = 0; dst[i]; i++) {
        if (dst[i] >= 'A' && dst[i] <= 'Z') {
            dst[i] = dst[i] - 'A' + 'a';
        }
    }
}

/**
 * Compare filenames (case-insensitive)
 */
static int iso9660_compare_name(const char *name1, const char *name2) {
    while (*name1 && *name2) {
        char c1 = *name1;
        char c2 = *name2;
        
        /* Convert to lowercase */
        if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
        if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
        
        if (c1 != c2) {
            return c1 - c2;
        }
        
        name1++;
        name2++;
    }
    
    return *name1 - *name2;
}

/**
 * Allocate a node from cache
 */
static fs_node_t *iso9660_alloc_node(void) {
    fs_node_t *node = &iso9660_node_cache[iso9660_cache_index];
    iso9660_file_t *file = &iso9660_file_cache[iso9660_cache_index];
    
    iso9660_cache_index = (iso9660_cache_index + 1) % ISO9660_MAX_CACHED_ENTRIES;
    
    memset(node, 0, sizeof(fs_node_t));
    memset(file, 0, sizeof(iso9660_file_t));
    
    node->private_data = file;
    
    return node;
}

/**
 * Read file data
 */
static int iso9660_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !buffer || !node->private_data) {
        return FS_ERR_INVALID;
    }
    
    iso9660_file_t *file = (iso9660_file_t *)node->private_data;
    
    /* Check bounds */
    if (offset >= file->size) {
        return 0;
    }
    
    if (offset + size > file->size) {
        size = file->size - offset;
    }
    
    /* Calculate starting sector and offset within sector */
    uint32_t start_sector = file->lba + (offset / ISO9660_SECTOR_SIZE);
    uint32_t sector_offset = offset % ISO9660_SECTOR_SIZE;
    uint32_t bytes_read = 0;
    
    while (bytes_read < size) {
        /* Read sector */
        if (iso9660_read_sectors(iso9660_fs_data.drive, start_sector, 1, iso9660_sector_buf) != IDE_OK) {
            return FS_ERR_IO;
        }
        
        /* Calculate bytes to copy from this sector */
        uint32_t bytes_to_copy = ISO9660_SECTOR_SIZE - sector_offset;
        if (bytes_to_copy > size - bytes_read) {
            bytes_to_copy = size - bytes_read;
        }
        
        /* Copy data */
        memcpy(buffer + bytes_read, iso9660_sector_buf + sector_offset, bytes_to_copy);
        
        bytes_read += bytes_to_copy;
        start_sector++;
        sector_offset = 0;
    }
    
    return bytes_read;
}

/**
 * Read directory entry by index
 */
static dirent_t *iso9660_readdir(fs_node_t *node, uint32_t index) {
    if (!node || !node->private_data) {
        return NULL;
    }
    
    iso9660_file_t *dir = (iso9660_file_t *)node->private_data;
    
    uint32_t current_sector = dir->lba;
    uint32_t bytes_remaining = dir->size;
    uint32_t entry_index = 0;
    uint32_t sector_offset = 0;
    
    while (bytes_remaining > 0) {
        /* Read sector if needed */
        if (sector_offset == 0 || sector_offset >= ISO9660_SECTOR_SIZE) {
            if (iso9660_read_sectors(iso9660_fs_data.drive, current_sector, 1, iso9660_sector_buf) != IDE_OK) {
                return NULL;
            }
            sector_offset = 0;
            current_sector++;
        }
        
        /* Get directory entry */
        iso9660_dirent_t *entry = (iso9660_dirent_t *)(iso9660_sector_buf + sector_offset);
        
        /* Check for end of sector (zero length entry) */
        if (entry->length == 0) {
            /* Move to next sector */
            uint32_t skip = ISO9660_SECTOR_SIZE - sector_offset;
            if (skip > bytes_remaining) {
                break;
            }
            bytes_remaining -= skip;
            sector_offset = ISO9660_SECTOR_SIZE;
            continue;
        }
        
        /* Skip "." and ".." entries (single byte names) */
        if (entry->name_length == 1 && (entry->name[0] == 0 || entry->name[0] == 1)) {
            sector_offset += entry->length;
            bytes_remaining -= entry->length;
            continue;
        }
        
        /* Check if this is the entry we want */
        if (entry_index == index) {
            iso9660_parse_filename(entry->name, entry->name_length, iso9660_dirent.name);
            iso9660_dirent.inode = entry->extent_lba_le;
            return &iso9660_dirent;
        }
        
        entry_index++;
        sector_offset += entry->length;
        bytes_remaining -= entry->length;
    }
    
    return NULL;
}

/**
 * Find a file in a directory
 */
static fs_node_t *iso9660_finddir(fs_node_t *node, const char *name) {
    if (!node || !name || !node->private_data) {
        return NULL;
    }
    
    iso9660_file_t *dir = (iso9660_file_t *)node->private_data;
    
    uint32_t current_sector = dir->lba;
    uint32_t bytes_remaining = dir->size;
    uint32_t sector_offset = 0;
    
    while (bytes_remaining > 0) {
        /* Read sector if needed */
        if (sector_offset == 0 || sector_offset >= ISO9660_SECTOR_SIZE) {
            if (iso9660_read_sectors(iso9660_fs_data.drive, current_sector, 1, iso9660_sector_buf) != IDE_OK) {
                return NULL;
            }
            sector_offset = 0;
            current_sector++;
        }
        
        /* Get directory entry */
        iso9660_dirent_t *entry = (iso9660_dirent_t *)(iso9660_sector_buf + sector_offset);
        
        /* Check for end of sector */
        if (entry->length == 0) {
            uint32_t skip = ISO9660_SECTOR_SIZE - sector_offset;
            if (skip > bytes_remaining) {
                break;
            }
            bytes_remaining -= skip;
            sector_offset = ISO9660_SECTOR_SIZE;
            continue;
        }
        
        /* Parse filename */
        char parsed_name[FS_MAX_NAME];
        
        /* Handle "." and ".." */
        if (entry->name_length == 1 && entry->name[0] == 0) {
            parsed_name[0] = '.';
            parsed_name[1] = '\0';
        } else if (entry->name_length == 1 && entry->name[0] == 1) {
            parsed_name[0] = '.';
            parsed_name[1] = '.';
            parsed_name[2] = '\0';
        } else {
            iso9660_parse_filename(entry->name, entry->name_length, parsed_name);
        }
        
        /* Compare names */
        if (iso9660_compare_name(parsed_name, name) == 0) {
            /* Found it! Create a node */
            fs_node_t *found = iso9660_alloc_node();
            iso9660_file_t *file = (iso9660_file_t *)found->private_data;
            
            strcpy(found->name, parsed_name);
            found->inode = entry->extent_lba_le;
            found->length = entry->data_length_le;
            
            file->lba = entry->extent_lba_le;
            file->size = entry->data_length_le;
            file->flags = entry->flags;
            
            if (entry->flags & ISO9660_FLAG_DIRECTORY) {
                found->flags = FS_DIRECTORY;
                found->readdir = iso9660_readdir;
                found->finddir = iso9660_finddir;
            } else {
                found->flags = FS_FILE;
                found->read = iso9660_read;
            }
            
            return found;
        }
        
        sector_offset += entry->length;
        bytes_remaining -= entry->length;
    }
    
    return NULL;
}

/**
 * Initialize ISO9660 filesystem driver
 */
void iso9660_init(void) {
    iso9660_cache_index = 0;
    memset(iso9660_node_cache, 0, sizeof(iso9660_node_cache));
    memset(iso9660_file_cache, 0, sizeof(iso9660_file_cache));
    
    /* Register filesystem type */
    fs_register(&iso9660_fstype);
}

/**
 * Mount an ISO9660 filesystem from a drive
 */
fs_node_t *iso9660_mount(uint8_t drive) {
    ide_device_t *dev = ide_get_device(drive);
    
    /* Check if device exists and is ATAPI */
    if (!dev || dev->type != IDE_TYPE_ATAPI) {
        return NULL;
    }
    
    /* Read Primary Volume Descriptor (sector 16) */
    if (iso9660_read_sectors(drive, ISO9660_SYSTEM_AREA, 1, iso9660_sector_buf) != IDE_OK) {
        return NULL;
    }
    
    iso9660_pvd_t *pvd = (iso9660_pvd_t *)iso9660_sector_buf;
    
    /* Verify ISO9660 signature */
    if (pvd->type != ISO9660_VD_PRIMARY ||
        pvd->id[0] != 'C' || pvd->id[1] != 'D' ||
        pvd->id[2] != '0' || pvd->id[3] != '0' || pvd->id[4] != '1') {
        return NULL;
    }
    
    /* Extract root directory information */
    iso9660_dirent_t *root_entry = (iso9660_dirent_t *)pvd->root_dir;
    
    /* Store filesystem info */
    iso9660_fs_data.drive = drive;
    iso9660_fs_data.root_lba = root_entry->extent_lba_le;
    iso9660_fs_data.root_size = root_entry->data_length_le;
    iso9660_fs_data.block_size = pvd->logical_block_le;
    
    /* Copy volume ID */
    memcpy(iso9660_fs_data.volume_id, pvd->volume_id, 32);
    iso9660_fs_data.volume_id[32] = '\0';
    
    /* Trim trailing spaces from volume ID */
    for (int i = 31; i >= 0 && iso9660_fs_data.volume_id[i] == ' '; i--) {
        iso9660_fs_data.volume_id[i] = '\0';
    }
    
    /* Create root node */
    fs_node_t *root = iso9660_alloc_node();
    iso9660_file_t *root_file = (iso9660_file_t *)root->private_data;
    
    strcpy(root->name, "/");
    root->flags = FS_DIRECTORY;
    root->inode = iso9660_fs_data.root_lba;
    root->length = iso9660_fs_data.root_size;
    root->readdir = iso9660_readdir;
    root->finddir = iso9660_finddir;
    
    root_file->lba = iso9660_fs_data.root_lba;
    root_file->size = iso9660_fs_data.root_size;
    root_file->flags = ISO9660_FLAG_DIRECTORY;
    
    return root;
}

/**
 * Unmount an ISO9660 filesystem
 */
int iso9660_unmount(fs_node_t *root) {
    UNUSED(root);
    /* Nothing to do for read-only filesystem */
    return FS_OK;
}

/**
 * Get mounted volume ID
 */
const char *iso9660_get_volume_id(void) {
    return iso9660_fs_data.volume_id;
}
