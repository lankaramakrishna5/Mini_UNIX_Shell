# Mini Unix Shell — Project Problem Statement

## 1. Project Overview

This project involves building a simplified Unix-like shell in C++ that runs on Linux and uses the operating system's process-management APIs directly. The objective is not to reproduce the full behaviour of Bash, but to implement the core ideas behind a command interpreter: parsing user input, spawning processes, redirecting input and output, chaining commands with pipes, handling background jobs, and managing signals safely.

The shell will act as a lightweight user interface between the user and the Linux kernel. It will accept commands, interpret them, and execute programs through system calls such as fork(), execvp(), waitpid(), pipe(), dup2(), and open().

## 2. Problem Statement

A Unix shell provides a command-line interface that allows users to interact with the operating system by running programs, redirecting data streams, and controlling job execution. Although modern shells are highly feature-rich, the core of shell behaviour is relatively small and elegant: read a line of text, parse tokens, create child processes, and replace those child processes with the requested executable.

The goal of this project is to design and implement a minimal but functional Unix-like shell from scratch in C/C++ using Linux system calls. The shell should support interactive command execution, built-in commands, standard process execution, I/O redirection, pipes, and process control.

This project is intended to deepen practical understanding of operating systems concepts such as:

- Processes and process lifecycle
- File descriptors and redirection
- Inter-process communication through pipes
- Signals and signal handlers
- Child process reaping and zombie prevention
- Shell parsing and command execution flow

## 3. Objectives

The project should help the student achieve the following:

- Understand how a shell interacts with the kernel and the operating system
- Create and manage child processes using fork() and execvp()
- Execute external programs without using system()
- Implement essential built-in commands like cd, pwd, and exit
- Support input and output redirection using file descriptors
- Support pipes so that commands can exchange output streams
- Handle background processes and prevent zombie processes
- Use signal handling to manage Ctrl+C and child termination cleanly
- Build robust error handling for invalid commands and failed system calls
- Produce code that is understandable, modular, and interview-ready

## 4. Functional Requirements

### 4.1 Interactive Shell Prompt

The program should run in an interactive loop and print a prompt such as:

```bash
myshell>
```

It should accept commands repeatedly until the user types:

```bash
exit
```

### 4.2 External Command Execution

The shell must be able to run standard Linux commands such as:

```bash
ls
pwd
date
whoami
cat file.txt
mkdir test
```

The implementation must use process creation and program execution APIs such as:

- fork()
- execvp()
- waitpid()

It must not rely on system() for normal command execution.

### 4.3 Built-in Commands

The shell must support these built-ins:

- cd — change the current working directory of the shell
- pwd — display the shell's current directory
- exit — terminate the shell session

Important: cd must be handled in the shell process rather than in a child process, otherwise the directory change would not persist in the current shell session.

### 4.4 Input Redirection

The shell must support input redirection using <.

Example:

```bash
myshell> cat < input.txt
```

This must be implemented using open(), dup2(), and close().

### 4.5 Output Redirection

The shell must support output redirection using >.

Example:

```bash
myshell> ls > output.txt
```

The output file should be overwritten if it already exists.

### 4.6 Append Redirection

The shell must support append redirection using >>.

Example:

```bash
myshell> ls >> output.txt
```

New output must be appended to existing file content without overwriting it.

### 4.7 Pipes

The shell must support Unix pipes using |.

Example:

```bash
myshell> ls | grep cpp
```

The output of one process becomes the input of the next.

The implementation should support:

- single pipes
- multiple pipes in a sequence

Example:

```bash
myshell> cat file.txt | grep hello | wc -l
```

### 4.8 Background Execution

The shell must support a background operator &:

Example:

```bash
myshell> sleep 10 &
[1] 12345
myshell>
```

The shell should return to the prompt immediately and not wait for the background process to finish. Background child processes must be reaped to avoid zombie processes.

### 4.9 Signal Handling

The shell should handle basic Unix signals, especially:

- SIGINT
- SIGCHLD

Example:

```bash
myshell> sleep 20
^C
myshell>
```

Ctrl+C should interrupt the foreground process without unnecessarily terminating the shell itself.

### 4.10 Error Handling

The shell must handle invalid commands and malformed input gracefully. Examples include:

```bash
myshell> abcxyz
myshell> cd nonexistent
myshell> cat < missing.txt
myshell> ls |
myshell> ls >
```

The shell should print meaningful error messages and continue running.

## 5. Command Parsing Requirements

The parser should separate command parsing from execution. It should identify the following tokens in the user's input:

- command names
- arguments
- |
- <
- >
- > >
- &

Example:

```bash
cat file.txt | grep hello > result.txt &
```

This should be parsed into:

- Command 1: cat file.txt
- Pipe: |
- Command 2: grep hello
- Redirection: > result.txt
- Background execution: &

The parser should produce a structured representation of the command line that the executor can then act on.

## 6. Expected Architecture

The implementation should follow a modular design resembling this:

```text
User
  |
  v
main.cpp
  |
  v
Parser
  |
  v
ParsedLine / Command structure
  |
  +---------------------------+
  |                           |
  v                           v
Built-in handling         External command execution
  |                           |
  |                           v
  |                     fork() + execvp()
  |                           |
  |                    waitpid() / signal handling
  |
  +---------------------------+
```

Suggested files may include:

- src/main.cpp
- src/parser.cpp
- src/parser.h
- src/executor.cpp
- src/executor.h
- src/builtins.cpp
- src/builtins.h
- src/signals.cpp
- src/signals.h

## 7. Core Linux APIs

The project should rely on the following operating system interfaces:

- fork() — create a child process
- execvp() — execute a program in the child process
- waitpid() — wait for child termination
- pipe() — create IPC channel between processes
- dup2() — duplicate file descriptors for redirection
- open() — open files for read/write
- close() — close file descriptors
- chdir() — change the current directory
- getcwd() — get the current directory
- sigaction() — handle signals in a structured way

## 8. Suggested Development Phases

### Phase 1 — Basic shell

- prompt loop
- command reading
- parsing
- exec of simple external commands
- cd, pwd, exit

### Phase 2 — Redirection

- <
- >
- > >
- file descriptor management

### Phase 3 — Pipes

- single-pipe behavior
- multi-pipe behavior

### Phase 4 — Background jobs

- &
- asynchronous execution
- child reaping

### Phase 5 — Signal handling

- SIGINT
- SIGCHLD
- safe shell behaviour under interrupts

### Phase 6 — Testing and documentation

- validate commands, invalid input, redirection, pipes, and background jobs
- document architecture and learning outcomes

## 9. Example Final Interaction

```bash
$ ./myshell

myshell> pwd
/home/user/mini-unix-shell

myshell> ls

myshell> cd src
myshell> pwd

myshell> ls | grep cpp

myshell> ls > files.txt
myshell> cat files.txt

myshell> echo hello >> files.txt
myshell> cat files.txt

myshell> sleep 10 &
[1] 18452

myshell> pwd

myshell> sleep 20
^C

myshell> exit
$
```

## 10. Constraints

- The implementation must be written in C or C++.
- It should run in a Linux/Unix environment; Windows users may use WSL2 + Ubuntu.
- It should not use system() for command execution.
- The shell must use Linux system calls directly where appropriate.
- The goal is to understand operating system mechanisms, not to perfectly clone Bash.
- The implementation should remain compatible with standard GCC compiler settings used in academic labs.

## 11. Expected Learning Outcomes

After completing the project, the student should be able to explain:

- What happens when fork() is called
- How a parent and child process differ
- What execvp() does and why it is used
- Why waitpid() is necessary
- What stdin, stdout, and stderr are
- What a file descriptor is
- What dup2() does during redirection
- How pipe() allows two processes to communicate
- What SIGINT and SIGCHLD mean
- Why zombie processes appear and how they are reaped

## 12. CV and Portfolio Target

Once implemented and tested, the project can be summarized in a CV or portfolio entry as:

```text
Mini Unix Shell | C++, Linux, Operating Systems

Built a Unix-like shell from scratch using Linux system calls for command parsing, process creation, execution, and synchronization.

Implemented process control with fork(), execvp(), waitpid(), and signal handling for foreground and background jobs.

Added Unix pipes and file redirection using pipe(), dup2(), and file descriptors to support command chaining and I/O management.

Designed the project with modular parsing and execution logic to improve reliability and maintainability.
```

Only claim features that are actually implemented and can be explained confidently in an interview or demo.

## 13. Deliverables

The final project should include:

- working shell implementation
- modular source files
- clear README documentation
- problem statement and project overview
- testing examples for basic functionality
- optional documentation for CV and interview preparation

## 14. Acceptance Criteria

The project is considered successful when:

- the shell launches and shows a prompt
- it executes simple external commands
- it supports builtin commands
- it handles input/output redirection
- it supports pipes and chained commands
- it runs jobs in the background
- it handles Ctrl+C and child processes without crashing
- invalid commands and malformed input are handled gracefully

This project is suitable for both academic learning and portfolio showcase work in systems programming, Linux development, and C++ software engineering.
Parent Child
| |
waitpid() dup2()
|
pipe?
|
execvp()

6. Core Linux APIs

Function

Purpose

fork()

Create a process

execvp()

Execute a program

waitpid()

Wait for child processes

pipe()

Create an IPC channel

dup2()

Redirect file descriptors

open()

Open files

close()

Close file descriptors

chdir()

Change working directory

getcwd()

Get current directory

sigaction()

Handle signals

7. Suggested Repository Structure

mini-unix-shell/
|
├── src/
│ ├── main.cpp
│ ├── parser.cpp
│ ├── parser.h
│ ├── executor.cpp
│ ├── executor.h
│ ├── builtins.cpp
│ ├── builtins.h
│ ├── signals.cpp
│ └── signals.h
|
├── tests/
│ ├── basic.txt
│ ├── redirection.txt
│ ├── pipes.txt
│ └── background.txt
|
├── docs/
│ └── problem-statement.md
|
├── Makefile
├── README.md
├── LICENSE
└── .gitignore

8. Development Phases

Phase 1 — Basic Shell

Prompt

Input

Parsing

fork()

execvp()

waitpid()

cd

pwd

exit

Phase 2 — Redirection

<

>

> >

open()

dup2()

close()

Phase 3 — Pipes

Single pipe

Multiple pipes

Phase 4 — Background Processes

&

Child-process cleanup

Zombie prevention

Phase 5 — Signals

SIGINT

SIGCHLD

Robust process lifecycle handling

Phase 6 — Testing and Documentation

Test normal commands, built-ins, invalid commands, redirection, pipes, multiple pipes, background execution, signals, and child termination. Document architecture and OS concepts in the README.

9. Example Final Session

$ ./myshell

myshell> pwd
/home/user/mini-unix-shell

myshell> ls

myshell> cd src
myshell> pwd

myshell> ls | grep cpp

myshell> ls > files.txt
myshell> cat files.txt

myshell> echo hello >> files.txt
myshell> cat files.txt

myshell> sleep 10 &
[1] 18452

myshell> pwd

myshell> sleep 20
^C

myshell> exit
$

10. Constraints

Use C/C++.

Run in Linux/Unix; Windows users may use WSL2 + Ubuntu.

Do not use system() for command execution.

Use Linux/Unix system calls directly where appropriate.

The shell does not need to reproduce complete Bash functionality.

Focus on the underlying OS mechanisms.

Keep the implementation compatible with GCC 4.8.5 / C++11 in the current lab environment.

11. Expected Learning Outcomes

After completing the project, you should be able to explain:

Processes

What happens when fork() is called?

What is a parent/child process?

What does exec() do?

Why is waitpid() required?

File Descriptors

What are stdin/stdout/stderr?

What is a file descriptor?

What does dup2() do?

IPC

How does pipe() work?

How does ls | grep work internally?

Signals

What happens when Ctrl+C is pressed?

What is SIGINT?

How does the shell remain alive?

Process Lifecycle

What is a zombie process?

How are child processes reaped?

What happens when a background process terminates?

12. CV Target

Once implemented and tested, the project can be represented as:

Mini Unix Shell | C++, Linux, Operating Systems

Built a Unix-like shell from scratch using Linux system calls for command parsing, process creation, execution, and synchronization.

Implemented fork/exec/waitpid based process management with foreground and background execution.

Implemented Unix pipes and I/O redirection using pipe, dup2, and file descriptors.

Added signal handling and child-process management for reliable process lifecycle handling.

Only claim features that are actually implemented and that you can explain in an interview.
