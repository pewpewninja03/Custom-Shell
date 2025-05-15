#ifndef SHELL_H
#define SHELL_H

#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

// Tokenizing and parsing
int tokenize(char *input, char **commands);
bool remove_ampersand(char *input);

// Input handling
char *getUserInput(char *buffer);

// Directory commands
void display_directory(void);
void pwd_case(int size);
void cd_case(char **command, int size);

// Command execution
void execute(char **commandArgs, bool background, bool isInternal);
bool internal_commands(char **command, int size);

// Built-in command handlers
void exit_case(int size);
void help_case(char **command, int size);
void display_help_cmds(void);

// History management
void add_to_history(char *input);
void remove_oldest_history(void);
void print_history(void);
void run_cmd_num(char **command);
void run_last_history(void);

// Utility functions
bool is_digit(char *input);
void printArray(char **array, int length);
void clean_zombies(void);
void write_cmd_msg(char *cmd, char *inMsg);

// Signal handling
void handle_signal(int sig);

#endif // SHELL_H
