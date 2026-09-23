#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int mk(const char *p) {
    if (mkdir(p, 0755) == 0 || errno == EEXIST) return 0;
    perror(p); return -1;
}

static int copy_file(const char *src, const char *dst, mode_t mode) {
    char buf[8192];
    ssize_t n;
    int in = open(src, O_RDONLY);
    int out;
    if (in < 0) { perror(src); return -1; }
    out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (out < 0) { perror(dst); close(in); return -1; }
    while ((n = read(in, buf, sizeof buf)) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));
            if (w < 0) { perror(dst); close(in); close(out); return -1; }
            off += w;
        }
    }
    if (n < 0) { perror(src); close(in); close(out); return -1; }
    if (fsync(out) != 0) { perror(dst); close(in); close(out); return -1; }
    close(in); close(out); return 0;
}

int main(int argc, char **argv) {
    char share[1024], regdir[1024], poldir[1024], dstreg[1024], dstpol[1024];
    const char *prefix = argc > 1 ? argv[1] : "./zipraf-install";
    if (strlen(prefix) > 800) { fputs("prefix too long\n", stderr); return 2; }
    printf("\x1b[36mZIPRAF BBS INSTALL V1\x1b[0m\n");
    printf("prefix=%s\n", prefix);
    snprintf(share,sizeof share,"%s/share",prefix);
    snprintf(regdir,sizeof regdir,"%s/share/zipraf/registry",prefix);
    snprintf(poldir,sizeof poldir,"%s/share/zipraf/policy",prefix);
    if (mk(prefix)||mk(share)) return 3;
    { char z[1024]; snprintf(z,sizeof z,"%s/share/zipraf",prefix); if (mk(z)) return 3; }
    if (mk(regdir)||mk(poldir)) return 3;
    snprintf(dstreg,sizeof dstreg,"%s/crypto_algorithms.json",regdir);
    snprintf(dstpol,sizeof dstpol,"%s/ZIPRAF_FS_V1.md",poldir);
    if (copy_file("registry/crypto_algorithms.json", dstreg, 0644)) return 4;
    if (copy_file("fs/ZIPRAF_FS_V1.md", dstpol, 0644)) return 4;
    puts("INSTALL_PASS files=2 network_fetch=0 privilege_escalation=0");
    return 0;
}
