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
 *   run     - Run a program
 *   exit    - Exit shell (halt system)
 */

#include <user.h>

/* Maximum command line length */
#define CMD_MAX_LEN     256
#define MAX_ARGS        16

/* Current working directory */
static char cwd[CMD_MAX_LEN] = "/user";

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
    print("  pwd           ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Print working directory\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  cd <dir>      ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Change directory\n");
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
    print("  run <program> ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Run a program from /user/\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  version       ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Show version information\n");
    setcolor(COLOR_YELLOW, COLOR_BLACK);
    print("  exit          ");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print("- Exit shell and halt system\n");
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
    print_int(count);
    print(" file(s)\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
}

/**
 * Built-in: pwd
 */
static void cmd_pwd(void) {
    print_color(cwd, COLOR_WHITE, COLOR_BLACK);
    newline();
}

/**
 * Built-in: cd
 */
static void cmd_cd(const char *path) {
    char entry[256];
    char new_path[CMD_MAX_LEN];
    
    if (!path || !*path) {
        /* cd with no args goes to root */
        strcpy(cwd, "/");
        return;
    }
    
    /* Handle absolute vs relative paths */
    if (path[0] == '/') {
        /* Absolute path */
        strcpy(new_path, path);
    } else if (strcmp(path, "..") == 0) {
        /* Go up one directory */
        strcpy(new_path, cwd);
        /* Find last slash */
        int len = strlen(new_path);
        if (len > 1) {
            /* Remove trailing slash if present */
            if (new_path[len-1] == '/') {
                new_path[len-1] = '\0';
                len--;
            }
            /* Find previous slash */
            while (len > 0 && new_path[len-1] != '/') {
                len--;
            }
            if (len == 0) {
                new_path[0] = '/';
                len = 1;
            }
            new_path[len] = '\0';
        }
    } else {
        /* Relative path */
        strcpy(new_path, cwd);
        int len = strlen(new_path);
        if (len > 1 || new_path[0] != '/') {
            strcat(new_path, "/");
        }
        strcat(new_path, path);
    }
    
    /* Verify directory exists by trying to read it */
    if (readdir(new_path, 0, entry) >= 0) {
        strcpy(cwd, new_path);
    } else {
        print_error("Directory not found: ");
        println(path);
    }
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
        println(text);
    } else {
        newline();
    }
}

/**
 * Built-in: beep
 */
static void cmd_beep(void) {
    beep(1000, 100);
}

/**
 * Built-in: version
 */
static void cmd_version(void) {
    print("\n");
    setcolor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print("E93-2026 ");
    setcolor(COLOR_WHITE, COLOR_BLACK);
    print(VERSION);
    print("\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print(COPYRIGHT_TEXT);
    print("\n");
    print(LICENSE_TEXT);
    print("\n\n");
}

/**
 * Built-in: run
 */
static void cmd_run(const char *name) {
    char path[CMD_MAX_LEN];
    
    if (!name || !*name) {
        print_error("Usage: run <program>\n");
        return;
    }
    
    /* Build full path */
    strcpy(path, "/user/");
    strcat(path, name);
    
    /* Try to execute */
    if (exec(path) < 0) {
        print_error("Program not found: ");
        println(name);
    }
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
    str_tolower(cmd);
    
    /* Check built-in commands */
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        cmd_help();
    }
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) {
        cmd_ls(rest);
    }
    else if (strcmp(cmd, "pwd") == 0) {
        cmd_pwd();
    }
    else if (strcmp(cmd, "cd") == 0) {
        cmd_cd(rest);
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
    else if (strcmp(cmd, "version") == 0 || strcmp(cmd, "ver") == 0) {
        cmd_version();
    }
    else if (strcmp(cmd, "run") == 0) {
        cmd_run(rest);
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        println("Goodbye!");
        exit(0);
    }
    else {
        print_error("Unknown command: ");
        println(cmd);
        println("Type 'help' for available commands.");
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
    print("Welcome to E93-2026 " VERSION "!\n");
    setcolor(COLOR_LIGHT_GREY, COLOR_BLACK);
    print(COPYRIGHT_TEXT "\n");
    print(LICENSE_TEXT "\n");
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
