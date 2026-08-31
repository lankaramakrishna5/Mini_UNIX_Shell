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

    while (true)
    {
        cout << "myshell> ";
        cout.flush();

        if (!getline(cin, input))
        {
            if (cin.eof())
            {
                cout << endl;
                break;
            }

            continue;
        }

        if (input.empty())
            continue;

        ParsedLine parsedLine = parseCommand(input);

        if (parsedLine.commands.empty())
            continue;

        executeParsedLine(parsedLine);
    }

    return 0;
}
