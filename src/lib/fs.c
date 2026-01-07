/**
 * Virtual Filesystem (VFS) Implementation
 * Abstract filesystem interface for the kernel
 */

#include "fs.h"
#include "kernel.h"
#include "string.h"
#include "vga.h"

/* Maximum number of registered filesystems */
#define MAX_FILESYSTEMS 8

/* Registered filesystems */
static filesystem_t *filesystems[MAX_FILESYSTEMS];
static int fs_count = 0;

/* Root filesystem node */
static fs_node_t *fs_root_node = NULL;

/**
 * Initialize the virtual filesystem
 */
void fs_init(void) {
    fs_count = 0;
    fs_root_node = NULL;
    memset(filesystems, 0, sizeof(filesystems));
}

/**
 * Read from a file
 */
int fs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !buffer) {
        return FS_ERR_INVALID;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    if (node->read) {
        return node->read(node, offset, size, buffer);
    }
    
    return FS_ERR_INVALID;
}

/**
 * Write to a file
 */
int fs_write(fs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    if (!node || !buffer) {
        return FS_ERR_INVALID;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    if (node->write) {
        return node->write(node, offset, size, buffer);
    }
    
    return FS_ERR_INVALID;
}

/**
 * Open a file
 */
void fs_open(fs_node_t *node) {
    if (!node) {
        return;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    if (node->open) {
        node->open(node);
    }
}

/**
 * Close a file
 */
void fs_close(fs_node_t *node) {
    if (!node) {
        return;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    if (node->close) {
        node->close(node);
    }
}

/**
 * Read a directory entry
 */
dirent_t *fs_readdir(fs_node_t *node, uint32_t index) {
    if (!node) {
        return NULL;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    /* Must be a directory */
    if ((node->flags & 0x07) != FS_DIRECTORY) {
        return NULL;
    }
    
    if (node->readdir) {
        return node->readdir(node, index);
    }
    
    return NULL;
}

/**
 * Find a file in a directory
 */
fs_node_t *fs_finddir(fs_node_t *node, const char *name) {
    if (!node || !name) {
        return NULL;
    }
    
    /* Follow mount points */
    if ((node->flags & FS_MOUNTPOINT) && node->ptr) {
        node = node->ptr;
    }
    
    /* Must be a directory */
    if ((node->flags & 0x07) != FS_DIRECTORY) {
        return NULL;
    }
    
    if (node->finddir) {
        return node->finddir(node, name);
    }
    
    return NULL;
}

/**
 * Register a filesystem type
 */
int fs_register(filesystem_t *fs) {
    if (!fs || fs_count >= MAX_FILESYSTEMS) {
        return FS_ERR_INVALID;
    }
    
    filesystems[fs_count++] = fs;
    return FS_OK;
}

/**
 * Find a registered filesystem by name
 */
static filesystem_t *fs_find(const char *name) {
    for (int i = 0; i < fs_count; i++) {
        if (strcmp(filesystems[i]->name, name) == 0) {
            return filesystems[i];
        }
    }
    return NULL;
}

/**
 * Mount a filesystem
 */
fs_node_t *fs_mount(uint8_t drive, const char *fstype) {
    filesystem_t *fs = fs_find(fstype);
    if (!fs) {
        return NULL;
    }
    
    if (!fs->mount) {
        return NULL;
    }
    
    fs_node_t *root = fs->mount(drive);
    if (root && !fs_root_node) {
        /* First mount becomes root filesystem */
        fs_root_node = root;
    }
    
    return root;
}

/**
 * Get the root filesystem node
 */
fs_node_t *fs_root(void) {
    return fs_root_node;
}

/**
 * Resolve a path to a node
 */
fs_node_t *fs_namei(const char *path) {
    if (!path || !fs_root_node) {
        return NULL;
    }
    
    /* Handle root path */
    if (path[0] == '/' && path[1] == '\0') {
        return fs_root_node;
    }
    
    /* Start from root */
    fs_node_t *current = fs_root_node;
    
    /* Skip leading slash */
    if (*path == '/') {
        path++;
    }
    
    /* Parse path components */
    char component[FS_MAX_NAME];
    
    while (*path && current) {
        /* Extract next path component */
        int i = 0;
        while (*path && *path != '/' && i < FS_MAX_NAME - 1) {
            component[i++] = *path++;
        }
        component[i] = '\0';
        
        /* Skip trailing slashes */
        while (*path == '/') {
            path++;
        }
        
        /* Handle special cases */
        if (strcmp(component, ".") == 0) {
            /* Current directory - do nothing */
            continue;
        }
        
        if (strcmp(component, "..") == 0) {
            /* Parent directory - not fully implemented yet */
            continue;
        }
        
        /* Find component in current directory */
        current = fs_finddir(current, component);
    }
    
    return current;
}
