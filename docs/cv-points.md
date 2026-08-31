# CV and Portfolio Points

## Short project title

Mini Unix Shell | C++, Linux, Operating Systems

## One-line summary

Built a Unix-like shell in C++ that parses commands, creates child processes, redirects I/O, supports pipes, and manages background jobs using Linux system calls.

## CV-ready bullet points

- Built a Unix-like shell in C++ for Linux using fork(), execvp(), waitpid(), pipe(), dup2(), and file descriptors.
- Implemented command parsing and execution flow for external programs, built-in commands, and error handling.
- Added support for input redirection, output redirection, and append redirection for file-based command workflows.
- Designed command pipelines to connect the output of one process to the input of another using Unix pipes.
- Implemented background process execution with signal-aware job management and child-process cleanup.
- Applied operating systems concepts including process lifecycle, file descriptor management, and signal handling.
- Structured the project into modular source files to keep parsing, built-ins, and execution logic maintainable.

## Interview-friendly explanation

This project demonstrates hands-on understanding of how a shell works at the operating system level. Instead of invoking a shell command directly, the implementation creates child processes with fork(), replaces them with the target program using execvp(), and waits for completion with waitpid(). Pipes and redirection are implemented by manipulating file descriptors, which shows a practical understanding of how the kernel manages process I/O.

## Stronger portfolio phrasing

A command-line shell is one of the clearest examples of systems programming because it combines parsing, process management, synchronization, and file handling in a single small program. This project helped build confidence in working close to the Linux kernel and understanding how user-level programs coordinate with OS primitives.