#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

struct Command
{
    std::vector<std::string> args;

    std::string inputFile;
    std::string outputFile;

    bool append;
};

struct ParsedLine
{
    std::vector<Command> commands;

    bool background;
};

ParsedLine parseCommand(const std::string& input, int lastExitStatus = 0);

#endif
