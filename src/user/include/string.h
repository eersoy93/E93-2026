/**
 * String Library Header
 * Provides string and memory manipulation functions for userspace programs
 */

#ifndef USER_STRING_H
#define USER_STRING_H

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

/* Type definitions */
typedef unsigned int size_t;

/**
 * String length
 */
static inline size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/**
 * String compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/**
 * String compare with length limit
 */
static inline int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/**
 * String copy
 */
static inline char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

/**
 * String copy with length limit
 */
static inline char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dest;
}

/**
 * String concatenate
 */
static inline char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

/**
 * String concatenate with length limit
 */
static inline char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    while (n-- && (*d++ = *src++));
    *d = '\0';
    return dest;
}

/**
 * Find character in string
 * @return pointer to first occurrence, or NULL if not found
 */
static inline char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return (c == '\0') ? (char *)s : (char *)0;
}

/**
 * Find character in string (reverse)
 * @return pointer to last occurrence, or NULL if not found
 */
static inline char *strrchr(const char *s, int c) {
    const char *last = (char *)0;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (c == '\0') ? (char *)s : (char *)last;
}

/**
 * Memory set
 */
static inline void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

/**
 * Memory copy
 */
static inline void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

/**
 * Memory move (handles overlapping regions)
 */
static inline void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

/**
 * Memory compare
 */
static inline int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/**
 * Find substring in string
 * @return pointer to first occurrence, or NULL if not found
 */
static inline char *strstr(const char *haystack, const char *needle) {
    size_t needle_len;
    if (!*needle) return (char *)haystack;
    needle_len = strlen(needle);
    while (*haystack) {
        if (strncmp(haystack, needle, needle_len) == 0) {
            return (char *)haystack;
        }
        haystack++;
    }
    return (char *)0;
}

/**
 * Duplicate a string (static buffer, not thread-safe)
 * @param s: String to duplicate
 * @return: Pointer to duplicate (static buffer)
 * Note: Returns pointer to internal buffer, overwritten on next call
 */
static inline char *strdup_static(const char *s) {
    static char buf[256];
    size_t len = strlen(s);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

/**
 * Calculate string span (characters in accept)
 * @return length of initial segment containing only chars from accept
 */
static inline size_t strspn(const char *s, const char *accept) {
    size_t count = 0;
    while (*s) {
        const char *a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found) break;
        count++;
        s++;
    }
    return count;
}

/**
 * Calculate string complement span (characters not in reject)
 * @return length of initial segment containing no chars from reject
 */
static inline size_t strcspn(const char *s, const char *reject) {
    size_t count = 0;
    while (*s) {
        const char *r = reject;
        while (*r) {
            if (*s == *r) return count;
            r++;
        }
        count++;
        s++;
    }
    return count;
}

#endif /* USER_STRING_H */
