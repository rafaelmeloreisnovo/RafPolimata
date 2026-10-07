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
    int status = 0;
    pid_t waited;

    if (raf_exec_spec_valid(spec) == 0u) return 126;

    pid = fork();
    if (pid < (pid_t)0) return 125;
    if (pid == (pid_t)0) {
        execvp(
            spec->argv[0],
            (char *const *)(void *)spec->argv
        );
        _exit(127);
    }

    do {
        waited = waitpid(pid, &status, 0);
    } while (waited < (pid_t)0 && errno == EINTR);

    if (waited < (pid_t)0) return 125;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 125;
}
#else
static inline int raf_host_exec(const RafExecSpec *spec)
{
    (void)spec;
    return 127;
}
#endif

#endif
