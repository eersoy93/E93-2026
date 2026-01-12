/**
 * Utility Functions Header
 * Provides common utility functions for userspace programs
 */

#ifndef UTILS_H
#define UTILS_H

#define UNUSED(x) (void)(x)

/**
 * Check if character is whitespace
 */
static inline int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

/**
 * Check if character is a digit
 */
static inline int isdigit(int c) {
    return c >= '0' && c <= '9';
}

/**
 * Check if character is a hexadecimal digit
 */
static inline int isxdigit(int c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/**
 * Convert hex character to its numeric value
 * @param c: Hex character ('0'-'9', 'a'-'f', 'A'-'F')
 * @return: Numeric value (0-15), or 0 if invalid
 */
static inline int hex_char_value(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

/**
 * Compare 4 hex digits from string against a value (case-insensitive)
 * @param str: String containing hex digits
 * @param val: 16-bit value to compare against
 * @return: 1 if match, 0 if no match
 */
static inline int match_hex4(const char *str, unsigned short val) {
    if (!isxdigit(str[0]) || !isxdigit(str[1]) ||
        !isxdigit(str[2]) || !isxdigit(str[3])) {
        return 0;
    }
    unsigned short file_val = (hex_char_value(str[0]) << 12) |
                              (hex_char_value(str[1]) << 8) |
                              (hex_char_value(str[2]) << 4) |
                              hex_char_value(str[3]);
    return file_val == val;
}

/**
 * Check if character is alphabetic
 */
static inline int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * Check if character is alphanumeric
 */
static inline int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

/**
 * Check if character is uppercase
 */
static inline int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

/**
 * Check if character is lowercase
 */
static inline int islower(int c) {
    return c >= 'a' && c <= 'z';
}

/**
 * Convert character to lowercase
 */
static inline int tolower(int c) {
    if (isupper(c)) return c + ('a' - 'A');
    return c;
}

/**
 * Convert character to uppercase
 */
static inline int toupper(int c) {
    if (islower(c)) return c - ('a' - 'A');
    return c;
}

/**
 * Skip whitespace in string
 * @param s: Input string
 * @return: Pointer to first non-whitespace character
 */
static inline const char *skip_whitespace(const char *s) {
    while (isspace(*s)) s++;
    return s;
}

/**
 * Convert string to lowercase (in place)
 * @param s: String to convert
 */
static inline void str_tolower(char *s) {
    while (*s) {
        *s = tolower(*s);
        s++;
    }
}

/**
 * Convert string to uppercase (in place)
 * @param s: String to convert
 */
static inline void str_toupper(char *s) {
    while (*s) {
        *s = toupper(*s);
        s++;
    }
}

/**
 * Parse integer from string
 * @param s: Input string
 * @return: Parsed integer value
 */
static inline int atoi(const char *s) {
    int result = 0;
    int sign = 1;
    
    s = skip_whitespace(s);
    
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    
    while (isdigit(*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }
    
    return sign * result;
}

/**
 * Get next word from string
 * @param s: Input string (should be result of skip_whitespace)
 * @param word: Buffer to store word
 * @param max_len: Maximum length of word buffer
 * @return: Pointer to character after word
 */
static inline const char *get_word(const char *s, char *word, int max_len) {
    int i = 0;
    s = skip_whitespace(s);
    while (*s && !isspace(*s) && i < max_len - 1) {
        word[i++] = *s++;
    }
    word[i] = '\0';
    return s;
}

/**
 * Integer to string conversion
 * @param n: Number to convert
 * @param buf: Buffer to store result (must be at least 12 bytes for base 10)
 * @param base: Number base (2-16)
 * @return: Pointer to result string
 */
static inline char *itoa(int n, char *buf, int base) {
    const char *digits = "0123456789abcdef";
    char temp[33];
    int i = 0;
    int neg = 0;
    unsigned int un;
    
    if (base < 2 || base > 16) {
        buf[0] = '\0';
        return buf;
    }
    
    if (n < 0 && base == 10) {
        neg = 1;
        un = (unsigned int)(-n);
    } else {
        un = (unsigned int)n;
    }
    
    if (un == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }
    
    while (un > 0) {
        temp[i++] = digits[un % base];
        un /= base;
    }
    
    int j = 0;
    if (neg) buf[j++] = '-';
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
    
    return buf;
}

/**
 * Unsigned integer to string conversion
 * @param n: Number to convert
 * @param buf: Buffer to store result
 * @param base: Number base (2-16)
 * @return: Pointer to result string
 */
static inline char *utoa(unsigned int n, char *buf, int base) {
    const char *digits = "0123456789abcdef";
    char temp[33];
    int i = 0;
    
    if (base < 2 || base > 16) {
        buf[0] = '\0';
        return buf;
    }
    
    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }
    
    while (n > 0) {
        temp[i++] = digits[n % base];
        n /= base;
    }
    
    int j = 0;
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
    
    return buf;
}

/**
 * Parse integer with base detection
 * Supports "0x" prefix for hex, "0b" for binary
 * @param s: Input string
 * @return: Parsed integer value
 */
static inline int parse_int(const char *s) {
    int result = 0;
    int sign = 1;
    int base = 10;
    
    s = skip_whitespace(s);
    
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    
    /* Check for base prefix */
    if (*s == '0') {
        s++;
        if (*s == 'x' || *s == 'X') {
            base = 16;
            s++;
        } else if (*s == 'b' || *s == 'B') {
            base = 2;
            s++;
        } else if (isdigit(*s)) {
            base = 8;  /* Octal */
        } else {
            return 0;
        }
    }
    
    while (*s) {
        int digit;
        if (isdigit(*s)) {
            digit = *s - '0';
        } else if (*s >= 'a' && *s <= 'f') {
            digit = *s - 'a' + 10;
        } else if (*s >= 'A' && *s <= 'F') {
            digit = *s - 'A' + 10;
        } else {
            break;
        }
        
        if (digit >= base) break;
        result = result * base + digit;
        s++;
    }
    
    return sign * result;
}

/**
 * Get absolute value
 */
static inline int abs(int n) {
    return n < 0 ? -n : n;
}

/**
 * Get minimum of two integers
 */
static inline int min(int a, int b) {
    return a < b ? a : b;
}

/**
 * Get maximum of two integers
 */
static inline int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * Clamp a value to a range
 */
static inline int clamp(int value, int min_val, int max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

#endif /* UTILS_H */
