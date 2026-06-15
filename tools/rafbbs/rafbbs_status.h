#ifndef RAFBBS_STATUS_H
#define RAFBBS_STATUS_H

typedef enum {
    RAF_PASS = 0,
    RAF_FAIL = 1,
    RAF_STEP = 2,
    RAF_INFO = 3,
    RAF_WARN = 4,
    RAF_AUDIT = 5,
    RAF_SKIP = 6,
    RAF_RUNTIME = 7,
    RAF_REFERENCE = 8,
    RAF_PENDING = 9,
    RAF_TOKEN_VAZIO = 10,
    RAF_HASH = 11,
    RAF_DONE = 12,
    RAF_PASS_LIMITED = 13
} RafStatus;

static const char *raf_status_name(RafStatus s) {
    switch (s) {
        case RAF_PASS: return "PASS";
        case RAF_FAIL: return "FAIL";
        case RAF_STEP: return "STEP";
        case RAF_INFO: return "INFO";
        case RAF_WARN: return "WARN";
        case RAF_AUDIT: return "AUDIT";
        case RAF_SKIP: return "SKIP";
        case RAF_RUNTIME: return "RUNTIME";
        case RAF_REFERENCE: return "REFERENCE";
        case RAF_PENDING: return "PENDING";
        case RAF_TOKEN_VAZIO: return "TOKEN_VAZIO";
        case RAF_HASH: return "HASH";
        case RAF_DONE: return "DONE";
        case RAF_PASS_LIMITED: return "PASS_LIMITED";
        default: return "UNKNOWN";
    }
}

#endif
