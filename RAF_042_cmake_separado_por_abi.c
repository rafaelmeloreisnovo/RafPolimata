#include "RAF_rafaelia_common.h"

/*
 * Método M042: CMake separado por ABI
 * Alvo: Android NDK
 * Domínio: Build
 * Ganho estimado: organização + flags ótimas por arquitetura
 *
 * Demonstra configuração CMake por ABI embarcada como constante de string.
 * Self-test verifica presença dos marcadores ABI no template.
 */

/* Canonical CMakeLists.txt content for per-ABI NDK builds — embedded as
 * documentation and auditable at runtime. This is the intended file content
 * to be placed in the Android project's CMakeLists.txt. */
static const char _m042_cmake_template[] =
    "cmake_minimum_required(VERSION 3.22)\n"
    "project(RafNative C)\n"
    "\n"
    "add_library(rafnative SHARED rafnative.c)\n"
    "\n"
    "if(ANDROID_ABI STREQUAL \"arm64-v8a\")\n"
    "  target_compile_options(rafnative PRIVATE\n"
    "    -march=armv8.2-a+crypto+crc\n"
    "    -O3 -ffast-math)\n"
    "elseif(ANDROID_ABI STREQUAL \"armeabi-v7a\")\n"
    "  target_compile_options(rafnative PRIVATE\n"
    "    -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp\n"
    "    -O2)\n"
    "elseif(ANDROID_ABI STREQUAL \"x86_64\")\n"
    "  target_compile_options(rafnative PRIVATE\n"
    "    -march=x86-64 -msse4.2 -O2)\n"
    "endif()\n"
    "\n"
    "target_link_libraries(rafnative log)\n";

int rafaelia_m042_cmake_separado_por_abi(void)
{
    /* Scan the template for required ABI markers */
    const char *p = _m042_cmake_template;
    int found_arm64 = 0;
    int found_arm32 = 0;

    while (*p) {
        /* "arm64" */
        if (p[0] == 'a' && p[1] == 'r' && p[2] == 'm' &&
            p[3] == '6' && p[4] == '4') {
            found_arm64 = 1;
        }
        /* "armeabi" (armeabi-v7a) */
        if (p[0] == 'a' && p[1] == 'r' && p[2] == 'm' &&
            p[3] == 'e' && p[4] == 'a') {
            found_arm32 = 1;
        }
        p++;
    }

    return (found_arm64 && found_arm32) ? 0 : -1;
}
