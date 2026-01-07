/**
 * String Library Header
 * Basic string and memory manipulation functions
 */

#ifndef STRING_H
#define STRING_H

#include "../include/stddef.h"
#include "../include/stdint.h"

/* Memory functions */
void *memset(void *dest, int val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);

/* String length */
size_t strlen(const char *str);

/* String comparison */
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

/* String copy */
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

/* String concatenation */
char *strcat(char *dest, const char *src);

/* String search */
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);

#endif /* STRING_H */
