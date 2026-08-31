#include "signals.h"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
    void handleSigint(int signum)
    {
        (void)signum;
    }

    void handleSigtstp(int signum)
    {
        (void)signum;
    }
} // namespace

void setupSignalHandlers()
{
    struct sigaction sigIntAction;
    sigIntAction.sa_handler = handleSigint;
    sigemptyset(&sigIntAction.sa_mask);
    sigIntAction.sa_flags = 0;
    sigaction(SIGINT, &sigIntAction, nullptr);

    struct sigaction sigTstpAction;
    sigTstpAction.sa_handler = handleSigtstp;
    sigemptyset(&sigTstpAction.sa_mask);
    sigTstpAction.sa_flags = 0;
    sigaction(SIGTSTP, &sigTstpAction, nullptr);

    struct sigaction sigChldAction;
    sigChldAction.sa_handler = SIG_IGN;
    sigemptyset(&sigChldAction.sa_mask);
    sigChldAction.sa_flags = 0;
    sigaction(SIGCHLD, &sigChldAction, nullptr);
}
