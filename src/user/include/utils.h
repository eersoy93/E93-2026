/**
 * Utility Functions Header
 * Provides common utility functions for userspace programs
 */

#ifndef UTILS_H
#define UTILS_H

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

#endif /* UTILS_H */
