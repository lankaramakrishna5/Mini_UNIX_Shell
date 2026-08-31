#include "executor.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <glob.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>

#include "builtins.h"

using namespace std;

namespace
{
    vector<string> expandPattern(const string &pattern)
    {
        vector<string> result;

        if (pattern.find('*') == string::npos &&
            pattern.find('?') == string::npos &&
            pattern.find('[') == string::npos)
        {
            return {pattern};
        }

        glob_t globbuf;
        memset(&globbuf, 0, sizeof(globbuf));

        int rc = glob(pattern.c_str(), GLOB_TILDE, nullptr, &globbuf);
        if (rc == 0)
        {
            for (size_t i = 0; i < globbuf.gl_pathc; ++i)
            {
                result.push_back(string(globbuf.gl_pathv[i]));
            }
        }
        else if (rc == GLOB_NOMATCH)
        {
            result.push_back(pattern);
        }

        globfree(&globbuf);
        return result;
    }

    void expandWildcards(Command &command)
    {
        vector<string> expandedArgs;
        for (const string &arg : command.args)
        {
            vector<string> matches = expandPattern(arg);
            expandedArgs.insert(expandedArgs.end(), matches.begin(), matches.end());
        }

        if (!expandedArgs.empty())
        {
            command.args = expandedArgs;
        }

        if (!command.inputFile.empty())
        {
            vector<string> inputMatches = expandPattern(command.inputFile);
            if (!inputMatches.empty())
                command.inputFile = inputMatches[0];
        }

        if (!command.outputFile.empty())
        {
            vector<string> outputMatches = expandPattern(command.outputFile);
            if (!outputMatches.empty())
                command.outputFile = outputMatches[0];
        }
    }

    void applyRedirection(const Command &command, bool isFirstCommand, bool isLastCommand)
    {
        if (!command.inputFile.empty() && isFirstCommand)
        {
            int fd = open(command.inputFile.c_str(), O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                _exit(1);
            }
            if (dup2(fd, STDIN_FILENO) < 0)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);
        }

        if (!command.outputFile.empty() && isLastCommand)
        {
            int flags = O_WRONLY | O_CREAT | (command.append ? O_APPEND : O_TRUNC);
            int fd = open(command.outputFile.c_str(), flags, 0644);
            if (fd < 0)
            {
                perror("open");
                _exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) < 0)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);
        }
    }

    void execCommandWithArgs(const Command &command)
    {
        vector<char *> argv;
        for (size_t i = 0; i < command.args.size(); ++i)
        {
            argv.push_back(const_cast<char *>(command.args[i].c_str()));
        }
        argv.push_back(nullptr);

        if (argv.empty() || argv[0] == nullptr)
        {
            _exit(0);
        }

        execvp(argv[0], &argv[0]);
        perror("execvp");
        _exit(1);
    }

} // namespace

void executeCommand(const Command &command)
{
    if (command.args.empty())
        return;

    Command expandedCommand = command;
    expandWildcards(expandedCommand);

    if (isBuiltin(expandedCommand.args[0]))
    {
        executeBuiltin(expandedCommand.args[0], expandedCommand.args);
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        applyRedirection(expandedCommand, true, true);
        execCommandWithArgs(expandedCommand);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
        perror("waitpid");
    }
}

void executeParsedLine(const ParsedLine &parsedLine)
{
    if (parsedLine.commands.empty())
        return;

    const size_t commandCount = parsedLine.commands.size();

    if (commandCount == 1)
    {
        Command command = parsedLine.commands[0];
        expandWildcards(command);

        if (command.args.empty())
            return;

        if (isBuiltin(command.args[0]))
        {
            executeBuiltin(command.args[0], command.args);
            return;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return;
        }

        if (pid == 0)
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            applyRedirection(command, true, true);
            execCommandWithArgs(command);
        }

        if (parsedLine.background)
        {
            addJob(pid, joinCommand(command.args));
            cout << "[" << g_jobs.size() << "] " << pid << endl;
            return;
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0)
        {
            perror("waitpid");
        }
        return;
    }

    vector<int> pipeFds((commandCount - 1) * 2);
    for (size_t i = 0; i + 1 < commandCount; ++i)
    {
        if (pipe(&pipeFds[i * 2]) < 0)
        {
            perror("pipe");
            return;
        }
    }

    vector<pid_t> childPids;
    childPids.reserve(commandCount);

    for (size_t i = 0; i < commandCount; ++i)
    {
        Command command = parsedLine.commands[i];
        expandWildcards(command);

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            for (pid_t childPid : childPids)
            {
                waitpid(childPid, nullptr, 0);
            }
            return;
        }

        if (pid == 0)
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i > 0)
            {
                dup2(pipeFds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i + 1 < commandCount)
            {
                dup2(pipeFds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int fd : pipeFds)
            {
                close(fd);
            }

            if (!command.inputFile.empty() && i == 0)
            {
                int inFd = open(command.inputFile.c_str(), O_RDONLY);
                if (inFd < 0)
                {
                    perror("open");
                    _exit(1);
                }
                dup2(inFd, STDIN_FILENO);
                close(inFd);
            }

            if (!command.outputFile.empty() && i + 1 == commandCount)
            {
                int flags = O_WRONLY | O_CREAT | (command.append ? O_APPEND : O_TRUNC);
                int outFd = open(command.outputFile.c_str(), flags, 0644);
                if (outFd < 0)
                {
                    perror("open");
                    _exit(1);
                }
                dup2(outFd, STDOUT_FILENO);
                close(outFd);
            }

            if (!command.args.empty() && isBuiltin(command.args[0]))
            {
                executeBuiltin(command.args[0], command.args);
                _exit(0);
            }

            execCommandWithArgs(command);
        }

        childPids.push_back(pid);
    }

    for (int fd : pipeFds)
    {
        close(fd);
    }

    if (parsedLine.background)
    {
        for (pid_t childPid : childPids)
        {
            addJob(childPid, "pipeline");
        }
        cout << "[" << g_jobs.size() << "] " << childPids.front() << endl;
        return;
    }

    for (pid_t childPid : childPids)
    {
        int status = 0;
        if (waitpid(childPid, &status, 0) < 0)
        {
            if (errno != ECHILD)
            {
                perror("waitpid");
            }
        }
    }
}
