#!/usr/bin/env sh
set -eu

mode="${1:---plan}"
out_dir="${OUT_DIR:-build_host_check/android_matrix}"
api="${ANDROID_API:-28}"
mkdir -p "$out_dir"
plan="$out_dir/plan.txt"

cat > "$plan" <<PLAN
schema=1
android_min_api=$api
android_versions=9-16+
profile=armeabi-v7a;triple=armv7a-linux-androideabi;arch=arm32;isa=neon;install=lib/armeabi-v7a/raf_compile
profile=arm64-v8a;triple=aarch64-linux-android;arch=arm64;isa=simd;install=lib/arm64-v8a/raf_compile
PLAN

printf 'ANDROID PLAN %s\n' "$plan"
cat "$plan"

if [ "$mode" = "--plan" ]; then
  exit 0
fi
if [ "$mode" != "--build" ]; then
  echo "usage: $0 [--plan|--build]"
  exit 2
fi

ndk="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"
if [ -z "$ndk" ]; then
  echo "SKIP android build: ANDROID_NDK_HOME/ANDROID_NDK_ROOT not set"
  exit 0
fi
host_tag="linux-x86_64"
toolbin="$ndk/toolchains/llvm/prebuilt/$host_tag/bin"
if [ ! -d "$toolbin" ]; then
  echo "SKIP android build: missing $toolbin"
  exit 0
fi

srcs="raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c"
for abi in armeabi-v7a arm64-v8a; do
  case "$abi" in
    armeabi-v7a) cc="$toolbin/armv7a-linux-androideabi${api}-clang" ;;
    arm64-v8a) cc="$toolbin/aarch64-linux-android${api}-clang" ;;
  esac
  if [ ! -x "$cc" ]; then
    echo "SKIP $abi: missing compiler $cc"
    continue
  fi
  abi_dir="$out_dir/$abi"
  mkdir -p "$abi_dir"
  "$cc" -std=c11 -Wall -Wextra -Werror -Os -fPIE -pie \
    -ffreestanding -fno-builtin $srcs -o "$abi_dir/raf_compile"
  "$abi_dir/raf_compile" --help >/dev/null
  printf 'BUILT %s %s\n' "$abi" "$abi_dir/raf_compile"
done
