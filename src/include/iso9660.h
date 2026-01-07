/**
 * ISO9660 Filesystem Driver Header
 * CD-ROM filesystem support (ECMA-119)
 */

#ifndef ISO9660_H
#define ISO9660_H

#include "stdint.h"
#include "fs.h"

/* ISO9660 constants */
#define ISO9660_SECTOR_SIZE     2048
#define ISO9660_SYSTEM_AREA     16      /* First 16 sectors reserved */

/* Volume descriptor types */
#define ISO9660_VD_BOOT         0       /* Boot record */
#define ISO9660_VD_PRIMARY      1       /* Primary volume descriptor */
#define ISO9660_VD_SUPPLEMENTARY 2      /* Supplementary volume descriptor */
#define ISO9660_VD_PARTITION    3       /* Volume partition descriptor */
#define ISO9660_VD_TERMINATOR   255     /* Volume descriptor set terminator */

/* Directory entry flags */
#define ISO9660_FLAG_HIDDEN     0x01    /* Hidden file */
#define ISO9660_FLAG_DIRECTORY  0x02    /* Directory */
#define ISO9660_FLAG_ASSOCIATED 0x04    /* Associated file */
#define ISO9660_FLAG_EXTENDED   0x08    /* Extended attribute record */
#define ISO9660_FLAG_PERMS      0x10    /* Permissions in extended attr */
#define ISO9660_FLAG_NOTFINAL   0x80    /* Not the final directory entry */

/* ISO9660 date/time structure (7 bytes) */
typedef struct {
    uint8_t years_since_1900;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int8_t  gmt_offset;         /* 15 minute intervals from GMT */
} __attribute__((packed)) iso9660_datetime_t;

/* ISO9660 directory entry (variable length, minimum 33 bytes) */
typedef struct {
    uint8_t  length;            /* Length of directory entry */
    uint8_t  ext_attr_length;   /* Extended attribute length */
    uint32_t extent_lba_le;     /* Location of extent (LBA) - little endian */
    uint32_t extent_lba_be;     /* Location of extent (LBA) - big endian */
    uint32_t data_length_le;    /* Data length - little endian */
    uint32_t data_length_be;    /* Data length - big endian */
    iso9660_datetime_t datetime;/* Recording date and time */
    uint8_t  flags;             /* File flags */
    uint8_t  unit_size;         /* File unit size (interleaved) */
    uint8_t  gap_size;          /* Interleave gap size */
    uint16_t vol_seq_le;        /* Volume sequence number - LE */
    uint16_t vol_seq_be;        /* Volume sequence number - BE */
    uint8_t  name_length;       /* Length of file identifier */
    char     name[1];           /* File identifier (variable length) */
} __attribute__((packed)) iso9660_dirent_t;

/* ISO9660 Primary Volume Descriptor */
typedef struct {
    uint8_t  type;              /* Volume descriptor type (1) */
    char     id[5];             /* "CD001" */
    uint8_t  version;           /* Version (1) */
    uint8_t  unused1;
    char     system_id[32];     /* System identifier */
    char     volume_id[32];     /* Volume identifier */
    uint8_t  unused2[8];
    uint32_t volume_space_le;   /* Volume space size - LE */
    uint32_t volume_space_be;   /* Volume space size - BE */
    uint8_t  unused3[32];
    uint16_t volume_set_le;     /* Volume set size - LE */
    uint16_t volume_set_be;     /* Volume set size - BE */
    uint16_t volume_seq_le;     /* Volume sequence number - LE */
    uint16_t volume_seq_be;     /* Volume sequence number - BE */
    uint16_t logical_block_le;  /* Logical block size - LE */
    uint16_t logical_block_be;  /* Logical block size - BE */
    uint32_t path_table_size_le;/* Path table size - LE */
    uint32_t path_table_size_be;/* Path table size - BE */
    uint32_t path_table_lba_le; /* L path table location */
    uint32_t opt_path_table_le; /* Optional L path table location */
    uint32_t path_table_lba_be; /* M path table location */
    uint32_t opt_path_table_be; /* Optional M path table location */
    uint8_t  root_dir[34];      /* Root directory entry */
    char     volume_set_id[128];/* Volume set identifier */
    char     publisher_id[128]; /* Publisher identifier */
    char     preparer_id[128];  /* Data preparer identifier */
    char     application_id[128];/* Application identifier */
    char     copyright_file[37];/* Copyright file identifier */
    char     abstract_file[37]; /* Abstract file identifier */
    char     biblio_file[37];   /* Bibliographic file identifier */
    char     creation_date[17]; /* Volume creation date/time */
    char     modification_date[17];/* Volume modification date/time */
    char     expiration_date[17];/* Volume expiration date/time */
    char     effective_date[17];/* Volume effective date/time */
    uint8_t  file_structure_ver;/* File structure version (1) */
    uint8_t  unused4;
    uint8_t  application_data[512];/* Application use */
    uint8_t  reserved[653];     /* Reserved */
} __attribute__((packed)) iso9660_pvd_t;

/* ISO9660 filesystem private data */
typedef struct {
    uint8_t     drive;          /* IDE drive number */
    uint32_t    root_lba;       /* Root directory LBA */
    uint32_t    root_size;      /* Root directory size */
    uint16_t    block_size;     /* Logical block size */
    char        volume_id[33];  /* Volume identifier */
} iso9660_fs_t;

/* ISO9660 file private data */
typedef struct {
    uint32_t    lba;            /* Starting LBA */
    uint32_t    size;           /* File size */
    uint8_t     flags;          /* File flags */
} iso9660_file_t;

/* Function declarations */

/**
 * Initialize ISO9660 filesystem driver
 */
void iso9660_init(void);

/**
 * Mount an ISO9660 filesystem from a drive
 * @param drive: IDE drive number
 * @return Root node or NULL on error
 */
fs_node_t *iso9660_mount(uint8_t drive);

/**
 * Unmount an ISO9660 filesystem
 * @param root: Root node
 * @return 0 on success, error code on failure
 */
int iso9660_unmount(fs_node_t *root);

#endif /* ISO9660_H */
