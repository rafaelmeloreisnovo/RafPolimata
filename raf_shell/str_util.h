/* str_util.h — string/IO utilities, no libc. All ops via os_write().
 * Branch-free where possible. No heap. */
#pragma once
#include "sys_host.h"

#define STDOUT 1
#define STDERR 2

/* ── length ──────────────────────────────────────────────────────────── */
static inline sz _len(const char *s) {
    sz n=0; while(s[n]) n++; return n;
}

/* ── copy (bounded, always NUL-terminates) ──────────────────────────── */
static inline void _copy(char *dst, const char *src, sz cap) {
    sz i=0;
    while(i+1<cap && src[i]) { dst[i]=src[i]; i++; }
    dst[i]=0;
}

/* ── append src into dst[*pos..cap) ────────────────────────────────── */
static inline void _cat(char *dst, sz *pos, sz cap, const char *src) {
    while(*pos+1<cap && *src) { dst[(*pos)++]=*src++; }
    dst[*pos]=0;
}

/* ── zero fill ───────────────────────────────────────────────────────── */
static inline void _zero(void *p, sz n) {
    u8 *b=(u8*)p; while(n--) *b++=0;
}

/* ── compare (returns 0 if equal) ───────────────────────────────────── */
static inline int _eq(const char *a, const char *b) {
    while(*a && *a==*b) { a++; b++; }
    return *a==*b;
}

/* ── write string to fd ──────────────────────────────────────────────── */
static inline void _puts_fd(i32 fd, const char *s) { os_write(fd,s,_len(s)); }
static inline void _puts(const char *s)             { _puts_fd(STDOUT,s); }
static inline void _putsn(const char *s, sz n)      { os_write(STDOUT,s,n); }
static inline void _err(const char *s)              { _puts_fd(STDERR,s); }

/* ── integer → decimal string (buf must be ≥21 bytes) ─────────────── */
static inline sz _itoa(char *buf, i64 v) {
    if(v==0){buf[0]='0';buf[1]=0;return 1;}
    char tmp[22]; sz i=0;
    i64 neg=(v<0); if(neg) v=-v;
    while(v){tmp[i++]=(char)('0'+(v%10));v/=10;}
    if(neg) tmp[i++]='-';
    sz j=0; while(i--) buf[j++]=tmp[i]; buf[j]=0;
    return j;
}

/* ── integer → hex string (no prefix, lowercase) ────────────────────── */
static inline sz _xtoa(char *buf, u64 v, int pad) {
    char hex[]="0123456789abcdef";
    char tmp[17]; int i=0;
    if(!v){tmp[i++]='0';}
    else{while(v){tmp[i++]=hex[v&0xf];v>>=4;}}
    /* pad with zeros */
    while(i<pad) tmp[i++]='0';
    sz j=0; while(i--) buf[j++]=tmp[i]; buf[j]=0;
    return j;
}

/* ── print decimal integer ──────────────────────────────────────────── */
static inline void _puti(i64 v) { char b[22]; _itoa(b,v); _puts(b); }

/* ── print hex integer ──────────────────────────────────────────────── */
static inline void _putx(u64 v) { char b[20]; _xtoa(b,v,1); _puts(b); }

/* ── copy file extension from path ─────────────────────────────────── */
static inline const char *_ext(const char *path) {
    const char *e=NULL, *p=path;
    while(*p){if(*p=='.')e=p;p++;}
    return e ? e : "";
}

/* ── basename (last component after /) ─────────────────────────────── */
static inline const char *_base(const char *path) {
    const char *b=path, *p=path;
    while(*p){if(*p=='/') b=p+1; p++;}
    return b;
}

/* ── path join: out = dir + "/" + name ──────────────────────────────── */
static inline void _path_join(char *out, sz cap, const char *dir, const char *name) {
    sz pos=0;
    _cat(out,&pos,cap,dir);
    if(pos && out[pos-1]!='/') { out[pos++]='/'; out[pos]=0; }
    _cat(out,&pos,cap,name);
}

/* ── write newline ──────────────────────────────────────────────────── */
static inline void _nl(void) { os_write(STDOUT,"\n",1); }
