#include "shell.h"
#include "msgs.h"
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_LENGTH 10

char *shellCommand = "$ ";
char *history[MAX_LENGTH];
int history_total_count = 0;
int history_size = 0;

int tokenize(char *input, char **commands) {
  char *saveptr;
  int commandsIndex = 0;
  char *token = strtok_r(input, " ", &saveptr);
  while (token != NULL) {
    commands[commandsIndex] = token;
    commandsIndex++;
    token = strtok_r(NULL, " ", &saveptr);
  }
  commands[commandsIndex] = NULL;
  return commandsIndex;
}

bool remove_ampersand(char *input) {
  char *ampersand = strchr(input, '&');
  if (ampersand != NULL) {
    *ampersand = '\0';
    return true;
  }
  return false;
}

char *getUserInput(char *buffer) {
  size_t len = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
  if (len == -1) {
    char *msg = FORMAT_MSG("shell", READ_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  } else {
    buffer[len - 1] = '\0';
  }
  return buffer;
}

void display_directory() {
  char buffer[BUFFER_SIZE];
  if (getcwd(buffer, sizeof(buffer)) == NULL) {
    char *msg = FORMAT_MSG("shell", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  } else {
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, shellCommand, strlen(shellCommand));
  }
}

void printArray(char **array, int length) {
  for (int i = 0; i < length; i++) {
    printf("%s\n", array[i]);
  }
  printf("size: %d\n", length);
}

void execute(char **commandArgs, bool background, bool isInternal) {
  if (isInternal == true) {
    return;
  }
  pid_t pid = fork();
  if (pid) {
    if (background == false) {
      int status = 0;
      if (waitpid(pid, &status, 0) == -1) {
        char *msg = FORMAT_MSG("shell", WAIT_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
      }
    }
  } else if (pid == 0) {
    if (execvp(commandArgs[0], commandArgs) == -1) {
      char *msg = FORMAT_MSG("shell", EXEC_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    }
  } else {
    char *msg = FORMAT_MSG("shell", FORK_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  }
}

void clean_zombies() {
  int status;
  while (waitpid(-1, &status, WNOHANG) > 0)
    ;
}

void exit_case(int size) {
  if (size > 1) {
    char *msg = FORMAT_MSG("exit", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  } else {
    exit(0);
  }
}

void pwd_case(int size) {
  if (size > 1) {
    char *msg = FORMAT_MSG("pwd", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  char directory[BUFFER_SIZE];
  if (getcwd(directory, BUFFER_SIZE) == NULL) {
    char *msg = FORMAT_MSG("pwd", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  } else {
    write(STDOUT_FILENO, directory, strlen(directory));
    write(STDOUT_FILENO, "\n", strlen("\n"));
  }
}

void cd_case(char **command, int size) {
  if (size > 2) {
    char *msg = FORMAT_MSG("cd", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  }

  struct passwd *uidPass = getpwuid(getuid());
  char currentDirectory[BUFFER_SIZE];
  static char prevDirectory[BUFFER_SIZE];
  getcwd(currentDirectory, BUFFER_SIZE);

  if (size < 2 || strcmp(command[1], "~") == 0) {
    if (chdir(uidPass->pw_dir) == -1) {
      char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else {
      strcpy(prevDirectory, currentDirectory);
    }
    return;
  }

  if (strchr(command[1], '~') != NULL) {
    char path[1024];
    snprintf(path, sizeof(path), "%s%s", uidPass->pw_dir, command[1] + 1);
    if (chdir(path) == -1) {
      char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    }
    strcpy(prevDirectory, path);
  } else if (strchr(command[1], '-')) {
    if (chdir(prevDirectory) == -1) {
      char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else {
      strcpy(prevDirectory, currentDirectory);
    }
  } else {
    if (chdir(command[1]) == -1) {
      char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else {
      strcpy(prevDirectory, currentDirectory);
    }
  }
}

void write_cmd_msg(char *cmd, char *inMsg) {
  char msg[BUFFER_SIZE];
  snprintf(msg, sizeof(msg), "%s: %s\n", cmd, inMsg);
  write(STDOUT_FILENO, msg, strlen(msg));
}

void display_help_cmds() {
  write_cmd_msg("exit", EXIT_HELP_MSG);
  write_cmd_msg("pwd", PWD_HELP_MSG);
  write_cmd_msg("cd", CD_HELP_MSG);
  write_cmd_msg("help", HELP_HELP_MSG);
  write_cmd_msg("history", HISTORY_HELP_MSG);
}

void help_case(char **command, int size) {
  if (size > 2) {
    char *msg = FORMAT_MSG("help", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  if (size < 2) {
    display_help_cmds();
  } else if (strcmp(command[1], "exit") == 0) {
    write_cmd_msg("exit", EXIT_HELP_MSG);
  } else if (strcmp(command[1], "pwd") == 0) {
    write_cmd_msg("pwd", PWD_HELP_MSG);
  } else if (strcmp(command[1], "cd") == 0) {
    write_cmd_msg("cd", CD_HELP_MSG);
  } else if (strcmp(command[1], "help") == 0) {
    write_cmd_msg("help", HELP_HELP_MSG);
  } else if (strcmp(command[1], "history") == 0) {
    write_cmd_msg("history", HISTORY_HELP_MSG);
  } else {
    write_cmd_msg(command[1], EXTERN_HELP_MSG);
  }
}

void remove_oldest_history() {
  if (history_size > 0) {
    free(history[0]);
    for (int i = 1; i < history_size; i++) {
      history[i - 1] = history[i];
    }
    history_size--;
  }
}

void add_to_history(char *input) {
  if (history_size >= MAX_LENGTH) {
    remove_oldest_history();
  }
  history[history_size] = strdup(input);
  history_total_count++;
  history_size++;
}

void print_history() {
  for (int i = 0; i < history_size; i++) {
    int cmd_num = history_total_count - i - 1;
    int index = history_size - 1 - i;
    char msg[BUFFER_SIZE]; // - 1 as max = 10, index [0,9]
    snprintf(msg, sizeof(msg), "%d\t%s\n", cmd_num, history[index]);
    write(STDOUT_FILENO, msg, strlen(msg));
  }
}

bool is_digit(char *input) {
  for (int i = 0; i < strlen(input); i++) {
    if (!isdigit(input[i])) {
      return false;
    }
  }
  return true;
}

void run_cmd_num(char **command) {
  char *exclamationPtr = strchr(command[0], '!');
  char value[BUFFER_SIZE];
  snprintf(value, sizeof(value), "%s", exclamationPtr + 1);

  if (!is_digit(value)) {
    char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  int cmdNum = atoi(value);
  int min = history_total_count - history_size;
  int max = history_total_count - 1;

  if (cmdNum >= min && cmdNum <= max) {
    int index = cmdNum - min;
    char *his_command = history[index];

    char his_copy[BUFFER_SIZE];
    strncpy(his_copy, his_command, BUFFER_SIZE);
    his_copy[BUFFER_SIZE - 1] = '\0';

    add_to_history(his_copy);
    write(STDOUT_FILENO, his_copy, strlen(his_copy));
    write(STDOUT_FILENO, "\n", 1);

    char *commands[BUFFER_SIZE];
    bool background = remove_ampersand(his_copy);
    tokenize(his_copy, commands);

    execute(commands, background, false);
  } else {
    char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }
}

void run_last_history() {
  if (history_size == 0) {
    char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }
  int index = history_size - 1;

  char his_copy[BUFFER_SIZE];
  strncpy(his_copy, history[index], BUFFER_SIZE);
  his_copy[BUFFER_SIZE - 1] = '\0';

  add_to_history(his_copy);
  write(STDOUT_FILENO, his_copy, strlen(his_copy));
  write(STDOUT_FILENO, "\n", 1);

  char *commands[BUFFER_SIZE];
  bool background = remove_ampersand(his_copy);
  tokenize(his_copy, commands);

  execute(commands, background, false);
}

bool internal_commands(char **command, int size) { // size includes NULL
  if (size == 0) {
    return false;
  } else if (strcmp(command[0], "exit") == 0) {
    exit_case(size);
    return true;
  } else if (strcmp(command[0], "pwd") == 0) {
    pwd_case(size);
    return true;
  } else if (strcmp(command[0], "cd") == 0) {
    cd_case(command, size);
    return true;
  } else if (strcmp(command[0], "help") == 0) {
    help_case(command, size);
    return true;
  } else if (strcmp(command[0], "history") == 0) {
    print_history();
    return true;
  } else if (strcmp(command[0], "!!") == 0) {
    run_last_history();
    return true;
  } else if (strchr(command[0], '!') != NULL) {
    run_cmd_num(command);
    return true;
  } else {
    return false;
  }
}

void handle_signal(int sig) { display_help_cmds(); }

int main() {
  struct sigaction handler;
  handler.sa_handler = handle_signal;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;
  if (sigaction(SIGINT, &handler, NULL) == -1) {
    char *errMsg = strerror(errno);
    write(STDERR_FILENO, errMsg, strlen(errMsg));
  }

  while (true) {
    clean_zombies();
    char buffer[BUFFER_SIZE];
    char *commands[BUFFER_SIZE];
    bool background = false;
    bool internal = false;

    display_directory();

    char *input = getUserInput(buffer);

    if (strcmp(input, "!!") != 0 && strchr(input, '!') == NULL) {
      add_to_history(input);
    }

    background = remove_ampersand(input);

    int commandsIndex = tokenize(input, commands);

    internal = internal_commands(commands, commandsIndex);
    if (commands[0] != NULL) {
      execute(commands, background, internal);
    }
  }
}
