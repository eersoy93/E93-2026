# System Call Reference

System calls are the interface between user programs and the kernel. They are invoked using interrupt `0x80`.

## System Call Convention

- **EAX**: System call number
- **EBX**: First argument
- **ECX**: Second argument
- **EDX**: Third argument
- **Return value**: In EAX

## Core System Calls

### SYS_EXIT (0)
Exit the current program.

```c
void exit(int code);
```

**Arguments:**
- `code`: Exit code (0 = success)

**Returns:** Does not return

---

### SYS_WRITE (1)
Write to a file descriptor.

```c
int write(int fd, const char *buf, int len);
```

**Arguments:**
- `fd`: File descriptor (STDOUT=1, STDERR=2)
- `buf`: Buffer to write
- `len`: Number of bytes

**Returns:** Number of bytes written

---

### SYS_READ (2)
Read a line from stdin with editing support.

```c
int readline(char *buf, int max_len);
```

**Arguments:**
- `buf`: Buffer to store input
- `max_len`: Maximum length

**Returns:** Number of characters read, -1 on Ctrl+C

---

### SYS_SLEEP (5)
Sleep for a duration.

```c
void sleep(int ms);
```

**Arguments:**
- `ms`: Milliseconds to sleep

---

### SYS_BEEP (6)
Play a sound through the PC speaker.

```c
void beep(int freq, int duration);
```

**Arguments:**
- `freq`: Frequency in Hz
- `duration`: Duration in milliseconds

---

### SYS_GETCHAR (7)
Read a single character (blocking).

```c
int getchar(void);
```

**Returns:** ASCII code of character pressed

---

### SYS_EXEC (8)
Execute another program.

```c
int exec(const char *path);
```

**Arguments:**
- `path`: Path to program (e.g., "/user/hello")

**Returns:** -1 on error, does not return on success

---

### SYS_READDIR (9)
Read directory entries.

```c
int readdir(const char *path, char *buf, int max_entries);
```

**Arguments:**
- `path`: Directory path
- `buf`: Buffer for entries (32 bytes each)
- `max_entries`: Maximum entries to read

**Returns:** Number of entries read

---

### SYS_CLEAR (10)
Clear the screen.

```c
void clear(void);
```

---

### SYS_SETCOLOR (11)
Set text foreground and background colors.

```c
void setcolor(int fg, int bg);
```

**Arguments:**
- `fg`: Foreground color (0-15)
- `bg`: Background color (0-15)

## VGA Graphics System Calls

See [graphics.md](graphics.md) for detailed VGA programming documentation.

### SYS_VGA_INIT (14)
Initialize VGA mode 12h (640x480, 16 colors).

### SYS_VGA_INIT_13H (21)
Initialize VGA mode 13h (320x200, 256 colors).

### SYS_VGA_INIT_X (22)
Initialize VGA Mode X (320x240, 256 colors, planar).

### SYS_VGA_INIT_Y (24)
Initialize VGA Mode Y (320x200, 256 colors, planar).

### SYS_VGA_EXIT (15)
Exit graphics mode and return to text mode.

### SYS_VGA_CLEAR (16)
Clear the graphics screen.

### SYS_VGA_PIXEL (17)
Draw a single pixel.

### SYS_VGA_LINE (18)
Draw a line between two points.

### SYS_VGA_RECT (19)
Draw a rectangle (outline or filled).

### SYS_VGA_CIRCLE (20)
Draw a circle (outline or filled).

### SYS_VGA_PALETTE (23)
Set a palette color (256-color modes only).

## Color Constants

```c
/* Text mode colors */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15
```

## Making Raw System Calls

If you need to make a system call directly:

```c
static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return ret;
}
```
