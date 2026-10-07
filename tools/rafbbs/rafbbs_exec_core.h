#ifndef RAFBBS_EXEC_CORE_H
#define RAFBBS_EXEC_CORE_H

#include "rafbbs_types.h"

#define RAFBBS_EXEC_MAX_ARGS 12u

typedef enum {
    RAF_EXEC_INVALID = 0,
    RAF_EXEC_ENCODERS_PY = 1,
    RAF_EXEC_ENCODER_CC = 2,
    RAF_EXEC_ENCODER_BIN = 3,
    RAF_EXEC_ROUNDTRIP = 4,
    RAF_EXEC_APKC_VALIDATE = 5,
    RAF_EXEC_PROOF_CHAIN = 6
} RafExecId;

typedef struct {
    const char *display;
    RafU32 argc;
    const char *argv[RAFBBS_EXEC_MAX_ARGS + 1u];
} RafExecSpec;

static inline RafU32 raf_exec_spec_fill(RafExecId id, RafExecSpec *spec)
{
    if (spec == (RafExecSpec *)0)
        return 0u;

    spec->display = (const char *)0;
    spec->argc = 0u;
    spec->argv[0] = (const char *)0;

    if (id == RAF_EXEC_ENCODERS_PY) {
        spec->display = "python3 tests/test_arm64_encoders.py";
        spec->argc = 2u;
        spec->argv[0] = "python3";
        spec->argv[1] = "tests/test_arm64_encoders.py";
        spec->argv[2] = (const char *)0;
        return 1u;
    }

    if (id == RAF_EXEC_ENCODER_CC) {
        spec->display = "cc -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o /tmp/test_arm64_encoders";
        spec->argc = 10u;
        spec->argv[0] = "cc";
        spec->argv[1] = "-std=c11";
        spec->argv[2] = "-Wall";
        spec->argv[3] = "-Wextra";
        spec->argv[4] = "-Werror";
        spec->argv[5] = "-I";
        spec->argv[6] = "Apkc";
        spec->argv[7] = "tests/test_arm64_encoders.c";
        spec->argv[8] = "-o";
        spec->argv[9] = "/tmp/test_arm64_encoders";
        spec->argv[10] = (const char *)0;
        return 1u;
    }

    if (id == RAF_EXEC_ENCODER_BIN) {
        spec->display = "/tmp/test_arm64_encoders";
        spec->argc = 1u;
        spec->argv[0] = "/tmp/test_arm64_encoders";
        spec->argv[1] = (const char *)0;
        return 1u;
    }

    if (id == RAF_EXEC_ROUNDTRIP) {
        spec->display = "sh tests/test_asm_roundtrip.sh";
        spec->argc = 2u;
        spec->argv[0] = "sh";
        spec->argv[1] = "tests/test_asm_roundtrip.sh";
        spec->argv[2] = (const char *)0;
        return 1u;
    }

    if (id == RAF_EXEC_APKC_VALIDATE) {
        spec->display = "sh scripts/apkc_validate.sh";
        spec->argc = 2u;
        spec->argv[0] = "sh";
        spec->argv[1] = "scripts/apkc_validate.sh";
        spec->argv[2] = (const char *)0;
        return 1u;
    }

    if (id == RAF_EXEC_PROOF_CHAIN) {
        spec->display = "bash scripts/capture_android_proof_chain.sh";
        spec->argc = 2u;
        spec->argv[0] = "bash";
        spec->argv[1] = "scripts/capture_android_proof_chain.sh";
        spec->argv[2] = (const char *)0;
        return 1u;
    }

    return 0u;
}

#endif
