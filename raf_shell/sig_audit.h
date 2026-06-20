/* sig_audit.h — CRC32 signature + chain-of-custody .sig file writer.
 * No libc. No malloc. Proof of provenance for every pipeline output. */
#pragma once
#include "sys_host.h"
#include "str_util.h"

/* ── CRC32 (ISO 3309 / ITU-T V.42) ─────────────────────────────── */
static inline u32 _crc32_step(u32 crc, u8 b) {
    crc ^= b;
    for(int i=0;i<8;i++)
        crc = (crc>>1) ^ (u32)(0xEDB88320u & (u32)(-(i32)(crc&1u)));
    return crc;
}

static inline u32 crc32_buf(const u8 *buf, sz n) {
    u32 crc = 0xFFFFFFFFu;
    while(n--) crc = _crc32_step(crc,*buf++);
    return crc ^ 0xFFFFFFFFu;
}

/* CRC32 of a file; returns 0 on error */
static inline u32 crc32_file(const char *path) {
    i32 fd = os_open(path, O_RDONLY, 0);
    if(fd<0) return 0;
    static u8 fbuf[4096];
    u32 crc=0xFFFFFFFFu; i64 n;
    while((n=os_read(fd,fbuf,sizeof fbuf))>0)
        for(i64 i=0;i<n;i++) crc=_crc32_step(crc,fbuf[i]);
    os_close(fd);
    return crc ^ 0xFFFFFFFFu;
}

/* Write <filepath>.sig containing:
 *   MODULE:<name>
 *   FILE:<path>
 *   CRC32:<hex>
 *   TIME:<elapsed_sec>
 *   STATUS:OK
 */
static inline int sig_write(const char *filepath, const char *module_name, i64 elapsed_sec) {
    /* build .sig filename */
    char sigpath[512]; sz p=0;
    _cat(sigpath,&p,sizeof sigpath,filepath);
    _cat(sigpath,&p,sizeof sigpath,".sig");

    u32 crc = crc32_file(filepath);

    i32 fd = os_open(sigpath, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fd<0) return -1;

    char line[256]; sz lp;

    /* MODULE line */
    lp=0; _cat(line,&lp,sizeof line,"MODULE:"); _cat(line,&lp,sizeof line,module_name); os_write(fd,line,lp); os_write(fd,"\n",1);

    /* FILE line */
    lp=0; _cat(line,&lp,sizeof line,"FILE:"); _cat(line,&lp,sizeof line,filepath); os_write(fd,line,lp); os_write(fd,"\n",1);

    /* CRC32 line */
    char hx[12]; _xtoa(hx,(u64)crc,8);
    lp=0; _cat(line,&lp,sizeof line,"CRC32:0x"); _cat(line,&lp,sizeof line,hx); os_write(fd,line,lp); os_write(fd,"\n",1);

    /* TIME line */
    char ts[22]; _itoa(ts,elapsed_sec);
    lp=0; _cat(line,&lp,sizeof line,"TIME:"); _cat(line,&lp,sizeof line,ts); _cat(line,&lp,sizeof line,"s"); os_write(fd,line,lp); os_write(fd,"\n",1);

    os_write(fd,"STATUS:OK\n",10);
    os_close(fd);
    return 0;
}

/* Print .sig contents to stdout */
static inline void sig_show(const char *filepath) {
    char sigpath[512]; sz p=0;
    _cat(sigpath,&p,sizeof sigpath,filepath);
    _cat(sigpath,&p,sizeof sigpath,".sig");
    i32 fd=os_open(sigpath,O_RDONLY,0);
    if(fd<0){ _puts("(no .sig file)\n"); return; }
    static char buf[1024]; i64 n;
    while((n=os_read(fd,buf,sizeof buf))>0) os_write(1,buf,(sz)n);
    os_close(fd);
}
