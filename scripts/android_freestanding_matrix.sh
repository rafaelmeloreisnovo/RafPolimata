#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
MODE=${1:---plan}
OUT=${OUT_DIR:-"$ROOT/build/freestanding-android"}
API=${ANDROID_API:-21}
NDK_VERSION=${ANDROID_NDK_VERSION:-27.2.12479018}

usage()
{
    echo "usage: $0 [--plan|--build|--verify]"
}

host_tag()
{
    os=$(uname -s)
    machine=$(uname -m)
    case "$os:$machine" in
        Linux:x86_64) echo linux-x86_64 ;;
        Linux:aarch64) echo linux-aarch64 ;;
        Darwin:x86_64) echo darwin-x86_64 ;;
        Darwin:arm64) echo darwin-x86_64 ;;
        *) return 1 ;;
    esac
}

resolve_ndk()
{
    for candidate in \
        "${ANDROID_NDK_HOME:-}" \
        "${ANDROID_NDK_ROOT:-}" \
        "${ANDROID_NDK_LATEST_HOME:-}" \
        "${ANDROID_SDK_ROOT:-}/ndk/$NDK_VERSION" \
        "${ANDROID_HOME:-}/ndk/$NDK_VERSION"; do
        if [ -n "$candidate" ] && [ -d "$candidate/toolchains/llvm/prebuilt" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    for base in "${ANDROID_SDK_ROOT:-}/ndk" "${ANDROID_HOME:-}/ndk"; do
        [ -d "$base" ] || continue
        candidate=$(find "$base" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | sort | tail -n 1)
        if [ -n "$candidate" ] && [ -d "$candidate/toolchains/llvm/prebuilt" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

emit_plan()
{
    mkdir -p "$OUT"
    cat > "$OUT/plan.txt" <<PLAN
schema=rafpolimata.freestanding.android.v1
android_api=$API
libc=none
heap=none
runtime=linux_syscall_exit_only
profile=armeabi-v7a;target=armv7a-linux-androideabi${API};entry=start_armv7.S
profile=arm64-v8a;target=aarch64-linux-android${API};entry=start_aarch64.S
verification=no_NEEDED;no_undefined;static_ELF;stripped_release;sha256
PLAN
    cat "$OUT/plan.txt"
}

verify_one()
{
    abi=$1
    binary=$2
    readelf_tool=$3
    nm_tool=$4

    [ -f "$binary" ] || { echo "missing binary: $binary" >&2; return 1; }
    needed=$($readelf_tool -d "$binary" 2>/dev/null | grep NEEDED || true)
    [ -z "$needed" ] || { echo "dynamic dependency in $abi: $needed" >&2; return 1; }
    undefined=$($nm_tool -u "$binary" 2>/dev/null || true)
    [ -z "$undefined" ] || { echo "undefined symbols in $abi: $undefined" >&2; return 1; }
    type=$($readelf_tool -h "$binary" | awk -F: '/Type:/{gsub(/^[[:space:]]+/,"",$2); print $2; exit}')
    case "$type" in
        EXEC*) ;;
        *) echo "unexpected ELF type for $abi: $type" >&2; return 1 ;;
    esac
    printf 'VERIFIED %s %s\n' "$abi" "$binary"
}

emit_plan
[ "$MODE" = "--plan" ] && exit 0
case "$MODE" in --build|--verify) ;; *) usage; exit 2 ;; esac

NDK=$(resolve_ndk) || {
    echo "ANDROID NDK NOT FOUND: set ANDROID_NDK_HOME or install ndk;$NDK_VERSION" >&2
    exit 3
}
TAG=$(host_tag) || { echo 'unsupported NDK host' >&2; exit 4; }
TOOLBIN="$NDK/toolchains/llvm/prebuilt/$TAG/bin"
READELF="$TOOLBIN/llvm-readelf"
NM="$TOOLBIN/llvm-nm"
STRIP="$TOOLBIN/llvm-strip"
SIZE="$TOOLBIN/llvm-size"

for tool in "$READELF" "$NM" "$STRIP" "$SIZE"; do
    [ -x "$tool" ] || { echo "missing NDK tool: $tool" >&2; exit 5; }
done

manifest="$OUT/manifest.sha256"
: > "$manifest"

for abi in armeabi-v7a arm64-v8a; do
    case "$abi" in
        armeabi-v7a)
            target="armv7a-linux-androideabi${API}"
            cc="$TOOLBIN/$target-clang"
            start="$ROOT/freestanding/omega/start_armv7.S"
            ;;
        arm64-v8a)
            target="aarch64-linux-android${API}"
            cc="$TOOLBIN/$target-clang"
            start="$ROOT/freestanding/omega/start_aarch64.S"
            ;;
    esac
    dir="$OUT/$abi"
    obj="$dir/omega_core.o"
    elf="$dir/omega_core"
    release="$dir/omega_core.stripped"
    mkdir -p "$dir"

    if [ "$MODE" = "--build" ]; then
        [ -x "$cc" ] || { echo "missing compiler: $cc" >&2; exit 6; }
        "$cc" -std=c11 -Oz -Wall -Wextra -Werror \
            -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
            -fno-unwind-tables -fno-asynchronous-unwind-tables \
            -fvisibility=hidden -ffunction-sections -fdata-sections -g0 \
            -I"$ROOT/freestanding/omega" \
            -c "$ROOT/freestanding/omega/omega_core.c" -o "$obj"
        "$cc" -nostdlib -static \
            -Wl,-e,_start -Wl,--gc-sections -Wl,--build-id=none \
            -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=4096 \
            "$obj" "$start" -o "$elf"
        verify_one "$abi" "$elf" "$READELF" "$NM"
        cp "$elf" "$release"
        "$STRIP" --strip-all "$release"
        verify_one "$abi" "$release" "$READELF" "$NM"
        "$SIZE" "$elf" > "$dir/size.txt"
    else
        verify_one "$abi" "$elf" "$READELF" "$NM"
        verify_one "$abi" "$release" "$READELF" "$NM"
    fi

    if command -v sha256sum >/dev/null 2>&1; then
        (cd "$OUT" && sha256sum "$abi/omega_core" "$abi/omega_core.stripped") >> "$manifest"
    else
        (cd "$OUT" && shasum -a 256 "$abi/omega_core" "$abi/omega_core.stripped") >> "$manifest"
    fi
done

printf 'ANDROID FREESTANDING MATRIX PASS ndk=%s output=%s\n' "$NDK" "$OUT"
