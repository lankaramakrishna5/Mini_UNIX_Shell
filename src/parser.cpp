#include "parser.h"

#include <sstream>

using namespace std;

Command parseCommand(const string& input)
{
    Command command;

    stringstream ss(input);
    string word;

    while (ss >> word)
    {
        command.args.push_back(word);
    }

    return command;
}
