#ifndef RAFBBS_HOST_H
#define RAFBBS_HOST_H
#ifndef RAFBBS_FREESTANDING_MODE
#include <stdlib.h>
static inline int raf_host_exec(const char *cmd) { return system(cmd); }
#else
static inline int raf_host_exec(const char *cmd) { (void)cmd; return 127; }
#endif
#endif
