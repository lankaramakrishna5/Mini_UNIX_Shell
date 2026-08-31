#include <iostream>
#include <string>

#include "builtins.h"
#include "executor.h"
#include "parser.h"
#include "signals.h"

using namespace std;

int main()
{
    setupSignalHandlers();

    string input;
    int lastExitCode = 0;

    while (true)
    {
        removeFinishedJobs();

        cout << "myshell> ";
        cout.flush();

        if (!getline(cin, input))
        {
            if (cin.eof())
            {
                cout << endl;
                break;
            }

            // Clear stream error state if interrupted by signal
            cin.clear();
            continue;
        }

        if (input.empty())
            continue;

        ParsedLine parsedLine = parseCommand(input, lastExitCode);

        if (parsedLine.commands.empty())
            continue;

        lastExitCode = executeParsedLine(parsedLine);
    }

    return 0;
}
