#include "parser.h"

#include <sstream>

using namespace std;

ParsedLine parseCommand(const string& input)
{
    ParsedLine result;
    result.background = false;

    Command currentCommand;
    currentCommand.append = false;

    stringstream ss(input);
    string word;

    while (ss >> word)
    {
        // Background execution
        if (word == "&")
        {
            result.background = true;
        }

        // Pipe
        else if (word == "|")
        {
            if (!currentCommand.args.empty())
            {
                result.commands.push_back(currentCommand);

                currentCommand = Command();
                currentCommand.append = false;
            }
        }

        // Input redirection
        else if (word == "<")
        {
            if (ss >> word)
            {
                currentCommand.inputFile = word;
            }
        }

        // Output redirection
        else if (word == ">")
        {
            if (ss >> word)
            {
                currentCommand.outputFile = word;
                currentCommand.append = false;
            }
        }

        // Append redirection
        else if (word == ">>")
        {
            if (ss >> word)
            {
                currentCommand.outputFile = word;
                currentCommand.append = true;
            }
        }

        // Normal argument
        else
        {
            currentCommand.args.push_back(word);
        }
    }

    // Add final command
    if (!currentCommand.args.empty())
    {
        result.commands.push_back(currentCommand);
    }

    return result;
}
