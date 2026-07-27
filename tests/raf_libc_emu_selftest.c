#include "Apkc/raf_libc_emu.h"

static int check_bytes(const char *a, const char *b, size_t n) {
    return memcmp(a, b, n) == 0;
}

int main(void) {
    char right[8] = "abcdef";
    char left[8] = "abcdef";
    char copy[8];
    char *end = (char *)0;

    memmove(right + 1, right, 5u);
    if (!check_bytes(right, "aabcde", 6u)) return 1;

    memmove(left, left + 1, 5u);
    if (!check_bytes(left, "bcdeff", 6u)) return 2;

    memset(copy, 0, sizeof(copy));
    strcpy(copy, "raf");
    if (strlen(copy) != 3u || strcmp(copy, "raf") != 0) return 3;
    if (memchr(copy, 'a', 3u) != copy + 1) return 4;

    if (strtoul("0", &end, 0) != 0ul || end == (char *)0 || *end != '\0') return 5;
    if (strtoul("0xffz", &end, 0) != 255ul || *end != 'z') return 6;
    if (strtoul("-2", &end, 10) != (0ul - 2ul) || *end != '\0') return 7;
    if (strtoul("10", &end, 1) != 0ul || end == (char *)0 || end[0] != '1') return 8;

    strncpy(copy, "xy", sizeof(copy));
    if (copy[0] != 'x' || copy[1] != 'y' || copy[2] != '\0') return 9;
    return 0;
}
