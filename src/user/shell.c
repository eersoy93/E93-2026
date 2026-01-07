/**
 * E93-2026 Shell
 * A basic command-line shell for userspace
 * 
 * Built-in commands:
 *   help    - Show available commands
 *   ls      - List directory contents
 *   clear   - Clear the screen
 *   echo    - Print text
 *   beep    - Play a beep sound
 *   exit    - Exit shell (halt system)
 * 
 * External commands:
 *   Any executable in /programs/ can be run by name
 */

#include "user.h"

/* Maximum command line length */
#define CMD_MAX_LEN     256
#define MAX_ARGS        16

/* Current working directory */
static char cwd[CMD_MAX_LEN] = "/programs";

/* Command buffer */
static char cmd_buf[CMD_MAX_LEN];

/**
 * Print shell prompt
 */
static void print_prompt(void) {
    setcolor(COLOR_LIGHT_GREEN, COLOR_BLACK);
    print("E93-2026");
    setcolor(COLOR_WHITE, COLOR_BLACK);
    print("> ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
}

/**
 * Print a number
 */
static void print_num(int n) {
    char buf[16];
    int i = 0;
    int neg = 0;
    
    if (n < 0) {
        neg = 1;
        n = -n;
    }
    
    if (n == 0) {
        putchar('0');
        return;
    }
    
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    if (neg) putchar('-');
    while (i > 0) putchar(buf[--i]);
}

/**
 * Skip whitespace in string
 */
static const char *skip_whitespace(const char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

/**
 * Get the next word from string, returns pointer to end
 */
static const char *get_word(const char *s, char *word, int max_len) {
    int i = 0;
    s = skip_whitespace(s);
    while (*s && *s != ' ' && *s != '\t' && i < max_len - 1) {
        word[i++] = *s++;
    }
    word[i] = '\0';
    return s;
}

/**
 * Convert string to lowercase (in place)
 */
static void to_lower(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') {
            *s = *s + ('a' - 'A');
        }
        s++;
    }
}

/**
 * Built-in: help
 */
static void cmd_help(void) {
    print("\n");
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("E93-2026 Shell Commands:\n");
    print("------------------------\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  help          ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Show this help message\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  ls [dir]      ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- List directory contents\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  clear         ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Clear the screen\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  echo <text>   ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Print text to screen\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  beep          ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Play a beep sound\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  exit          ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Exit shell and halt system\n");
    print("\n");
    setcolor(COLOR_DARK_GREY, COLOR_BLACK);
    print("Run programs from /programs/ by name (e.g., 'hello')\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("\n");
}

/**
 * Built-in: ls
 */
static void cmd_ls(const char *path) {
    char entry[256];
    int index = 0;
    int count = 0;
    
    if (!path || !*path) {
        path = cwd;
    }
    
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("Contents of ");
    setcolor(COLOR_WHITE, COLOR_BLACK);
    print(path);
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print(":\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    
    while (1) {
        int ret = readdir(path, index, entry);
        if (ret <= 0) break;
        
        /* Skip . and .. */
        if (strcmp(entry, ".") != 0 && strcmp(entry, "..") != 0) {
            print("  ");
            setcolor(COLOR_LIGHT_GREEN, COLOR_BLACK);
            print(entry);
            setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
            print("\n");
            count++;
        }
        index++;
    }
    
    if (count == 0) {
        setcolor(COLOR_DARK_GREY, COLOR_BLACK);
        print("  (empty)\n");
        setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    }
    
    setcolor(COLOR_DARK_GREY, COLOR_BLACK);
    print_num(count);
    print(" file(s)\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
}

/**
 * Built-in: clear
 */
static void cmd_clear(void) {
    clear();
}

/**
 * Built-in: echo
 */
static void cmd_echo(const char *text) {
    if (text && *text) {
        print(text);
    }
    print("\n");
}

/**
 * Built-in: beep
 */
static void cmd_beep(void) {
    beep(1000, 100);
}

/**
 * Try to execute an external program
 */
static int try_exec(const char *name) {
    char path[CMD_MAX_LEN];
    
    /* Build full path */
    strcpy(path, "/programs/");
    strcat(path, name);
    
    /* Try to execute */
    int ret = exec(path);
    
    /* If exec returns, it failed */
    return ret;
}

/**
 * Process a command line
 */
static void process_command(char *line) {
    char cmd[CMD_MAX_LEN];
    const char *rest;
    
    /* Skip leading whitespace */
    rest = skip_whitespace(line);
    
    /* Empty line */
    if (!*rest) return;
    
    /* Get command word */
    rest = get_word(rest, cmd, sizeof(cmd));
    rest = skip_whitespace(rest);
    
    /* Convert command to lowercase for comparison */
    to_lower(cmd);
    
    /* Check built-in commands */
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        cmd_help();
    }
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) {
        cmd_ls(rest);
    }
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        cmd_clear();
    }
    else if (strcmp(cmd, "echo") == 0) {
        cmd_echo(rest);
    }
    else if (strcmp(cmd, "beep") == 0) {
        cmd_beep();
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        print("Goodbye!\n");
        exit(0);
    }
    else {
        /* Try to run as external program */
        if (try_exec(cmd) < 0) {
            setcolor(COLOR_LIGHT_RED, COLOR_BLACK);
            print("Unknown command: ");
            setcolor(COLOR_WHITE, COLOR_BLACK);
            print(cmd);
            setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
            print("\nType 'help' for available commands.\n");
        }
    }
}

/**
 * Shell entry point
 */
void _start(void) {
    /* Print welcome shell banner */
    clear();
    print("\n");
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("  ______  ___  _____       ___   ___ ___   __\n");
    print(" |  ____|/ _ \\|___ /      |__ \\ / _ \\__ \\ / /\n");
    print(" | |__  | (_) | |_ \\ ______  ) | | | | ) / /_\n");
    print(" |  __| \\__, |___) |______|/ /| | | |/ / '_ \\\n");
    print(" | |____  / /|__ /       / /_| |_| / /| (_) |\n");
    print(" |______|/_/ |___/      |____|\\___/____\\___/\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("\n");
    setcolor(COLOR_WHITE, COLOR_BLACK);
    print("Welcome to E93-2026 Shell!\n");
    setcolor(COLOR_DARK_GREY, COLOR_BLACK);
    print("Type 'help' for available commands.\n\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    
    /* Main shell loop */
    while (1) {
        print_prompt();
        
        int len = readline(cmd_buf, CMD_MAX_LEN);
        
        if (len < 0) {
            /* Ctrl+C pressed */
            print("\n");
            continue;
        }
        
        if (len > 0) {
            process_command(cmd_buf);
        }
    }
}
