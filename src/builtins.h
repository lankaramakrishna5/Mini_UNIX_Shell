#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>
#include <sys/types.h>

struct Job
{
    pid_t pid;
    std::string command;
    bool running;
};

extern std::vector<Job> g_jobs;

std::string joinCommand(const std::vector<std::string> &args);
void addJob(pid_t pid, const std::string &command);
void removeFinishedJobs();
void listJobs();

bool isBuiltin(const std::string &command);

int executeBuiltin(const std::string &command,
                   const std::vector<std::string> &args);

#endif

