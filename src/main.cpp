#include <iostream>
#include <string>

#include "builtins.h"
#include "parser.h"
#include "executor.h"

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
        ParsedLine parsedLine = parseCommand(input);

        // No command
        if (parsedLine.commands.empty())
            continue;

        // For now, execute only the first command.
        // Pipes, redirection and background execution
        // will be handled by the executor later.
        Command command = parsedLine.commands[0];

        if (command.args.empty())
            continue;

        // Handle built-in commands
        if (isBuiltin(command.args[0]))
        {
            executeBuiltin(command.args[0], command.args);
            continue;
        }

        // Execute external command
        executeCommand(command);
    }

    return 0;
}
