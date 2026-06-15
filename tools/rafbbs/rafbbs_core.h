#ifndef RAFBBS_CORE_H
#define RAFBBS_CORE_H
#include <stdint.h>
#include <time.h>
#include "rafbbs_status.h"
#include "rafbbs_freestanding.h"

typedef struct {
    char run_id[32];
    char log_path[256];
    char manifest_path[256];
    char bin_manifest_path[256];
    char pipeline[64];
    char input[256];
    char output[256];
    char command[512];
    char branch[128];
    char commit[128];
    char host[128];
    char arch[64];
    char gaps[512];
    char input_sha256[65];
    char output_sha256[65];
    RafWatchdog watchdog;
    RafRollbackRing rollback;
    uint32_t input_crc32;
    uint32_t output_crc32;
    uint32_t hash_state;
    RafStatus final_status;
    int limited;
    int failed;
    int syslog_count;
    struct timespec start;
} RafContext;

static int raf_is_arm_host(void) {
#if defined(__aarch64__) || defined(__arm__)
    return 1;
#else
    return 0;
#endif
}
#endif
