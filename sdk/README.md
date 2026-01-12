# E93-2026 User Program SDK

This SDK provides everything you need to create user programs for the E93-2026 operating system.

## Quick Start

1. Copy the template from `sdk/template/` to a new file in `src/user/programs/`
2. Rename it and modify as needed
3. Run `make iso` to build your program into the OS image

## Directory Structure

```
sdk/
├── README.md           # This file
├── docs/
│   ├── syscalls.md     # System call reference
│   ├── graphics.md     # VGA graphics programming guide
│   └── io.md           # I/O and console programming guide
└── template/
    ├── program.c       # Template for console programs
    └── graphics_demo.c # Template for graphics programs
```

## Include Files

All headers are in `src/user/include/`. Headers must be included separately based on what functions you need:

| Header | Description |
|--------|-------------|
| `user.h` | Core syscall wrappers (exit, sleep, beep, exec) |
| `utils.h` | Type definitions, utility functions (isdigit, atoi, etc.) |
| `io.h` | Console I/O (print, readline, colors) and file I/O |
| `string.h` | String manipulation functions |
| `vga_gfx.h` | VGA graphics modes and drawing |
| `vga_font_user.h` | 8x8 bitmap font for graphics mode |
| `ide.h` | IDE device information |
| `pci.h` | PCI device information |
| `version.h` | OS version information |

**Note:** Include only the headers you need. Common combinations:
- Console programs: `user.h`, `io.h`
- Graphics programs: `user.h`, `io.h`, `vga_gfx.h`
- Full shell-like programs: `user.h`, `io.h`, `string.h`, `utils.h`, `version.h`

## Program Entry Point

All programs must define a `_start` function as the entry point:

```c
#include <user.h>
#include <io.h>

void _start(void) {
    print("Hello, World!\n");
    exit(0);
}
```

## Building

Programs are automatically built when you run:

```bash
make iso        # Build ISO with all user programs
make run        # Build and run in QEMU
```

Each `.c` file in `src/user/programs/` becomes a separate program.

## Memory Layout

- Programs are loaded at virtual address `0x400000`
- Stack is set up by the kernel before program execution
- No heap allocation is currently available

## Examples

See the existing programs in `src/user/programs/`:

- `hello.c` - Simple hello world
- `shell.c` - Interactive shell with commands
- `vga_demo_12h.c` - VGA 640x480 16-color demo
- `vga_demo_13h.c` - VGA 320x200 256-color demo
- `vga_demo_mode_x.c` - VGA Mode X 320x240 256-color demo
- `vga_demo_mode_y.c` - VGA Mode Y 320x200 256-color planar demo
