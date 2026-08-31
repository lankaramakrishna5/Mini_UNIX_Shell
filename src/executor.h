#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

int executeCommand(const Command &command);
int executeParsedLine(const ParsedLine &parsedLine);

#endif
