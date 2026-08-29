#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "builtins.h"
#include "parser.h"

using namespace std;

int main()
{
    string input;

    while (true)
    {
        cout << "myshell> ";

        getline(cin, input);

        // Handle Ctrl+D / EOF
        if (cin.eof())
        {
            cout << endl;
            break;
        }

        // Ignore empty input
        if (input.empty())
            continue;

        // Parse command
        Command command = parseCommand(input);

        if (command.args.empty())
            continue;

        vector<string> args = command.args;

        // Handle built-in commands
        if (isBuiltin(args[0]))
        {
            executeBuiltin(args[0], args);
            continue;
        }

        // Convert vector<string> to char* array for execvp()
        vector<char*> argv;

        for (size_t i = 0; i < args.size(); i++)
            argv.push_back(const_cast<char*>(args[i].c_str()));

        argv.push_back(NULL);

        // Create child process
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            // Child process
            execvp(argv[0], &argv[0]);

            // execvp() returns only if an error occurs
            perror("execvp");
            _exit(1);
        }
        else
        {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}
