# Implementation Roadmap

## Phase 1: Shell loop and basic execution

- print a prompt
- read user input
- ignore empty lines
- parse the command
- detect built-ins such as cd, pwd, and exit
- execute simple external commands using fork() and execvp()
- wait for foreground child processes with waitpid()

## Phase 2: Built-in commands

- implement cd in the parent shell process
- implement pwd using the current working directory
- implement exit to gracefully terminate the shell
- ensure invalid directories and missing paths are handled cleanly

## Phase 3: Redirection

- support < for input redirection
- support > for output redirection
- support >> for append mode
- use open(), dup2(), and close() carefully
- validate file access errors and malformed redirection syntax

## Phase 4: Pipes

- create a pipe with pipe()
- connect stdout of one process to stdin of the next using dup2()
- handle multiple pipes in sequence
- avoid leaking file descriptors
- support chained commands such as ls | grep cpp | wc -l

## Phase 5: Background jobs

- support the & operator
- allow the shell prompt to reappear quickly
- maintain a record of active child processes when needed
- reap finished children to prevent zombies
- avoid blocking on background tasks while waiting for foreground commands

## Phase 6: Signals and process cleanup

- handle Ctrl+C using sigaction()
- stop a foreground child from terminating the shell itself
- handle SIGCHLD to clean up terminated children
- ensure robust error handling for interrupted system calls

## Phase 7: Testing and validation

- test valid commands
- test invalid commands and missing paths
- test nested behaviour with pipes and redirection
- test background processes and repeated commands
- verify shell remains responsive after interrupts and errors

## Phase 8: Documentation and portfolio polish

- write a clear README
- add this problem statement and roadmap
- prepare CV bullets and interview talking points
- confirm each feature is truly implemented before listing it

## Suggested test cases

```bash
pwd
ls
cd /tmp
pwd
cat < file.txt
ls > output.txt
ls >> output.txt
ls | grep cpp
cat file.txt | grep hello | wc -l
sleep 10 &
ls
sleep 20
exit
```

## Quality bar

The project should not just compile — it should demonstrate a clear understanding of the Linux process model and the typical responsibilities of a shell.
