# I/O and Console Programming Guide

This guide covers console input/output programming for E93-2026 user programs.

## Basic Output

### Print String

```c
#include <syscall.h>
#include <io.h>

void _start(void) {
    print("Hello, World!\n");
    exit(0);
}
```

### Print Character

```c
putchar('A');
putchar('\n');
```

### Write to File Descriptor

```c
write(STDOUT, "Hello", 5);  /* Write 5 bytes to stdout */
write(STDERR, "Error!", 6); /* Write to stderr */
```

## Formatted Output

The SDK provides `printf`-style formatting:

```c
print_int(42);           /* Print integer */
print_hex(0xFF);         /* Print hexadecimal */
println("Message");      /* Print with newline */
```

### Manual Number Formatting

```c
static void print_number(int n) {
    char buf[16];
    itoa(n, buf, 10);
    print(buf);
}
```

## Input

### Read Single Character

```c
int ch = getchar();  /* Blocking read */
```

### Read Line with Editing

```c
char buffer[128];
int len = readline(buffer, sizeof(buffer));

if (len < 0) {
    print("Cancelled (Ctrl+C)\n");
} else {
    print("You entered: ");
    print(buffer);
    print("\n");
}
```

The `readline` function supports:
- Backspace editing
- Ctrl+C to cancel (returns -1)
- Enter to submit

## Screen Control

### Clear Screen

```c
clear();
```

### Set Colors

```c
setcolor(COLOR_WHITE, COLOR_BLUE);  /* White on blue */
print("Colored text!");
setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);  /* Reset to default */
```

### Available Colors

```c
/* Standard colors */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7

/* Bright colors */
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

/* Semantic aliases */
#define COLOR_ERROR   COLOR_LIGHT_RED
#define COLOR_INFO    COLOR_LIGHT_CYAN
#define COLOR_NORMAL  COLOR_LIGHT_GREY
#define COLOR_SUCCESS COLOR_LIGHT_GREEN
#define COLOR_WARNING COLOR_YELLOW
```

## Directory Operations

### List Directory Contents

```c
char entry[256];
int i = 0;

while (readdir("/user", i, entry) > 0) {
    print(entry);
    print("\n");
    i++;
}
```

## Program Execution

### Execute Another Program

```c
int result = exec("/user/hello");
if (result < 0) {
    print("Failed to execute program\n");
}
/* If exec succeeds, this code never runs */
```

## File I/O

### Open, Read, and Close a File

```c
int fd = fopen("/media/pci.ids");
if (fd < 0) {
    print("Failed to open file\n");
} else {
    int size = fsize(fd);
    char buf[1024];
    int bytes = fread(fd, buf, sizeof(buf) - 1);
    buf[bytes] = '\0';
    print(buf);
    fclose(fd);
}
```

### Convenience Function

```c
char buf[4096];
int bytes = read_file("/media/pci.ids", buf, sizeof(buf));
if (bytes > 0) {
    buf[bytes] = '\0';
    print(buf);
}
```

## Timing

### Sleep

```c
sleep(1000);  /* Sleep for 1 second */
```

### Beep

```c
beep(440, 500);  /* Play A4 (440 Hz) for 500ms */
beep(880, 250);  /* Play A5 (880 Hz) for 250ms */
```

## String Utilities

All string functions from `string.h` are available:

```c
/* Length */
size_t len = strlen("hello");

/* Compare */
if (strcmp(s1, s2) == 0) { /* equal */ }
if (strncmp(s1, s2, 5) == 0) { /* first 5 chars equal */ }

/* Copy */
strcpy(dest, src);
strncpy(dest, src, max_len);

/* Concatenate */
strcat(dest, src);
strncat(dest, src, max_len);

/* Search */
char *p = strchr(str, 'x');  /* Find character */
char *p = strstr(str, "sub"); /* Find substring */

/* Memory */
memcpy(dest, src, n);
memset(buf, 0, sizeof(buf));
int cmp = memcmp(a, b, n);
```

## Character Utilities

From `utils.h`:

```c
/* Character classification */
isspace(' ')   /* true */
isdigit('5')   /* true */
isalpha('A')   /* true */
isalnum('x')   /* true */
isupper('Z')   /* true */
islower('a')   /* true */

/* Character conversion */
tolower('A')   /* 'a' */
toupper('b')   /* 'B' */

/* String conversion */
str_tolower(s);  /* Convert in place */
str_toupper(s);  /* Convert in place */
```

## Number Parsing

```c
/* String to integer */
int n = atoi("123");

/* Integer to string */
char buf[16];
itoa(42, buf, 10);    /* Decimal: "42" */
itoa(255, buf, 16);   /* Hex: "ff" */

/* Parse with base detection */
int n = parse_int("0xFF");  /* Hex prefix */
int n = parse_int("123");   /* Decimal */
```

## Hardware Information

### Memory Information

```c
#include <syscall.h>

mem_info_t info;
if (get_mem_info(&info) == 0) {
    print("Lower memory: ");
    print_int(info.mem_lower);
    print(" KB\n");
    print("Upper memory: ");
    print_int(info.mem_upper);
    print(" KB\n");
    print("Total memory: ");
    print_int(info.total_kb);
    print(" KB\n");
}
```

### IDE Devices

```c
#include <ide.h>

/* Get number of IDE drives */
int count = ide_get_drive_count();

/* Get information about each drive */
ide_device_info_t info;
for (int i = 0; i < count; i++) {
    if (ide_get_device_info(i, &info) == 0) {
        print("Drive: ");
        print(info.model);
        print("\n");
    }
}
```

### PCI Devices

```c
#include <pci.h>

/* Get number of PCI devices */
int count = pci_get_device_count();

/* Get information about each device */
pci_device_info_t info;
for (int i = 0; i < count; i++) {
    if (pci_get_device_info(i, &info) == 0) {
        print("Device: ");
        print(pci_class_name(info.class_code));
        print("\n");
    }
}

/* Look up device names from pci.ids file */
char vendor_name[64], device_name[64];
pci_lookup_ids(info.vendor_id, info.device_id, vendor_name, device_name, sizeof(vendor_name));
```

## Example: Interactive Program

```c
#include <syscall.h>
#include <io.h>

void _start(void) {
    char name[64];
    
    clear();
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("Welcome to my program!\n\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    
    print("What is your name? ");
    if (readline(name, sizeof(name)) < 0) {
        print("\nGoodbye!\n");
        exit(1);
    }
    
    print("\nHello, ");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print(name);
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("!\n");
    
    beep(880, 100);
    
    print("\nPress any key to exit...");
    getchar();
    
    exit(0);
}
```
