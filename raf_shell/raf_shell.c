/* raf_shell.c — RAF Polimata Pipeline Orchestration Shell
 *
 * BBS/Cyberpunk TUI: file browser + module selector + live syslog panel.
 * Inspired by Clipper 5.2 tbrowse, MS-DOS Shell, and 90s BBS aesthetics.
 *
 * Architecture: 8 modules dispatched via fork+execve. Zero malloc.
 * Static buffers. Direct x86_64 Linux syscalls. VT100 terminal UI.
 *
 * CLI usage:
 *   ./raf_shell                    — BBS interactive UI
 *   ./raf_shell help               — show all modules and arguments
 *   ./raf_shell compile <src> <out.apk>
 *   ./raf_shell verbovivo <apk> <engram.svg>
 *   ./raf_shell fiberh  <file>
 *   ./raf_shell recall  <file>
 *   ./raf_shell chain   <src> [out.apk] [engram.svg]
 *   ./raf_shell bench   <src>
 *   ./raf_shell sign    <file>
 *   ./raf_shell logcat
 *
 * Build:
 *   gcc -std=c11 -O2 -Wall -Wextra -fno-builtin \
 *       raf_shell/raf_shell.c -o raf_shell
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | freestanding | no malloc | no libc calls */

#include "sys_host.h"
#include "str_util.h"
#include "tty.h"
#include "logbuf.h"
#include "dirbrowse.h"
#include "sig_audit.h"
#include "pipe_stages.h"

/* ════════════════════════════════════════════════════════════════════
 * LOGO — ASCII art, 6 lines, cyberpunk palette
 * ════════════════════════════════════════════════════════════════════ */
static const char *_LOGO[] = {
    " ██████╗  █████╗ ███████╗    ██████╗ ██╗██████╗ ███████╗",
    " ██╔══██╗██╔══██╗██╔════╝    ██╔══██╗██║██╔══██╗██╔════╝",
    " ██████╔╝███████║█████╗      ██████╔╝██║██████╔╝█████╗  ",
    " ██╔══██╗██╔══██║██╔══╝      ██╔═══╝ ██║██╔═══╝ ██╔══╝  ",
    " ██║  ██║██║  ██║██║         ██║     ██║██║     ███████╗",
    " ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝╚═╝     ╚══════╝",
};
#define LOGO_LINES 6

/* ════════════════════════════════════════════════════════════════════
 * UI STATE
 * ════════════════════════════════════════════════════════════════════ */
#define PANEL_FILES   0
#define PANEL_MODULES 1
#define PANEL_SYSLOG  2

static int _active_panel = PANEL_FILES;
static int _mod_sel      = 0;

/* selected pipeline output paths */
static char _out_apk[512] = "out.apk";
static char _out_svg[512] = "engram.svg";

/* ════════════════════════════════════════════════════════════════════
 * DRAWING PRIMITIVES
 * ════════════════════════════════════════════════════════════════════ */

static inline void _draw_hline_utf8(const char *left, const char *mid,
                                    const char *right, int w) {
    _puts(left);
    for(int i=2;i<w-2;i++) _puts(mid);
    _puts(right);
}

/* Draw the logo panel (rows 1..9) */
static void draw_logo(int cols) {
    /* row 1: top border */
    tty_goto(1,1);
    _puts(VT_BMAGENTA);
    _draw_hline_utf8("╔","═","╗",cols);
    _puts(VT_RESET);

    /* rows 2..7: logo lines */
    for(int i=0;i<LOGO_LINES;i++){
        tty_goto(i+2,1);
        /* alternate cyan / magenta for cyberpunk flicker look */
        _puts(i%2 ? VT_BCYAN : VT_BMAGENTA);
        _puts("║");
        _puts(VT_RESET);

        /* center the logo line */
        int llen=(int)_len(_LOGO[i]);
        int pad=(cols-2-llen)/2; if(pad<0)pad=0;
        _puts(VT_BCYAN VT_BG_BLACK);
        for(int j=0;j<pad;j++) _puts(" ");
        _puts(_LOGO[i]);
        for(int j=pad+llen;j<cols-2;j++) _puts(" ");
        _puts(VT_RESET);

        _puts(i%2 ? VT_BCYAN : VT_BMAGENTA);
        _puts("║");
        _puts(VT_RESET);
    }

    /* row 8: tagline */
    tty_goto(8,1);
    _puts(VT_BMAGENTA "║" VT_RESET);
    {
        static const char *tag = "  // P O L I M A T A   O R C H E S T R A T O R   S H E L L  v1.0 //";
        int tlen=(int)_len(tag);
        int pad=(cols-2-tlen)/2; if(pad<0)pad=0;
        _puts(VT_BYELLOW VT_BG_BLACK VT_BLINK);
        for(int j=0;j<pad;j++) _puts(" ");
        _puts(tag);
        for(int j=pad+tlen;j<cols-2;j++) _puts(" ");
        _puts(VT_RESET);
        _puts(VT_BMAGENTA "║" VT_RESET);
    }

    /* row 9: separator */
    tty_goto(9,1);
    _puts(VT_BMAGENTA);
    _draw_hline_utf8("╠","═","╣",cols);
    _puts(VT_RESET);
}

/* Draw the two-column middle section (rows 10..19):
 *   left half  = file browser
 *   right half = module menu */
static void draw_middle(int cols, int rows) {
    int mid = cols/2;
    int body_rows = rows - 9 - 7; /* rows below logo, above syslog */
    if(body_rows<1) body_rows=1;

    /* ── column header row ── */
    tty_goto(10,1);
    _puts(VT_BMAGENTA "║" VT_RESET);
    int active_files=(_active_panel==PANEL_FILES);
    _puts(active_files ? VT_BWHITE VT_BG_BLUE : VT_CYAN);
    _puts(" [FILES]");
    /* cwd abbreviated */
    static char cwd_short[48];
    sz cwl=_len(_db_cwd);
    if(cwl>36){ _copy(cwd_short,"...",48); sz p=3; _cat(cwd_short,&p,48,_db_cwd+cwl-33); }
    else _copy(cwd_short,_db_cwd,48);
    _puts("  "); _puts(cwd_short);
    /* pad to mid */
    int used=(int)(8+2+(int)_len(cwd_short));
    for(int i=used;i<mid-2;i++) _puts(" ");
    _puts(VT_RESET);
    _puts(VT_BMAGENTA "║" VT_RESET);

    int active_mod=(_active_panel==PANEL_MODULES);
    _puts(active_mod ? VT_BWHITE VT_BG_BLUE : VT_CYAN);
    _puts(" [PIPELINE MODULES]");
    for(int i=19;i<cols-mid-2;i++) _puts(" ");
    _puts(VT_RESET);
    _puts(VT_BMAGENTA "║\n" VT_RESET);

    /* ── file + module rows ── */
    /* scroll: show DB_VISIBLE entries starting at _db_sel - half */
    int db_vis = body_rows;
    int db_start = _db_sel - db_vis/2;
    if(db_start<0) db_start=0;
    if(db_start+db_vis>_db_count && _db_count>db_vis) db_start=_db_count-db_vis;

    for(int r=0;r<db_vis;r++){
        tty_goto(11+r,1);
        _puts(VT_BMAGENTA "║" VT_RESET);

        /* file entry */
        int fi=db_start+r;
        if(fi<_db_count){
            int sel=(fi==_db_sel);
            int is_dir=(_db_ent[fi].type==DT_DIR);
            if(sel && active_files)      _puts(VT_REV VT_BWHITE);
            else if(sel)                 _puts(VT_BYELLOW);
            else if(is_dir)              _puts(VT_BCYAN);
            else                         _puts(VT_WHITE);
            _puts(sel?" ▶ ":" · ");
            if(is_dir) _puts("[");
            tty_field(_db_ent[fi].name, mid-7);
            if(is_dir) _puts("]");
            else       _puts(" ");
        } else {
            for(int i=0;i<mid-2;i++) _puts(" ");
        }
        _puts(VT_RESET);
        _puts(VT_BMAGENTA "║" VT_RESET);

        /* module entry */
        int mi=r;
        if(mi<PIPE_MAX_STAGES){
            /* module selection: track separately */
            (void)0;
            int sel=(mi==_mod_sel);
            if(sel && active_mod)  _puts(VT_REV VT_BWHITE);
            else if(sel)           _puts(VT_BYELLOW);
            else                   _puts(VT_GREEN);
            _puts(sel?" ▶ ":" · ");
            tty_field(_stages[mi].label, cols-mid-6);
            _puts(VT_RESET);
        } else {
            for(int i=0;i<cols-mid-2;i++) _puts(" ");
        }
        _puts(VT_RESET VT_BMAGENTA "║\n" VT_RESET);
    }

    /* ── mid divider ── */
    tty_goto(11+db_vis,1);
    _puts(VT_BMAGENTA);
    _draw_hline_utf8("╠","═","╣",cols);
    _puts(VT_RESET "\n");
}

/* Draw the syslog panel */
static void draw_syslog(int cols, int rows) {
    int log_row = rows - 5;
    int syslog_vis = 4;

    /* syslog header */
    tty_goto(log_row,1);
    int active_log=(_active_panel==PANEL_SYSLOG);
    _puts(VT_BMAGENTA "║" VT_RESET);
    _puts(active_log ? VT_BWHITE VT_BG_BLUE : VT_CYAN);
    _puts(" [SYSLOG]");
    for(int i=9;i<cols-2;i++) _puts(" ");
    _puts(VT_RESET VT_BMAGENTA "║\n" VT_RESET);

    /* syslog lines */
    for(int r=0;r<syslog_vis;r++){
        tty_goto(log_row+1+r,1);
        _puts(VT_BMAGENTA "║" VT_RESET VT_GREEN VT_BG_BLACK " ");
        const char *line=log_line(r,syslog_vis);
        tty_field(line,cols-4);
        _puts(VT_RESET VT_BMAGENTA "║\n" VT_RESET);
    }

    /* bottom border */
    tty_goto(rows-1,1);
    _puts(VT_BMAGENTA);
    _draw_hline_utf8("╠","═","╣",cols);
    _puts(VT_RESET "\n");
}

/* Draw the status bar (last row) */
static void draw_statusbar(int cols, int rows) {
    tty_goto(rows,1);
    _puts(VT_BG_BLUE VT_BWHITE);
    static const char *hint=" TAB=Panel  ↑↓=Nav  ENTER=Run  F1=Help  F2=Select  Q=Quit  F3=Sign  F4=Bench ";
    int hlen=(int)_len(hint);
    _puts(hint);
    for(int i=hlen;i<cols;i++) _puts(" ");
    _puts(VT_RESET);

    /* bottom frame close */
    tty_goto(rows+1<rows?rows:rows,1); /* stays at rows */
}

/* Full screen redraw */
static void draw_all(int cols, int rows) {
    _puts(VT_HIDE_CUR VT_CLS VT_BG_BLACK);
    draw_logo(cols);
    draw_middle(cols, rows);
    draw_syslog(cols, rows);
    draw_statusbar(cols, rows);
    _puts(VT_SHOW_CUR);
}

/* ════════════════════════════════════════════════════════════════════
 * HELP SCREEN (full clear)
 * ════════════════════════════════════════════════════════════════════ */
static void draw_help(void) {
    _puts(VT_CLS VT_BCYAN);
    _puts("╔══════════════════════════════════════════════════════════╗\n");
    _puts("║  RAF POLIMATA ORCHESTRATOR — HELP                       ║\n");
    _puts("╠══════════════════════════════════════════════════════════╣\n");
    _puts("║" VT_RESET VT_BWHITE "  CLI SYNTAX:" VT_RESET VT_BCYAN "                                           ║\n");
    _puts("║" VT_RESET VT_GREEN "  ./raf_shell                   " VT_WHITE "→ BBS interactive UI  " VT_BCYAN "║\n");
    for(int i=0;i<PIPE_MAX_STAGES;i++){
        _puts("║" VT_RESET VT_YELLOW "  ./raf_shell ");
        tty_field(_stages[i].name,10);
        _puts(VT_WHITE " ");
        tty_field(_stages[i].args_desc,30);
        _puts(VT_BCYAN "║\n");
    }
    _puts("╠══════════════════════════════════════════════════════════╣\n");
    _puts("║" VT_RESET VT_BWHITE "  MODULE DESCRIPTIONS:" VT_RESET VT_BCYAN "                                  ║\n");
    for(int i=0;i<PIPE_MAX_STAGES;i++){
        _puts("║" VT_RESET VT_BGREEN "  ");
        tty_field(_stages[i].label,20);
        _puts(VT_WHITE);
        tty_field(_stages[i].desc,35);
        _puts(VT_BCYAN "║\n");
    }
    _puts("╠══════════════════════════════════════════════════════════╣\n");
    _puts("║" VT_RESET VT_BWHITE "  TUI KEYS:" VT_RESET VT_BCYAN "                                             ║\n");
    _puts("║" VT_RESET VT_CYAN "  TAB=Switch panel  ↑↓=Navigate  ENTER=Run module  " VT_BCYAN "   ║\n");
    _puts("║" VT_RESET VT_CYAN "  F1=This help  F2=Select file  F3=Sign  F4=Bench  " VT_BCYAN "   ║\n");
    _puts("║" VT_RESET VT_CYAN "  Q=Quit  ESC=Cancel                               " VT_BCYAN "   ║\n");
    _puts("╚══════════════════════════════════════════════════════════╝\n");
    _puts(VT_RESET "\nPress any key to return...\n");
}

/* ════════════════════════════════════════════════════════════════════
 * INPUT DIALOG — single-line prompt
 * ════════════════════════════════════════════════════════════════════ */
static int input_line(const char *prompt, char *buf, int cap, int rows, int cols) {
    tty_goto(rows-1,1);
    _puts(VT_BG_GREEN VT_BWHITE " ");
    _puts(prompt);
    _puts(": " VT_RESET VT_BG_BLACK VT_BWHITE);
    int n=0; buf[0]=0;
    /* show existing value */
    _puts(buf);
    while(1){
        unsigned char c;
        if(os_read(0,&c,1)<=0) break;
        if(c=='\n'||c=='\r') break;
        if(c==0x1b) { n=0; buf[0]=0; break; }
        if(c==0x7f||c==0x08){
            if(n>0){ n--; buf[n]=0; os_write(1,"\b \b",3); }
            continue;
        }
        if(n+1<cap){ buf[n++]=(char)c; buf[n]=0; os_write(1,&c,1); }
    }
    _puts(VT_RESET);
    (void)cols;
    return n;
}

/* ════════════════════════════════════════════════════════════════════
 * RUN CURRENT SELECTION
 * ════════════════════════════════════════════════════════════════════ */
static void run_selected(int rows, int cols) {
    /* get selected file */
    char src[512]; db_selected_path(src,sizeof src);
    if(!src[0]){ log_push("ERROR: no file selected"); return; }

    /* announce */
    char msg[LOG_COLS]; sz p=0;
    _cat(msg,&p,LOG_COLS,"run: "); _cat(msg,&p,LOG_COLS,_stages[_mod_sel].name);
    _cat(msg,&p,LOG_COLS," src="); _cat(msg,&p,LOG_COLS,_base(src));
    log_push(msg);

    /* restore terminal for subprocess output */
    tty_restore();
    _puts(VT_RESET "\n");
    _puts(VT_BCYAN "── running: " VT_BWHITE);
    _puts(_stages[_mod_sel].label); _puts(VT_RESET "\n");

    int rc = pipe_run(_mod_sel, src, _out_apk, _out_svg);

    char done[LOG_COLS]; sz dp=0;
    _cat(done,&dp,LOG_COLS,"done: rc="); char nb[22]; _itoa(nb,rc); _cat(done,&dp,LOG_COLS,nb);
    log_push(done);

    _puts(VT_BYELLOW "\n── done. Press any key ──\n" VT_RESET);
    { unsigned char c; os_read(0,&c,1); }

    tty_raw();
    draw_all(cols,rows);
    (void)cols;
}

/* ════════════════════════════════════════════════════════════════════
 * TUI EVENT LOOP
 * ════════════════════════════════════════════════════════════════════ */
static void run_tui(void) {
    int rows,cols; tty_size(&rows,&cols);
    log_init();
    log_push("RAF Polimata Shell — ready");
    log_push("TAB=switch panel  ENTER=run  Q=quit");

    /* initial scan of cwd */
    os_getcwd(_db_cwd,sizeof _db_cwd);
    db_scan(_db_cwd);

    tty_raw();
    draw_all(cols,rows);

    int running=1;
    while(running){
        int k=tty_readkey();
        tty_size(&rows,&cols);

        switch(k){
        case 'q': case 'Q':
            running=0; break;

        case KEY_TAB:
            _active_panel=(_active_panel+1)%3;
            draw_all(cols,rows); break;

        case KEY_UP:
            if(_active_panel==PANEL_FILES)        { db_move(-1); draw_all(cols,rows); }
            else if(_active_panel==PANEL_MODULES)  { if(_mod_sel>0) _mod_sel--; draw_all(cols,rows); }
            else if(_active_panel==PANEL_SYSLOG)   { /* scroll syslog up (future) */ }
            break;

        case KEY_DOWN:
            if(_active_panel==PANEL_FILES)        { db_move(1); draw_all(cols,rows); }
            else if(_active_panel==PANEL_MODULES)  { if(_mod_sel<PIPE_MAX_STAGES-1) _mod_sel++; draw_all(cols,rows); }
            break;

        case KEY_PGUP:
            if(_active_panel==PANEL_FILES)  { db_move(-8); draw_all(cols,rows); }
            break;
        case KEY_PGDN:
            if(_active_panel==PANEL_FILES)  { db_move(8); draw_all(cols,rows); }
            break;

        case KEY_ENTER: /* 0x0d = \r */
        case '\n':
            if(_active_panel==PANEL_FILES){
                if(_db_count>0 && _db_ent[_db_sel].type==DT_DIR){
                    db_enter();
                } else {
                    _active_panel=PANEL_MODULES;
                }
                draw_all(cols,rows);
            } else if(_active_panel==PANEL_MODULES){
                run_selected(rows,cols);
            }
            break;

        case KEY_F1:
            tty_restore();
            draw_help();
            { unsigned char c; os_read(0,&c,1); }
            tty_raw();
            draw_all(cols,rows);
            break;

        case KEY_F2:
            /* prompt for output APK name */
            input_line("Output APK", _out_apk, sizeof _out_apk, rows, cols);
            if(!_out_apk[0]) _copy(_out_apk,"out.apk",sizeof _out_apk);
            draw_all(cols,rows);
            break;

        case KEY_F3:
            /* quick sign selected file */
            {
                char src[512]; db_selected_path(src,sizeof src);
                if(src[0]){
                    i64 rc=sig_write(src,"manual-sign",0);
                    char msg[LOG_COLS]; sz p=0;
                    _cat(msg,&p,LOG_COLS,"sign: "); _cat(msg,&p,LOG_COLS,_base(src));
                    char nb[22]; _itoa(nb,rc); _cat(msg,&p,LOG_COLS," rc="); _cat(msg,&p,LOG_COLS,nb);
                    log_push(msg);
                }
                draw_all(cols,rows);
            }
            break;

        case KEY_F4:
            /* quick bench */
            _mod_sel=5;
            run_selected(rows,cols);
            break;

        case KEY_ESC:
            /* cancel / go back to files */
            _active_panel=PANEL_FILES;
            draw_all(cols,rows);
            break;

        default:
            break;
        }
    }

    tty_restore();
    _puts(VT_RESET VT_CLS);
    _puts(VT_BCYAN "RAF Polimata Shell — session ended.\n" VT_RESET);
    log_push("session end");
    log_flush_file();
}

/* ════════════════════════════════════════════════════════════════════
 * CLI DISPATCH (non-interactive)
 * ════════════════════════════════════════════════════════════════════ */
static void cli_help(void) {
    _puts(VT_BCYAN
"┌─────────────────────────────────────────────────────────────┐\n"
"│  RAF POLIMATA ORCHESTRATOR — CLI REFERENCE                  │\n"
"├─────────────────────────────────────────────────────────────┤\n"
VT_RESET VT_BWHITE
"  USAGE: ./raf_shell [module] [args...]\n\n"
VT_BYELLOW
"  (no args)       " VT_WHITE "→ launch BBS interactive TUI\n"
VT_BYELLOW
"  help            " VT_WHITE "→ this message\n\n"
VT_RESET VT_BCYAN
"  MODULES:\n" VT_RESET);
    for(int i=0;i<PIPE_MAX_STAGES;i++){
        _puts(VT_BYELLOW "  ");
        tty_field(_stages[i].name,12);
        _puts(VT_WHITE);
        tty_field(_stages[i].args_desc,28);
        _puts(VT_GREEN "  ");
        _puts(_stages[i].desc);
        _nl();
    }
    _puts(VT_BCYAN
"\n  OUTPUTS:\n" VT_RESET
"  out.apk         compiled APK (apkc)\n"
"  engram.svg      convergence engram (verbovivo)\n"
"  <file>.sig      chain-of-custody signature\n"
"  raf_bench.log   benchmark and process log\n"
VT_BCYAN
"└─────────────────────────────────────────────────────────────┘\n"
VT_RESET);
}

static int find_stage(const char *name) {
    for(int i=0;i<PIPE_MAX_STAGES;i++)
        if(_eq(_stages[i].name,name)) return i;
    return -1;
}

static int run_cli(int argc, char **argv) {
    log_init();
    int stage=find_stage(argv[1]);
    if(stage<0){
        _err("raf_shell: unknown module '"); _err(argv[1]); _err("'\n");
        _err("Run './raf_shell help' for usage.\n");
        return 1;
    }
    const char *src     = argc>2 ? argv[2] : NULL;
    const char *out_apk = argc>3 ? argv[3] : "out.apk";
    const char *out_svg = argc>4 ? argv[4] : "engram.svg";

    if(!src && _stages[stage].arg_src_pos>=0){
        _err("raf_shell: "); _err(_stages[stage].name);
        _err(" requires a source file argument\n"); return 1;
    }
    int rc = pipe_run(stage, src, out_apk, out_svg);
    log_flush_file();
    return rc;
}

/* ════════════════════════════════════════════════════════════════════
 * ENTRY POINT
 * ════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv) {
    if(argc<2){
        run_tui();
        return 0;
    }
    if(_eq(argv[1],"help")||_eq(argv[1],"--help")||_eq(argv[1],"-h")){
        cli_help();
        return 0;
    }
    return run_cli(argc,argv);
}
