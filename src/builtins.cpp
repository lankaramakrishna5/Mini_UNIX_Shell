#include "builtins.h"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

using namespace std;

vector<Job> g_jobs;

string joinCommand(const vector<string> &args)
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
    size_t parseJobIndex(const string &token)
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

void addJob(pid_t pid, const string &command)
{
    removeFinishedJobs();
    g_jobs.push_back(Job{pid, command, true});
}

void removeFinishedJobs()
{
    for (auto it = g_jobs.begin(); it != g_jobs.end();)
    {
        int status = 0;
        pid_t result = waitpid(it->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (result == it->pid)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                it = g_jobs.erase(it);
            }
            else if (WIFSTOPPED(status))
            {
                it->running = false;
                ++it;
            }
            else if (WIFCONTINUED(status))
            {
                it->running = true;
                ++it;
            }
            else
            {
                ++it;
            }
        }
        else if (result == -1 && errno == ECHILD)
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

bool isBuiltin(const string &command)
{
    return command == "cd" ||
           command == "pwd" ||
           command == "exit" ||
           command == "jobs" ||
           command == "bg" ||
           command == "fg" ||
           command == "export" ||
           command == "unset" ||
           command == "echo" ||
           command == "help";
}

int executeBuiltin(const string &command,
                   const vector<string> &args)
{
    if (command == "cd")
    {
        string target;
        char currentCwd[PATH_MAX];
        char *gotCwd = getcwd(currentCwd, sizeof(currentCwd));

        if (args.size() < 2 || args[1] == "~")
        {
            const char *home = getenv("HOME");
            if (!home)
            {
                cerr << "cd: HOME not set" << endl;
                return 1;
            }
            target = home;
        }
        else if (args[1] == "-")
        {
            const char *oldpwd = getenv("OLDPWD");
            if (!oldpwd)
            {
                cerr << "cd: OLDPWD not set" << endl;
                return 1;
            }
            target = oldpwd;
        }
        else
        {
            target = args[1];
        }

        if (chdir(target.c_str()) != 0)
        {
            perror("cd");
            return 1;
        }

        if (gotCwd)
        {
            setenv("OLDPWD", currentCwd, 1);
        }

        char newCwd[PATH_MAX];
        if (getcwd(newCwd, sizeof(newCwd)) != nullptr)
        {
            setenv("PWD", newCwd, 1);
            if (args.size() >= 2 && args[1] == "-")
            {
                cout << newCwd << endl;
            }
        }

        return 0;
    }

    if (command == "pwd")
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            cout << cwd << endl;
            return 0;
        }
        perror("pwd");
        return 1;
    }

    if (command == "echo")
    {
        bool noNewline = false;
        size_t startIdx = 1;

        if (args.size() > 1 && args[1] == "-n")
        {
            noNewline = true;
            startIdx = 2;
        }

        for (size_t i = startIdx; i < args.size(); ++i)
        {
            if (i > startIdx)
                cout << ' ';
            cout << args[i];
        }

        if (!noNewline)
            cout << '\n';

        cout.flush();
        return 0;
    }

    if (command == "export")
    {
        if (args.size() == 1)
        {
            for (char **env = environ; *env != nullptr; ++env)
            {
                cout << *env << endl;
            }
            return 0;
        }

        for (size_t i = 1; i < args.size(); ++i)
        {
            size_t eqPos = args[i].find('=');
            if (eqPos != string::npos)
            {
                string key = args[i].substr(0, eqPos);
                string val = args[i].substr(eqPos + 1);
                setenv(key.c_str(), val.c_str(), 1);
            }
        }
        return 0;
    }

    if (command == "unset")
    {
        if (args.size() < 2)
        {
            cerr << "unset: not enough arguments" << endl;
            return 1;
        }

        for (size_t i = 1; i < args.size(); ++i)
        {
            unsetenv(args[i].c_str());
        }
        return 0;
    }

    if (command == "help")
    {
        cout << "Mini UNIX Shell Built-in Commands:\n"
             << "  cd [dir]       Change current working directory (supports ~, -)\n"
             << "  pwd            Print current working directory\n"
             << "  echo [args]    Display a line of text (supports -n)\n"
             << "  export [K=V]   Set environment variable(s)\n"
             << "  unset [VAR]    Unset environment variable(s)\n"
             << "  jobs           List background and stopped jobs\n"
             << "  bg [job_id]    Resume suspended job in background\n"
             << "  fg [job_id]    Bring job to foreground\n"
             << "  help           Show this help message\n"
             << "  exit [code]    Exit the shell\n";
        return 0;
    }

    if (command == "jobs")
    {
        listJobs();
        return 0;
    }

    if (command == "bg")
    {
        if (args.size() < 2)
        {
            cerr << "bg: missing job id" << endl;
            return 1;
        }

        size_t jobIndex = parseJobIndex(args[1]);
        if (jobIndex == 0 || jobIndex > g_jobs.size())
        {
            cerr << "bg: invalid job id" << endl;
            return 1;
        }

        pid_t pid = g_jobs[jobIndex - 1].pid;
        if (kill(pid, SIGCONT) < 0)
        {
            perror("kill");
            return 1;
        }

        g_jobs[jobIndex - 1].running = true;
        return 0;
    }

    if (command == "fg")
    {
        if (args.size() < 2)
        {
            cerr << "fg: missing job id" << endl;
            return 1;
        }

        size_t jobIndex = parseJobIndex(args[1]);
        if (jobIndex == 0 || jobIndex > g_jobs.size())
        {
            cerr << "fg: invalid job id" << endl;
            return 1;
        }

        pid_t pid = g_jobs[jobIndex - 1].pid;
        cout << g_jobs[jobIndex - 1].command << endl;

        if (kill(pid, SIGCONT) < 0)
            perror("kill");

        int status = 0;
        waitpid(pid, &status, 0);
        removeFinishedJobs();

        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        return 0;
    }

    if (command == "exit")
    {
        int exitCode = 0;
        if (args.size() > 1)
        {
            exitCode = atoi(args[1].c_str());
        }
        exit(exitCode);
    }

    return 1;
}
