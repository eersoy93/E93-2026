/**
 * Version Header
 * Provides version and copyright information for userspace programs
 */

#ifndef USER_VERSION_H
#define USER_VERSION_H

#define OS_VERSION_MAJOR 0
#define OS_VERSION_MINOR 2
#define OS_VERSION_PATCH 0

#define VERSION STRINGIFY(OS_VERSION_MAJOR) "." STRINGIFY(OS_VERSION_MINOR) "." STRINGIFY(OS_VERSION_PATCH)

#define COPYRIGHT_YEAR 2026
#define COPYRIGHT_TEXT "Copyright (c) " STRINGIFY(COPYRIGHT_YEAR) " Erdem Ersoy (eersoy93)"
#define LICENSE_TEXT "Licensed under the Apache License 2.0."

#endif /* USER_VERSION_H */