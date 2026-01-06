# E93-2026
The new E93! E93-2026!

## Overview

A minimal 32-bit operating system kernel template that boots via GRUB2 (Multiboot specification). Written in C and x86 Assembly.

## Prerequisites

- nasm
- qemu (x86 system)
- GRUB2
- xorriso
- mtools
- GCC
- GCC cross compiler (optionally)

## Building

### Using System GCC (Easier)
```bash
make -f Makefile.gcc
```

### Using Cross-Compiler (Recommended)
```bash
make
```

### Build ISO Image
```bash
make iso    # or: make -f Makefile.gcc iso
```

## Running

### Direct Kernel Boot (QEMU)
```bash
make run    # or: make -f Makefile.gcc run
```

### Boot from ISO (GRUB2)
```bash
make run-iso    # or: make -f Makefile.gcc run-iso
```

## Copyright and License

Copyright (c) 2026 Erdem Ersoy (eersoy93). Licensed with Apache License 2.0. See the LICENSE file for license text.

E93-2026 written with the help of GitHub Copilot!
