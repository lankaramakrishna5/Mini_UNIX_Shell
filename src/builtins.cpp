#include "builtins.h"

#include <csignal>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

using namespace std;

vector<Job> g_jobs;

string joinCommand(const vector<string>& args)
{
    if (args.empty())
        return {};

    ostringstream oss;
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (i > 0)
            oss << ' ';
        oss << args[i];
    }
    return oss.str();
}

namespace
{
    size_t parseJobIndex(const string& token)
    {
        size_t value = 0;
        size_t idx = 0;

        if (token.empty())
            return 0;

        for (char ch : token)
        {
            if (ch < '0' || ch > '9')
                return 0;
            value = value * 10 + static_cast<size_t>(ch - '0');
            ++idx;
        }

        if (idx == 0)
            return 0;

        return value;
    }
} // namespace

void addJob(pid_t pid, const string& command)
{
    removeFinishedJobs();
    g_jobs.push_back(Job{pid, command, true});
}

void removeFinishedJobs()
{
    for (auto it = g_jobs.begin(); it != g_jobs.end();)
    {
        int status = 0;
        pid_t result = waitpid(it->pid, &status, WNOHANG);

        if (result == it->pid)
        {
            it = g_jobs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void listJobs()
{
    removeFinishedJobs();
    for (size_t i = 0; i < g_jobs.size(); ++i)
    {
        cout << '[' << (i + 1) << "] " << g_jobs[i].pid << ' '
             << (g_jobs[i].running ? "Running" : "Stopped") << " "
             << g_jobs[i].command << endl;
    }
}

bool isBuiltin(const string& command)
{
    return command == "cd" ||
           command == "pwd" ||
           command == "exit" ||
           command == "jobs" ||
           command == "bg" ||
           command == "fg";
}

bool executeBuiltin(const string& command,
                    const vector<string>& args)
{
    if (command == "cd")
    {
        if (args.size() < 2)
        {
            cerr << "cd: missing argument" << endl;
            return true;
        }

        if (chdir(args[1].c_str()) != 0)
            perror("cd");

        return true;
    }

    if (command == "pwd")
    {
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            cout << cwd << endl;
        else
            perror("pwd");

        return true;
    }

    if (command == "jobs")
    {
        listJobs();
        return true;
    }

    if (command == "bg")
    {
        if (args.size() < 2)
        {
            cerr << "bg: missing job id" << endl;
            return true;
        }

        size_t jobIndex = parseJobIndex(args[1]);
        if (jobIndex == 0 || jobIndex > g_jobs.size())
        {
            cerr << "bg: invalid job id" << endl;
            return true;
        }

        pid_t pid = g_jobs[jobIndex - 1].pid;
        if (kill(pid, SIGCONT) < 0)
            perror("kill");
        else
            g_jobs[jobIndex - 1].running = true;

        return true;
    }

    if (command == "fg")
    {
        if (args.size() < 2)
        {
            cerr << "fg: missing job id" << endl;
            return true;
        }

        size_t jobIndex = parseJobIndex(args[1]);
        if (jobIndex == 0 || jobIndex > g_jobs.size())
        {
            cerr << "fg: invalid job id" << endl;
            return true;
        }

        pid_t pid = g_jobs[jobIndex - 1].pid;
        cout << g_jobs[jobIndex - 1].command << endl;

        if (kill(pid, SIGCONT) < 0)
            perror("kill");

        int status = 0;
        waitpid(pid, &status, 0);
        removeFinishedJobs();

        return true;
    }

    if (command == "exit")
    {
        exit(0);
    }

    return false;
}
