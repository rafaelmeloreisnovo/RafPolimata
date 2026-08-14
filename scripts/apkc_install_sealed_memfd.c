#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <stdbool.h>
#include <stdint.h>
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

static const int REQUIRED_SEALS = F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL;

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

static int is_sha256(const char *s) {
    if (!s || strlen(s) != 64) return 0;
    for (size_t i = 0; i < 64; i++) {
        char c = s[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return 0;
    }
    return 1;
}

static void mkdir_best_effort(const char *p) {
    if (p && *p) (void)mkdir(p, 0700);
}

static void write_status(const char *outdir, const char *status, const char *reason,
                         const char *expected, const char *observed, int seals) {
    if (!outdir || !*outdir) return;
    mkdir_best_effort(outdir);
    char path[4096];
    if (snprintf(path, sizeof(path), "%s/sealed-memfd.status", outdir) >= (int)sizeof(path)) return;
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "sealed_snapshot_status=%s\n", status ? status : "FAIL");
    fprintf(f, "claim_allowed=false\n");
    fprintf(f, "same_inode_inplace_protection=%s\n",
            (status && strcmp(status, "READY") == 0) ? "MITIGATED_BY_SEALED_SNAPSHOT" : "TOKEN_VAZIO");
    if (expected) fprintf(f, "expected_sha256=%s\n", expected);
    if (observed) fprintf(f, "sealed_snapshot_sha256=%s\n", observed);
    if (seals >= 0) fprintf(f, "seals=0x%x\n", seals);
    if (reason) fprintf(f, "reason=%s\n", reason);
    fclose(f);
}

static int copy_all(int in, int out) {
    unsigned char buf[1 << 16];
    for (;;) {
        ssize_t n = read(in, buf, sizeof(buf));
        if (n == 0) return 0;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));
            if (w < 0) {
                if (errno == EINTR) continue;
                return -1;
            }
            off += w;
        }
    }
}

static int sha256_fd_path(int fd, char out[65]) {
    int p[2];
    if (pipe(p) < 0) return -1;
    pid_t pid = fork();
    if (pid < 0) {
        close(p[0]);
        close(p[1]);
        return -1;
    }
    if (pid == 0) {
        (void)dup2(p[1], STDOUT_FILENO);
        close(p[0]);
        close(p[1]);
        char path[64];
        (void)snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
        execlp("sha256sum", "sha256sum", path, (char *)NULL);
        _exit(127);
    }
    close(p[1]);
    char buf[256];
    size_t used = 0;
    while (used + 1 < sizeof(buf)) {
        ssize_t n = read(p[0], buf + used, sizeof(buf) - 1 - used);
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        used += (size_t)n;
    }
    close(p[0]);
    buf[used] = '\0';
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0 || used < 64) return -1;
    memcpy(out, buf, 64);
    out[64] = '\0';
    return is_sha256(out) ? 0 : -1;
}

static void write_installer_exit(const char *outdir, int rc) {
    if (!outdir) return;
    char path[4096];
    if (snprintf(path, sizeof(path), "%s/installer-exit.status", outdir) >= (int)sizeof(path)) return;
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "installer_exit=%d\n", rc);
    fprintf(f, "claim_allowed=false\n");
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 6 || strcmp(argv[4], "--") != 0) {
        fprintf(stderr, "usage: %s APK EXPECTED_SHA256 OUT_DIR -- COMMAND [ARGS... @APK_FD@ ...]\n", argv[0]);
        return 116;
    }

    const char *apk = argv[1];
    const char *expected = argv[2];
    const char *outdir = argv[3];
    if (!is_sha256(expected)) {
        write_status(outdir, "FAIL", "expected digest malformed", expected, NULL, -1);
        return 117;
    }

    int src = open(apk, O_RDONLY | O_CLOEXEC);
    if (src < 0) {
        write_status(outdir, "FAIL", "cannot open APK source", expected, NULL, -1);
        return 118;
    }
    struct stat st;
    if (fstat(src, &st) < 0 || !S_ISREG(st.st_mode) || st.st_size <= 0) {
        close(src);
        write_status(outdir, "FAIL", "APK source missing, non-regular, or empty", expected, NULL, -1);
        return 119;
    }

    int mfd = x_memfd_create("apkc-sealed-apk", MFD_ALLOW_SEALING);
    if (mfd < 0) {
        close(src);
        write_status(outdir, "FAIL", "memfd_create unavailable", expected, NULL, -1);
        return 120;
    }
    if (copy_all(src, mfd) < 0) {
        close(src);
        close(mfd);
        write_status(outdir, "FAIL", "copy into memfd failed", expected, NULL, -1);
        return 121;
    }
    close(src);

    if (fcntl(mfd, F_ADD_SEALS, REQUIRED_SEALS) < 0) {
        close(mfd);
        write_status(outdir, "FAIL", "cannot apply required memfd seals", expected, NULL, -1);
        return 122;
    }
    int seals = fcntl(mfd, F_GET_SEALS);
    if (seals < 0 || (seals & REQUIRED_SEALS) != REQUIRED_SEALS) {
        close(mfd);
        write_status(outdir, "FAIL", "required memfd seals not present", expected, NULL, seals);
        return 123;
    }

    if (lseek(mfd, 0, SEEK_SET) < 0) {
        close(mfd);
        write_status(outdir, "FAIL", "cannot rewind sealed memfd", expected, NULL, seals);
        return 124;
    }

    char observed[65];
    if (sha256_fd_path(mfd, observed) < 0) {
        close(mfd);
        write_status(outdir, "FAIL", "cannot hash sealed memfd snapshot", expected, NULL, seals);
        return 125;
    }
    if (strcmp(observed, expected) != 0) {
        close(mfd);
        write_status(outdir, "FAIL", "sealed snapshot digest mismatch", expected, observed, seals);
        return 126;
    }

    int cmdc = argc - 5;
    char **cmd = &argv[5];
    int replaced = 0;
    char fdpath[64];
    (void)snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", mfd);
    char **exec_argv = calloc((size_t)cmdc + 1, sizeof(char *));
    if (!exec_argv) {
        close(mfd);
        write_status(outdir, "FAIL", "argv allocation failed", expected, observed, seals);
        return 127;
    }
    for (int i = 0; i < cmdc; i++) {
        if (strcmp(cmd[i], "@APK_FD@") == 0) {
            exec_argv[i] = fdpath;
            replaced++;
        } else {
            exec_argv[i] = cmd[i];
        }
    }
    if (replaced != 1) {
        free(exec_argv);
        close(mfd);
        write_status(outdir, "FAIL", "command must contain exactly one @APK_FD@ placeholder", expected, observed, seals);
        return 128;
    }

    write_status(outdir, "READY", NULL, expected, observed, seals);

    pid_t child = fork();
    if (child < 0) {
        free(exec_argv);
        close(mfd);
        write_status(outdir, "FAIL", "cannot fork installer", expected, observed, seals);
        return 129;
    }
    if (child == 0) {
        execvp(exec_argv[0], exec_argv);
        _exit(127);
    }

    int ws = 0;
    while (waitpid(child, &ws, 0) < 0) {
        if (errno != EINTR) {
            ws = -1;
            break;
        }
    }
    int rc = 130;
    if (ws >= 0 && WIFEXITED(ws)) rc = WEXITSTATUS(ws);
    else if (ws >= 0 && WIFSIGNALED(ws)) rc = 128 + WTERMSIG(ws);

    write_installer_exit(outdir, rc);
    free(exec_argv);
    close(mfd);
    return rc;
}
