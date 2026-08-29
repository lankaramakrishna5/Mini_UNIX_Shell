#include "builtins.h"

#include <iostream>
#include <unistd.h>
#include <limits.h>

using namespace std;

bool isBuiltin(const string& command)
{
    return command == "cd" ||
           command == "pwd" ||
           command == "exit";
}

bool executeBuiltin(const string& command,
                    const vector<string>& args)
{
    if (command == "cd")
    {
        if (args.size() < 2)
        {
            cerr << "cd: missing argument" << endl;
            return true;
        }

        if (chdir(args[1].c_str()) != 0)
            perror("cd");

        return true;
    }

    if (command == "pwd")
    {
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            cout << cwd << endl;
        else
            perror("pwd");

        return true;
    }

    if (command == "exit")
    {
        exit(0);
    }

    return false;
}
