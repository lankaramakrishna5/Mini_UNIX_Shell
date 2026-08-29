#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

struct Command
{
    std::vector<std::string> args;
};

Command parseCommand(const std::string& input);

#endif
