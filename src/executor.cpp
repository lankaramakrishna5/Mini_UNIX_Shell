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

    bool applyRedirection(const Command &command, bool isFirstCommand, bool isLastCommand)
    {
        if (!command.inputFile.empty() && isFirstCommand)
        {
            int fd = open(command.inputFile.c_str(), O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                return false;
            }
            if (dup2(fd, STDIN_FILENO) < 0)
            {
                perror("dup2");
                close(fd);
                return false;
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
                return false;
            }
            if (dup2(fd, STDOUT_FILENO) < 0)
            {
                perror("dup2");
                close(fd);
                return false;
            }
            close(fd);
        }
        return true;
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

        execvp(argv[0], argv.data());
        perror("execvp");
        _exit(127);
    }

} // namespace

int executeCommand(const Command &command)
{
    ParsedLine pl;
    pl.commands.push_back(command);
    pl.background = false;
    return executeParsedLine(pl);
}

int executeParsedLine(const ParsedLine &parsedLine)
{
    if (parsedLine.commands.empty())
        return 0;

    const size_t commandCount = parsedLine.commands.size();

    // Single command execution
    if (commandCount == 1)
    {
        Command command = parsedLine.commands[0];
        expandWildcards(command);

        if (command.args.empty())
            return 0;

        if (isBuiltin(command.args[0]))
        {
            int savedIn = -1;
            int savedOut = -1;
            bool hasRedir = !command.inputFile.empty() || !command.outputFile.empty();

            if (hasRedir)
            {
                if (!command.inputFile.empty())
                    savedIn = dup(STDIN_FILENO);
                if (!command.outputFile.empty())
                    savedOut = dup(STDOUT_FILENO);

                if (!applyRedirection(command, true, true))
                {
                    if (savedIn >= 0)
                    {
                        dup2(savedIn, STDIN_FILENO);
                        close(savedIn);
                    }
                    if (savedOut >= 0)
                    {
                        dup2(savedOut, STDOUT_FILENO);
                        close(savedOut);
                    }
                    return 1;
                }
            }

            int ret = executeBuiltin(command.args[0], command.args);

            if (hasRedir)
            {
                cout.flush();
                cerr.flush();
                if (savedIn >= 0)
                {
                    dup2(savedIn, STDIN_FILENO);
                    close(savedIn);
                }
                if (savedOut >= 0)
                {
                    dup2(savedOut, STDOUT_FILENO);
                    close(savedOut);
                }
            }

            return ret;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return 1;
        }

        if (pid == 0)
        {
            if (parsedLine.background)
            {
                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
            }
            else
            {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
            }

            if (!applyRedirection(command, true, true))
            {
                _exit(1);
            }
            execCommandWithArgs(command);
        }

        if (parsedLine.background)
        {
            addJob(pid, joinCommand(command.args));
            cout << "[" << g_jobs.size() << "] " << pid << endl;
            return 0;
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0)
        {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);

        return 0;
    }

    // Pipeline execution (commandCount > 1)
    vector<int> pipeFds((commandCount - 1) * 2);
    for (size_t i = 0; i + 1 < commandCount; ++i)
    {
        if (pipe(&pipeFds[i * 2]) < 0)
        {
            perror("pipe");
            return 1;
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
            for (int fd : pipeFds)
                close(fd);
            for (pid_t childPid : childPids)
                waitpid(childPid, nullptr, 0);
            return 1;
        }

        if (pid == 0)
        {
            if (parsedLine.background)
            {
                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
            }
            else
            {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
            }

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
                int exitCode = executeBuiltin(command.args[0], command.args);
                _exit(exitCode);
            }

            execCommandWithArgs(command);
        }

        childPids.push_back(pid);
    }

    // Close all pipe fds in parent
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
        return 0;
    }

    int lastExitCode = 0;
    for (size_t i = 0; i < childPids.size(); ++i)
    {
        int status = 0;
        if (waitpid(childPids[i], &status, 0) >= 0)
        {
            if (i == childPids.size() - 1)
            {
                if (WIFEXITED(status))
                    lastExitCode = WEXITSTATUS(status);
                else if (WIFSIGNALED(status))
                    lastExitCode = 128 + WTERMSIG(status);
            }
        }
        else
        {
            if (errno != ECHILD)
            {
                perror("waitpid");
            }
        }
    }

    return lastExitCode;
}

