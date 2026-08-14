/* freestanding_intrinsics.h — compiler-required memory intrinsics without libc.
 *
 * ApkC links with -nostdlib. Clang/LLVM may lower aggregate copies to a memcpy
 * symbol even when the source contains no explicit memcpy call. This header is
 * force-included by proof builds so that such lowering remains freestanding,
 * deterministic and independent of hosted libc.
 */
#ifndef APKC_FREESTANDING_INTRINSICS_H
#define APKC_FREESTANDING_INTRINSICS_H

__attribute__((used,noinline))
void *memcpy(void *dst, const void *src, __SIZE_TYPE__ n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (__SIZE_TYPE__ i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}

#endif /* APKC_FREESTANDING_INTRINSICS_H */
