# Mini Unix Shell

A lightweight Unix-like shell written in C++ for Linux systems. This project focuses on the core mechanisms behind a shell: command parsing, process creation, file redirection, pipes, background jobs, and signal handling.

## Project goal

Build a working shell that can:

- read commands from a prompt
- run external programs using fork() and execvp()
- support built-ins like cd, pwd, and exit
- redirect input and output
- chain commands with pipes
- support background execution with &
- handle signals safely and avoid zombie processes

## Repository structure

- src/ — implementation files
- tests/ — sample command sets and validation scenarios
- problem-statement.md — full project brief and assignment description
- docs/ — supporting material such as CV-ready points and project roadmap
- Makefile — build configuration
- .gitignore — ignored generated and editor files

## Key documents

- [problem-statement.md](problem-statement.md)
- [docs/cv-points.md](docs/cv-points.md)
- [docs/implementation-roadmap.md](docs/implementation-roadmap.md)

## Build and run

```bash
make
./myshell
```

## Notes

This project is intentionally focused on learning OS concepts rather than reproducing the full behaviour of Bash.
