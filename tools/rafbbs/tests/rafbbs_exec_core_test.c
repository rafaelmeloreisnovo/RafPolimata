#include "rafbbs_exec_core.h"

static int raf_exec_text_eq(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

int rafbbs_exec_core_test(void)
{
    RafExecSpec spec;

    if (!raf_exec_spec_fill(RAF_EXEC_ENCODERS_PY, &spec))
        return 1;
    if (spec.argc != 2u ||
        !raf_exec_text_eq(spec.argv[0], "python3") ||
        !raf_exec_text_eq(spec.argv[1], "tests/test_arm64_encoders.py") ||
        spec.argv[2] != (const char *)0)
        return 2;

    if (!raf_exec_spec_fill(RAF_EXEC_ENCODER_CC, &spec))
        return 3;
    if (spec.argc != 10u ||
        !raf_exec_text_eq(spec.argv[0], "cc") ||
        !raf_exec_text_eq(spec.argv[5], "-I") ||
        !raf_exec_text_eq(spec.argv[6], "Apkc") ||
        !raf_exec_text_eq(spec.argv[9], "/tmp/test_arm64_encoders") ||
        spec.argv[10] != (const char *)0)
        return 4;

    if (!raf_exec_spec_fill(RAF_EXEC_ENCODER_BIN, &spec) ||
        spec.argc != 1u ||
        !raf_exec_text_eq(spec.argv[0], "/tmp/test_arm64_encoders"))
        return 5;

    if (!raf_exec_spec_fill(RAF_EXEC_ROUNDTRIP, &spec) ||
        !raf_exec_text_eq(spec.argv[0], "sh"))
        return 6;

    if (!raf_exec_spec_fill(RAF_EXEC_APKC_VALIDATE, &spec) ||
        !raf_exec_text_eq(spec.argv[1], "scripts/apkc_validate.sh"))
        return 7;

    if (!raf_exec_spec_fill(RAF_EXEC_PROOF_CHAIN, &spec) ||
        !raf_exec_text_eq(spec.argv[0], "bash"))
        return 8;

    if (raf_exec_spec_fill(RAF_EXEC_INVALID, &spec) != 0u ||
        spec.argc != 0u ||
        spec.argv[0] != (const char *)0)
        return 9;

    return 0;
}

#ifdef RAFBBS_EXEC_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_exec_core_test();
}
#endif
