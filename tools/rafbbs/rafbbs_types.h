#ifndef RAFBBS_TYPES_H
#define RAFBBS_TYPES_H

/*
 * RafBBS authorial freestanding scalar types.
 *
 * Boundary:
 * - zero includes;
 * - zero libc / hosted runtime;
 * - zero heap / syscall;
 * - compiler is a factory tool, not a runtime dependency.
 *
 * We fail closed unless the compiler exposes an 8-bit-byte model and the
 * selected C integer widths match the binary contracts used by RafBBS.
 */
#ifndef __CHAR_BIT__
#error "RafBBS requires a compiler-provided __CHAR_BIT__ constant"
#endif
#if __CHAR_BIT__ != 8
#error "RafBBS requires 8-bit bytes"
#endif

typedef unsigned char RafU8;
typedef unsigned int RafU32;
typedef unsigned long long RafU64;

typedef char rafbbs_u8_must_be_1_byte[(sizeof(RafU8) == 1u) ? 1 : -1];
typedef char rafbbs_u32_must_be_4_bytes[(sizeof(RafU32) == 4u) ? 1 : -1];
typedef char rafbbs_u64_must_be_8_bytes[(sizeof(RafU64) == 8u) ? 1 : -1];

#endif
