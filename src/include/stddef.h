/**
 * Standard Definitions
 * Freestanding implementation for 32-bit x86
 */

#ifndef STDDEF_H
#define STDDEF_H

/* size_t - unsigned integer type for sizes */
typedef unsigned int size_t;

/* ssize_t - signed integer type for sizes */
typedef signed int ssize_t;

/* ptrdiff_t - signed integer type for pointer differences */
typedef signed int ptrdiff_t;

/* NULL pointer constant */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* offsetof macro - offset of member in struct */
#define offsetof(type, member) ((size_t)&((type *)0)->member)

#endif /* STDDEF_H */
