#include "signals.h"

#include <csignal>
#include <iostream>
#include <unistd.h>

namespace
{
    void handleSigint(int signum)
    {
        (void)signum;
        // On Ctrl+C, output a newline so shell stays responsive
        const char msg[] = "\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }
} // namespace

void setupSignalHandlers()
{
    struct sigaction saInt;
    saInt.sa_handler = handleSigint;
    sigemptyset(&saInt.sa_mask);
    saInt.sa_flags = SA_RESTART;
    sigaction(SIGINT, &saInt, nullptr);

    struct sigaction saTstp;
    saTstp.sa_handler = SIG_IGN;
    sigemptyset(&saTstp.sa_mask);
    saTstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &saTstp, nullptr);

    struct sigaction saQuit;
    saQuit.sa_handler = SIG_IGN;
    sigemptyset(&saQuit.sa_mask);
    saQuit.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &saQuit, nullptr);
}

