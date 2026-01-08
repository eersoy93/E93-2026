/**
 * Version Header
 * Provides version information for userspace programs
 */

#ifndef VERSION_H
#define VERSION_H

#define OS_VERSION_MAJOR 0
#define OS_VERSION_MINOR 1
#define OS_VERSION_PATCH 0

#define VERSION STRINGIFY(OS_VERSION_MAJOR) "." STRINGIFY(OS_VERSION_MINOR) "." STRINGIFY(OS_VERSION_PATCH)

#endif /* VERSION_H */
