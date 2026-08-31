# Mini Unix Shell

A lightweight Unix-like shell written in modern C++ (C++17) for Linux systems. This project demonstrates core operating systems mechanisms: command tokenization, process creation, file descriptor redirection, multi-stage pipelines, background jobs, and signal handling.

## Project Features

- **Process Lifecycle Management:** External program execution using `fork()`, `execvp()`, and `waitpid()`.
- **Built-in Commands:** `cd` (with `~` and `cd -` / `OLDPWD` support), `pwd`, `echo` (with `-n`), `export`, `unset`, `jobs`, `fg`, `bg`, `help`, and `exit`.
- **Quote-Aware Parser:** Full support for single quotes (`'...'`), double quotes (`"..."`), escape characters (`\`), and comments (`#`).
- **Environment Variable Expansion:** Dynamic expansion of `$VAR`, `$?` (last exit code), `$$` (shell PID), and `~` (home directory).
- **Stream Redirection:** Input (`<`), Output (`>`), and Append (`>>`) redirection with file descriptor save/restore mechanics.
- **Pipelines:** Arbitrary multi-stage piping (`cmd1 | cmd2 | ... | cmdN`) via `pipe()` and `dup2()`.
- **Job Control & Signals:** Background execution with `&`, signal-safe non-blocking reaping of finished jobs (`WNOHANG`), and graceful handling of `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+Z), and `SIGCHLD`.

## Repository Structure

- `src/` — modular source files (`main.cpp`, `parser.cpp`, `executor.cpp`, `builtins.cpp`, `signals.cpp`)
- `tests/` — automated test runner and test scenarios
- `docs/` — implementation roadmaps and documentation
- `problem-statement.md` — full project brief and assignment description
- `Makefile` — build configuration

## Key Documents

- [docs/implementation-roadmap.md](docs/implementation-roadmap.md) — project roadmap
- [problem-statement.md](problem-statement.md) — project brief

## Build and Run

```bash
make
./myshell
```

## Running Tests

```bash
bash tests/run_tests.sh
```

