#include "executor.h"

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void executeCommand(const Command& command)
{
    // Convert vector<string> to char* array for execvp()
    vector<char*> argv;

    for (size_t i = 0; i < command.args.size(); i++)
    {
        argv.push_back(
            const_cast<char*>(command.args[i].c_str())
        );
    }

    argv.push_back(NULL);

    // Create child process
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        // Child process

        execvp(argv[0], &argv[0]);

        // execvp() returns only when an error occurs
        perror("execvp");
        _exit(1);
    }
    else
    {
        // Parent process

        int status;

        if (waitpid(pid, &status, 0) < 0)
            perror("waitpid");
    }
}
