#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef F_ADD_SEALS
#define F_ADD_SEALS 1033
#endif
#ifndef F_GET_SEALS
#define F_GET_SEALS 1034
#endif
#ifndef F_SEAL_SEAL
#define F_SEAL_SEAL 0x0001
#endif
#ifndef F_SEAL_SHRINK
#define F_SEAL_SHRINK 0x0002
#endif
#ifndef F_SEAL_GROW
#define F_SEAL_GROW 0x0004
#endif
#ifndef F_SEAL_WRITE
#define F_SEAL_WRITE 0x0008
#endif
#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif

static int x_memfd_create(const char *name, unsigned int flags) {
#ifdef SYS_memfd_create
    return (int)syscall(SYS_memfd_create, name, flags);
#elif defined(__NR_memfd_create)
    return (int)syscall(__NR_memfd_create, name, flags);
#else
    errno = ENOSYS;
    return -1;
#endif
}

static void emit(const char *k, const char *v) { printf("%s=%s\n", k, v); }

int main(void) {
    const int required = F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL;
    const char payload[] = "APKC_MEMFD_RUNTIME_PROBE_V1\n";
    int mfd = x_memfd_create("apkc-runtime-probe", MFD_ALLOW_SEALING);
    if (mfd < 0) {
        emit("memfd_create", "FAIL");
        printf("errno=%d\nclaim_allowed=false\n", errno);
        return 20;
    }
    emit("memfd_create", "PASS");
    if (write(mfd, payload, sizeof(payload)-1) != (ssize_t)(sizeof(payload)-1)) {
        emit("payload_write", "FAIL"); emit("claim_allowed", "false"); return 21;
    }
    if (fcntl(mfd, F_ADD_SEALS, required) < 0) {
        emit("apply_seals", "FAIL"); printf("errno=%d\nclaim_allowed=false\n", errno); return 22;
    }
    int seals = fcntl(mfd, F_GET_SEALS);
    if (seals < 0 || (seals & required) != required) {
        emit("readback_seals", "FAIL"); printf("seals=0x%x\nclaim_allowed=false\n", seals); return 23;
    }
    emit("readback_seals", "PASS");
    printf("seals=0x%x\n", seals);

    errno = 0;
    ssize_t wr = write(mfd, "X", 1);
    if (wr >= 0 || errno != EPERM) {
        emit("sealed_write_rejected", "FAIL"); printf("write_rc=%zd\nerrno=%d\nclaim_allowed=false\n", wr, errno); return 24;
    }
    emit("sealed_write_rejected", "PASS");

    if (lseek(mfd, 0, SEEK_SET) < 0) { emit("rewind", "FAIL"); emit("claim_allowed", "false"); return 25; }
    int p[2];
    if (pipe(p) < 0) { emit("pipe", "FAIL"); emit("claim_allowed", "false"); return 26; }
    pid_t pid = fork();
    if (pid < 0) { emit("fork", "FAIL"); emit("claim_allowed", "false"); return 27; }
    if (pid == 0) {
        if (dup2(p[1], STDOUT_FILENO) < 0) _exit(120);
        close(p[0]); close(p[1]);
        char fdpath[64];
        snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", mfd);
        execlp("cat", "cat", fdpath, (char*)NULL);
        _exit(121);
    }
    close(p[1]);
    char buf[128] = {0};
    ssize_t n = read(p[0], buf, sizeof(buf)-1);
    close(p[0]);
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) { emit("exec_wait", "FAIL"); emit("claim_allowed", "false"); return 28; }
    if (n != (ssize_t)(sizeof(payload)-1) || memcmp(buf, payload, sizeof(payload)-1) != 0 || !WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        emit("proc_self_fd_exec_inheritance", "FAIL");
        printf("read_bytes=%zd\nchild_status=%d\nclaim_allowed=false\n", n, st);
        return 29;
    }
    emit("proc_self_fd_exec_inheritance", "PASS");
    emit("runtime_probe", "PASS");
    emit("android_compatibility", "TOKEN_VAZIO");
    emit("claim_allowed", "false");
    close(mfd);
    return 0;
}
