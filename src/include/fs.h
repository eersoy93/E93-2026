/**
 * Virtual Filesystem (VFS) Header
 * Abstract filesystem interface for the kernel
 */

#ifndef FS_H
#define FS_H

#include "stdint.h"
#include "stdbool.h"

/* Maximum path length */
#define FS_MAX_PATH     256
#define FS_MAX_NAME     256     /* Supports Rock Ridge long filenames */

/* File types */
#define FS_FILE         0x01
#define FS_DIRECTORY    0x02
#define FS_CHARDEVICE   0x03
#define FS_BLOCKDEVICE  0x04
#define FS_PIPE         0x05
#define FS_SYMLINK      0x06
#define FS_MOUNTPOINT   0x08

/* File open flags */
#define FS_OPEN_READ    0x01
#define FS_OPEN_WRITE   0x02
#define FS_OPEN_APPEND  0x04
#define FS_OPEN_CREATE  0x08
#define FS_OPEN_TRUNC   0x10

/* Seek origins */
#define FS_SEEK_SET     0   /* From beginning */
#define FS_SEEK_CUR     1   /* From current position */
#define FS_SEEK_END     2   /* From end */

/* Error codes */
#define FS_OK           0
#define FS_ERR_NOTFOUND -1
#define FS_ERR_NOTDIR   -2
#define FS_ERR_ISDIR    -3
#define FS_ERR_NOSPACE  -4
#define FS_ERR_INVALID  -5
#define FS_ERR_IO       -6
#define FS_ERR_NOMEM    -7
#define FS_ERR_NOENT    -8
#define FS_ERR_EXIST    -9
#define FS_ERR_NOTMOUNT -10

/* Forward declarations */
struct fs_node;
struct dirent;

/* Filesystem operations function pointers */
typedef int (*read_fn)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef int (*write_fn)(struct fs_node *, uint32_t, uint32_t, const uint8_t *);
typedef void (*open_fn)(struct fs_node *);
typedef void (*close_fn)(struct fs_node *);
typedef struct dirent *(*readdir_fn)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_fn)(struct fs_node *, const char *);

/* Filesystem node (file/directory) */
typedef struct fs_node {
    char name[FS_MAX_NAME];     /* Filename */
    uint32_t flags;             /* Node type and flags */
    uint32_t inode;             /* Inode number */
    uint32_t length;            /* File size in bytes */
    uint32_t impl;              /* Implementation-defined */
    
    /* Filesystem operations */
    read_fn read;
    write_fn write;
    open_fn open;
    close_fn close;
    readdir_fn readdir;
    finddir_fn finddir;
    
    /* For mount points */
    struct fs_node *ptr;        /* Mounted filesystem root */
    
    /* Private data for filesystem driver */
    void *private_data;
} fs_node_t;

/* Directory entry */
typedef struct dirent {
    char name[FS_MAX_NAME];     /* Filename */
    uint32_t inode;             /* Inode number */
} dirent_t;

/* Filesystem type structure */
typedef struct filesystem {
    char name[32];              /* Filesystem name (e.g., "iso9660") */
    
    /* Mount/unmount operations */
    fs_node_t *(*mount)(uint8_t drive);
    int (*unmount)(fs_node_t *root);
} filesystem_t;

/* VFS functions */

/**
 * Initialize the virtual filesystem
 */
void fs_init(void);

/**
 * Read from a file
 * @param node: File node
 * @param offset: Byte offset to start reading
 * @param size: Number of bytes to read
 * @param buffer: Buffer to store data
 * @return Number of bytes read, or negative error code
 */
int fs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

/**
 * Write to a file
 * @param node: File node
 * @param offset: Byte offset to start writing
 * @param size: Number of bytes to write
 * @param buffer: Buffer containing data
 * @return Number of bytes written, or negative error code
 */
int fs_write(fs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer);

/**
 * Open a file
 * @param node: File node
 */
void fs_open(fs_node_t *node);

/**
 * Close a file
 * @param node: File node
 */
void fs_close(fs_node_t *node);

/**
 * Read a directory entry
 * @param node: Directory node
 * @param index: Entry index
 * @return Directory entry or NULL
 */
dirent_t *fs_readdir(fs_node_t *node, uint32_t index);

/**
 * Find a file in a directory
 * @param node: Directory node
 * @param name: Filename to find
 * @return File node or NULL
 */
fs_node_t *fs_finddir(fs_node_t *node, const char *name);

/**
 * Mount a filesystem
 * @param drive: Drive number
 * @param fstype: Filesystem type name
 * @return Root node of mounted filesystem, or NULL on error
 */
fs_node_t *fs_mount(uint8_t drive, const char *fstype);

/**
 * Register a filesystem type
 * @param fs: Filesystem structure
 * @return 0 on success, error code on failure
 */
int fs_register(filesystem_t *fs);

/**
 * Get the root filesystem node
 * @return Root node or NULL
 */
fs_node_t *fs_root(void);

/**
 * Resolve a path to a node
 * @param path: Path string (e.g., "/boot/kernel.bin")
 * @return File node or NULL
 */
fs_node_t *fs_namei(const char *path);

#endif /* FS_H */
