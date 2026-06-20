#!/usr/bin/env bash
# S08 — Build NDK separado do build MCU
# Compila RAF_041-047 para Android NDK (arm64-v8a e armeabi-v7a).
# Requer: Android NDK no caminho ANDROID_NDK_ROOT ou NDK_PATH
#
# Uso:
#   export ANDROID_NDK_ROOT=/opt/android-ndk-r26
#   bash Benchmark/build_ndk.sh                  # compila M041-M047 para ambos ABIs
#   bash Benchmark/build_ndk.sh arm64-v8a        # compila apenas arm64-v8a
#
# Saída: build_ndk/arm64-v8a/<arquivo>.so e build_ndk/armeabi-v7a/<arquivo>.so

set -euo pipefail

NDK="${ANDROID_NDK_ROOT:-${NDK_PATH:-}}"
OUTDIR="build_ndk"
API_LEVEL=21
PASS=0
FAIL=0
SKIP=0

# NDK method files
NDK_FILES=($(ls RAF_04{1,2,3,4,5,6,7}_*.c 2>/dev/null || true))

ABIS=("arm64-v8a" "armeabi-v7a")
if [ $# -gt 0 ] && [[ "$1" == "arm64-v8a" || "$1" == "armeabi-v7a" ]]; then
    ABIS=("$1")
fi

if [ -z "${NDK}" ] || [ ! -d "${NDK}" ]; then
    echo "SKIP: ANDROID_NDK_ROOT not set or not a directory."
    echo "      Set ANDROID_NDK_ROOT=/path/to/ndk and re-run."
    exit 0
fi

if [ ${#NDK_FILES[@]} -eq 0 ]; then
    echo "No NDK files found (RAF_041-047_*.c). Run from repository root."
    exit 1
fi

for abi in "${ABIS[@]}"; do
    case "${abi}" in
        arm64-v8a)
            TRIPLE="aarch64-linux-android"
            CLANG="${NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/${TRIPLE}${API_LEVEL}-clang"
            CFLAGS="-fPIC -D__ANDROID__=1 -std=c11 -Wall -O2"
            ;;
        armeabi-v7a)
            TRIPLE="armv7a-linux-androideabi"
            CLANG="${NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/${TRIPLE}${API_LEVEL}-clang"
            CFLAGS="-fPIC -D__ANDROID__=1 -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -std=c11 -Wall -O2"
            ;;
    esac

    if [ ! -x "${CLANG}" ]; then
        echo "SKIP [${abi}]: ${CLANG} not found"
        SKIP=$((SKIP + ${#NDK_FILES[@]}))
        continue
    fi

    mkdir -p "${OUTDIR}/${abi}"

    for src in "${NDK_FILES[@]}"; do
        base=$(basename "${src}" .c)
        out="${OUTDIR}/${abi}/${base}.so"
        if ${CLANG} ${CFLAGS} -I. -shared "${src}" -o "${out}" 2>/dev/null; then
            echo "PASS  [${abi}]  ${base}"
            PASS=$((PASS + 1))
        else
            echo "FAIL  [${abi}]  ${base}"
            FAIL=$((FAIL + 1))
        fi
    done
done

echo ""
echo "NDK build: ${PASS} PASS  ${FAIL} FAIL  ${SKIP} SKIP"
[ ${FAIL} -eq 0 ]
