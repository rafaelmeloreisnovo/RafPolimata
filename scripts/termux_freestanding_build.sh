#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
MODE=${1:---plan}
OUT=${OUT_DIR:-"$ROOT/build/freestanding-termux"}
API=${ANDROID_API:-21}
CC=${CC:-clang}

usage()
{
    echo "usage: $0 [--plan|--build|--run]"
}

select_tool()
{
    preferred=$1
    fallback=$2
    if command -v "$preferred" >/dev/null 2>&1; then
        command -v "$preferred"
    elif command -v "$fallback" >/dev/null 2>&1; then
        command -v "$fallback"
    else
        return 1
    fi
}

arch=$(uname -m)
case "$arch" in
    aarch64|arm64)
        abi=arm64-v8a
        target="aarch64-linux-android${API}"
        start="$ROOT/freestanding/omega/start_aarch64.S"
        ;;
    armv7l|armv8l)
        abi=armeabi-v7a
        target="armv7a-linux-androideabi${API}"
        start="$ROOT/freestanding/omega/start_armv7.S"
        ;;
    *)
        abi=UNSUPPORTED
        target=UNSUPPORTED
        start=UNSUPPORTED
        ;;
esac

mkdir -p "$OUT"
cat > "$OUT/plan.txt" <<PLAN
schema=rafpolimata.freestanding.termux.v1
host_arch=$arch
abi=$abi
target=$target
compiler=$CC
libc=none
heap=none
link=static
entry=_start
run_requires=Android_Termux_on_ARM
PLAN
cat "$OUT/plan.txt"

[ "$MODE" = "--plan" ] && exit 0
case "$MODE" in --build|--run) ;; *) usage; exit 2 ;; esac
[ "$abi" != UNSUPPORTED ] || { echo "unsupported Termux architecture: $arch" >&2; exit 3; }
command -v "$CC" >/dev/null 2>&1 || { echo "missing compiler: pkg install clang" >&2; exit 4; }
READELF=$(select_tool llvm-readelf readelf) || { echo 'missing readelf tool' >&2; exit 5; }
NM=$(select_tool llvm-nm nm) || { echo 'missing nm tool' >&2; exit 5; }
STRIP=$(select_tool llvm-strip strip) || { echo 'missing strip tool' >&2; exit 5; }

DIR="$OUT/$abi"
OBJ="$DIR/omega_core.o"
ELF="$DIR/omega_core"
mkdir -p "$DIR"

"$CC" --target="$target" -std=c11 -Oz -Wall -Wextra -Werror \
    -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -fvisibility=hidden -ffunction-sections -fdata-sections -g0 \
    -I"$ROOT/freestanding/omega" \
    -c "$ROOT/freestanding/omega/omega_core.c" -o "$OBJ"
"$CC" --target="$target" -nostdlib -static \
    -Wl,-e,_start -Wl,--gc-sections -Wl,--build-id=none \
    -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=4096 \
    "$OBJ" "$start" -o "$ELF"

needed=$($READELF -d "$ELF" 2>/dev/null | grep NEEDED || true)
[ -z "$needed" ] || { echo "unexpected dynamic dependency: $needed" >&2; exit 6; }
undefined=$($NM -u "$ELF" 2>/dev/null || true)
[ -z "$undefined" ] || { echo "undefined symbols: $undefined" >&2; exit 7; }
"$STRIP" --strip-all "$ELF"

if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$ELF" > "$ELF.sha256"
fi
printf 'TERMUX BUILD PASS abi=%s binary=%s\n' "$abi" "$ELF"

if [ "$MODE" = "--run" ]; then
    set +e
    "$ELF"
    rc=$?
    set -e
    printf 'TERMUX RUN exit=%s\n' "$rc"
    [ "$rc" -eq 0 ] || exit "$rc"
fi
