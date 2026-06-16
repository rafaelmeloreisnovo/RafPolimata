/* tty.h — terminal raw mode, VT100 helpers, ANSI palette.
 * No libc. Cyberpunk BBS color scheme. */
#pragma once
#include "sys_host.h"
#include "str_util.h"

/* ── saved terminal state ────────────────────────────────────────── */
static struct termios _tty_saved;
static int            _tty_raw_active = 0;

static inline void tty_raw(void) {
    os_ioctl(0, TCGETS, &_tty_saved);
    struct termios t = _tty_saved;
    t.c_lflag &= ~(u32)(ECHO | ICANON | ISIG);
    t.c_iflag &= ~(u32)(IXON);
    t.c_cc[VMIN]  = 1;
    t.c_cc[VTIME] = 0;
    os_ioctl(0, TCSETS, &t);
    _tty_raw_active = 1;
}

static inline void tty_restore(void) {
    if (_tty_raw_active) {
        os_ioctl(0, TCSETS, &_tty_saved);
        _tty_raw_active = 0;
    }
}

static inline void tty_size(int *rows, int *cols) {
    struct winsize w;
    _zero(&w, sizeof w);
    os_ioctl(1, TIOCGWINSZ, &w);
    *rows = w.ws_row ? w.ws_row : 24;
    *cols = w.ws_col ? w.ws_col : 80;
}

/* ── VT100 / ANSI escape macros ─────────────────────────────────── */
#define VT_RESET    "\033[0m"
#define VT_BOLD     "\033[1m"
#define VT_DIM      "\033[2m"
#define VT_BLINK    "\033[5m"
#define VT_REV      "\033[7m"

/* foreground colors */
#define VT_BLACK    "\033[30m"
#define VT_RED      "\033[31m"
#define VT_GREEN    "\033[32m"
#define VT_YELLOW   "\033[33m"
#define VT_BLUE     "\033[34m"
#define VT_MAGENTA  "\033[35m"
#define VT_CYAN     "\033[36m"
#define VT_WHITE    "\033[37m"

/* bright foreground */
#define VT_BRED     "\033[1;31m"
#define VT_BGREEN   "\033[1;32m"
#define VT_BYELLOW  "\033[1;33m"
#define VT_BBLUE    "\033[1;34m"
#define VT_BMAGENTA "\033[1;35m"
#define VT_BCYAN    "\033[1;36m"
#define VT_BWHITE   "\033[1;37m"

/* background colors */
#define VT_BG_BLACK   "\033[40m"
#define VT_BG_RED     "\033[41m"
#define VT_BG_GREEN   "\033[42m"
#define VT_BG_BLUE    "\033[44m"
#define VT_BG_MAGENTA "\033[45m"
#define VT_BG_CYAN    "\033[46m"

/* cursor / screen */
#define VT_CLS       "\033[2J\033[H"
#define VT_HIDE_CUR  "\033[?25l"
#define VT_SHOW_CUR  "\033[?25h"
#define VT_SAVE_CUR  "\033[s"
#define VT_REST_CUR  "\033[u"

/* ── cursor move to 1-based row, col ────────────────────────────── */
static inline void tty_goto(int row, int col) {
    char buf[24]; sz p=0;
    char r[8], c[8];
    sz rl=_itoa(r,(i64)row);
    sz cl=_itoa(c,(i64)col);
    buf[p++]='\033'; buf[p++]='[';
    for(sz i=0;i<rl;i++) buf[p++]=r[i];
    buf[p++]=';';
    for(sz i=0;i<cl;i++) buf[p++]=c[i];
    buf[p++]='H';
    os_write(1,buf,p);
}

/* ── write a fixed-width field (pad or truncate with spaces) ────── */
static inline void tty_field(const char *s, int width) {
    int n=(int)_len(s);
    int out = n < width ? n : width;
    os_write(1, s, (sz)out);
    for(int i=out; i<width; i++) os_write(1," ",1);
}

/* ── draw a horizontal rule of char c, length w ─────────────────── */
static inline void tty_hline(char c, int w) {
    char buf[256]; int n = w<255?w:255;
    for(int i=0;i<n;i++) buf[i]=c;
    os_write(1,buf,(sz)n);
}

/* ── read one key (blocks) ──────────────────────────────────────── */
#define KEY_UP    0x100
#define KEY_DOWN  0x101
#define KEY_LEFT  0x102
#define KEY_RIGHT 0x103
#define KEY_F1    0x201
#define KEY_F2    0x202
#define KEY_F3    0x203
#define KEY_F4    0x204
#define KEY_F5    0x205
#define KEY_PGUP  0x301
#define KEY_PGDN  0x302
#define KEY_HOME  0x303
#define KEY_END   0x304
#define KEY_ENTER 0x0d
#define KEY_TAB   0x09
#define KEY_ESC   0x1b
#define KEY_BS    0x7f
#define KEY_QUIT  'q'

static inline int tty_readkey(void) {
    unsigned char c;
    if(os_read(0,&c,1)<=0) return -1;
    if(c!=0x1b) return (int)c;
    /* escape sequence */
    unsigned char seq[8]; int n=0;
    /* non-blocking read of sequence bytes (VMIN=0 trick) */
    struct termios t; os_ioctl(0,TCGETS,&t);
    struct termios nb=t; nb.c_cc[VMIN]=0; nb.c_cc[VTIME]=1;
    os_ioctl(0,TCSETS,&nb);
    while(n<6) {
        if(os_read(0,&seq[n],1)<=0) break;
        n++;
    }
    os_ioctl(0,TCSETS,&t);
    if(n==0) return KEY_ESC;
    if(seq[0]=='[') {
        if(n>=2) {
            if(seq[1]=='A') return KEY_UP;
            if(seq[1]=='B') return KEY_DOWN;
            if(seq[1]=='C') return KEY_RIGHT;
            if(seq[1]=='D') return KEY_LEFT;
            if(seq[1]=='H') return KEY_HOME;
            if(seq[1]=='F') return KEY_END;
        }
        if(n>=3 && seq[2]=='~') {
            if(seq[1]=='5') return KEY_PGUP;
            if(seq[1]=='6') return KEY_PGDN;
        }
        /* F1-F5: \033[11~ .. \033[15~ */
        if(n>=3 && seq[2]=='~') {
            if(seq[1]=='1' && n>=4 && seq[2]=='1') return KEY_F1;
            if(seq[1]=='1' && n>=4 && seq[2]=='2') return KEY_F2;
            if(seq[1]=='1' && n>=4 && seq[2]=='3') return KEY_F3;
        }
        /* \033[11~ = F1, \033[12~ = F2 etc (xterm style) */
        if(n>=3 && seq[1]=='1') {
            if(seq[2]=='1' && n>=4 && seq[3]=='~') return KEY_F1;
            if(seq[2]=='2' && n>=4 && seq[3]=='~') return KEY_F2;
            if(seq[2]=='3' && n>=4 && seq[3]=='~') return KEY_F3;
            if(seq[2]=='4' && n>=4 && seq[3]=='~') return KEY_F4;
            if(seq[2]=='5' && n>=4 && seq[3]=='~') return KEY_F5;
        }
    }
    if(seq[0]=='O') {
        if(seq[1]=='P') return KEY_F1;
        if(seq[1]=='Q') return KEY_F2;
        if(seq[1]=='R') return KEY_F3;
        if(seq[1]=='S') return KEY_F4;
    }
    return KEY_ESC;
}
