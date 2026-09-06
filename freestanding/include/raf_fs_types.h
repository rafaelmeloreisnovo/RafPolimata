#ifndef RAF_FS_TYPES_H
#define RAF_FS_TYPES_H

/* No hosted headers: these are language primitive aliases only. */
typedef unsigned char      raf_u8;
typedef signed char        raf_i8;
typedef unsigned short     raf_u16;
typedef signed short       raf_i16;
typedef unsigned int       raf_u32;
typedef signed int         raf_i32;
typedef unsigned long long raf_u64;
typedef signed long long   raf_i64;
typedef __UINTPTR_TYPE__   raf_uptr;
typedef __INTPTR_TYPE__    raf_iptr;
typedef __SIZE_TYPE__      raf_usize;

#define RAF_FS_STATIC_ASSERT(name, expr) typedef char raf_fs_static_assert_##name[(expr) ? 1 : -1]
RAF_FS_STATIC_ASSERT(u8_is_8,   sizeof(raf_u8)  == 1);
RAF_FS_STATIC_ASSERT(u16_is_16, sizeof(raf_u16) == 2);
RAF_FS_STATIC_ASSERT(u32_is_32, sizeof(raf_u32) == 4);
RAF_FS_STATIC_ASSERT(u64_is_64, sizeof(raf_u64) == 8);

#endif
