#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>

bool isBuiltin(const std::string& command);

bool executeBuiltin(const std::string& command,
                    const std::vector<std::string>& args);

#endif
