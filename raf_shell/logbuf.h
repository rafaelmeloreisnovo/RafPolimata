/* logbuf.h — static ring buffer log + file append.
 * No malloc. 256 lines × 160 chars. CRC32 chain-of-custody support. */
#pragma once
#include "sys_host.h"
#include "str_util.h"

#define LOG_LINES   256
#define LOG_COLS    160
#define LOG_FILE    "raf_bench.log"

static char _log_buf[LOG_LINES][LOG_COLS];
static int  _log_head  = 0;   /* next write position */
static int  _log_count = 0;   /* total lines ever written */

/* monotonic start time for relative timestamps */
static i64 _log_t0 = 0;

static inline void log_init(void) {
    struct timespec ts; os_clock_gettime(CLOCK_MONOTONIC,&ts);
    _log_t0 = ts.tv_sec;
}

/* elapsed seconds since log_init() */
static inline i64 log_elapsed(void) {
    struct timespec ts; os_clock_gettime(CLOCK_MONOTONIC,&ts);
    return ts.tv_sec - _log_t0;
}

/* push one line (truncated to LOG_COLS-1) prefixed with [MM:SS] */
static inline void log_push(const char *s) {
    i64 el = log_elapsed();
    i64 mm = el/60, ss = el%60;
    char *dst = _log_buf[_log_head % LOG_LINES];
    _zero(dst, LOG_COLS);
    /* build "[MM:SS] " prefix */
    sz pos=0;
    char tmp[8];
    dst[pos++]='[';
    if(mm<10){dst[pos++]='0';} sz ml=_itoa(tmp,mm); for(sz i=0;i<ml;i++) dst[pos++]=tmp[i];
    dst[pos++]=':';
    if(ss<10){dst[pos++]='0';} sz sl=_itoa(tmp,ss); for(sz i=0;i<sl;i++) dst[pos++]=tmp[i];
    dst[pos++]=']'; dst[pos++]=' ';
    /* append message */
    sz i=0;
    while(pos+1<(sz)LOG_COLS && s[i]) dst[pos++]=s[i++];
    dst[pos]=0;
    _log_head = (_log_head+1) % LOG_LINES;
    _log_count++;
}

/* append a key=value pair after a label */
static inline void log_kv(const char *label, const char *key, i64 val) {
    char line[LOG_COLS]; sz p=0;
    sz i=0; while(p+1<LOG_COLS && label[i]) line[p++]=label[i++];
    line[p++]=' '; i=0; while(p+1<LOG_COLS && key[i]) line[p++]=key[i++];
    line[p++]='=';
    char nb[22]; sz nl=_itoa(nb,val);
    i=0; while(p+1<LOG_COLS && i<nl) line[p++]=nb[i++];
    line[p]=0;
    log_push(line);
}

/* append all log lines to LOG_FILE */
static inline void log_flush_file(void) {
    i32 fd = os_open(LOG_FILE, O_WRONLY|O_CREAT|O_APPEND, 0644);
    if(fd<0) return;
    int start = _log_count > LOG_LINES ? (_log_head) : 0;
    int total = _log_count > LOG_LINES ? LOG_LINES : _log_count;
    for(int i=0;i<total;i++){
        const char *line = _log_buf[(start+i)%LOG_LINES];
        os_write(fd,line,_len(line));
        os_write(fd,"\n",1);
    }
    os_write(fd,"---\n",4);
    os_close(fd);
}

/* return pointer to log line[idx] from the visible tail (0=oldest visible) */
static inline const char *log_line(int idx, int visible) {
    int total = _log_count > LOG_LINES ? LOG_LINES : _log_count;
    if(total==0) return "";
    /* show last 'visible' lines */
    int start = total > visible ? total - visible : 0;
    int rel = start + idx;
    if(rel>=total) return "";
    int base = _log_count > LOG_LINES ? _log_head : 0;
    return _log_buf[(base+rel)%LOG_LINES];
}
