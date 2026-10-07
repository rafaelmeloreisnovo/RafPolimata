#ifndef RAFBBS_HOST_H
#define RAFBBS_HOST_H

#include "rafbbs_exec_core.h"

#ifndef RAFBBS_FREESTANDING_MODE
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static inline int raf_host_exec(const RafExecSpec *spec)
{
    pid_t pid;
    pid_t waited;
    int status = 0;

    if (spec == (const RafExecSpec *)0 ||
        spec->argc == 0u ||
        spec->argv[0] == (const char *)0)
        return 127;

    pid = fork();
    if (pid < 0)
        return 127;

    if (pid == 0) {
        execvp(spec->argv[0], (char *const *)spec->argv);
        _exit(127);
    }

    do {
        waited = waitpid(pid, &status, 0);
    } while (waited < 0 && errno == EINTR);

    if (waited < 0)
        return 127;
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return 127;
}
#else
static inline int raf_host_exec(const RafExecSpec *spec)
{
    (void)spec;
    return 127;
}
#endif

#endif
