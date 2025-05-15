# Shell Project in C

![Shell](https://img.shields.io/badge/language-C-blue?style=flat) 

---

## Overview

This project is a **Unix shell** implemented in C. It demonstrates my proficiency in systems programming and process control. The shell supports:

- Tokenizing and parsing user input into commands and arguments
- Executing both internal (built-in) and external commands
- Managing background and foreground processes
- Maintaining a history of commands with the ability to rerun previous commands
- Handling Unix signals (e.g., SIGINT) gracefully
- Providing useful built-in commands such as `cd`, `pwd`, `exit`, `help`, and `history`

---

## Features

- **Command Tokenization:** Splits input strings into executable commands and arguments.
- **Background Process Execution:** Supports running commands in the background using `&`.
- **Internal Command Handling:** Implements built-in shell commands without spawning new processes.
- **History Management:** Keeps track of previous commands, with support for `!!` and `!n` syntax to rerun commands.
- **Signal Handling:** Captures and handles Ctrl+C (SIGINT) to display helpful information instead of exiting abruptly.
- **Error Handling:** Provides descriptive error messages for common issues such as invalid commands or directory changes.
- **Modular Design:** Functions are organized into clear sections for parsing, command execution, history management, and utilities.

---

## Technical Highlights

- Use of **POSIX APIs** such as `fork()`, `execvp()`, `waitpid()`, `getcwd()`, and signal handling.
- Careful **memory management** for command history using dynamic allocation and cleanup.
- Robust **string parsing** with `strtok_r()` and manual input manipulation.
- Clear separation of **internal vs external commands**.
- Use of **standard C libraries** and POSIX-compliant system calls.

---

## Code Organization

- **shell.h:** Contains function declarations, macros, and extern global variables.
- **shell.c:** Main loop, signal setup, and orchestrates command execution.

---

## How to Use

1. Clone the repository:
   ```bash
   git clone https://github.com/pewpewninja03/Custom-Shell.git

2. Create the build folder and enter the directory
   ```bash
   mkdir build
   cd build

3. Run Cmake and make
   ```bash
   cd build
   cmake ..
   make

3. Run the shell executable
   ```bash
   ./shell

