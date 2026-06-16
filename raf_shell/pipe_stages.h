/* pipe_stages.h — pipeline stage registry. 8 modules, fork+execve dispatch.
 * Each stage declares its CLI binary, argument template, and description.
 * No libc. No malloc. Chain-of-custody via sig_audit.h. */
#pragma once
#include "sys_host.h"
#include "str_util.h"
#include "logbuf.h"
#include "sig_audit.h"

#define PIPE_MAX_STAGES  8
#define PIPE_MAX_ARGS   16

typedef struct {
    const char *name;        /* short key: "compile", "verbovivo", ... */
    const char *label;       /* UI display: "[1] APKc Compiler" */
    const char *desc;        /* one-line description */
    const char *binary;      /* executable name (searched via PATH) */
    const char *args_desc;   /* argument hint for help */
    /* fixed argv slots; NULL = fill src/out at runtime */
    const char *argv[PIPE_MAX_ARGS];
    int         arg_src_pos; /* position in argv to insert source file */
    int         arg_out_pos; /* position in argv to insert output file (-1=none) */
} PipeStage;

/* ── 8 pipeline stages ───────────────────────────────────────────── */
static const PipeStage _stages[PIPE_MAX_STAGES] = {
    /* 0 */ {
        "compile", "[1] APKc Compiler",
        "Compile source → ARM64 APK via language dispatch table",
        "./apkc",
        "<src> <out.apk>",
        {"./apkc", NULL, NULL, NULL},
        1, 2
    },
    /* 1 */ {
        "verbovivo", "[2] VerbVivo Engine",
        "T^7 toroid + Fiber-H → SVG engram (convergence analysis)",
        "./verbovivo",
        "<apk> <engram.svg>",
        {"./verbovivo", NULL, NULL, NULL},
        1, 2
    },
    /* 2 */ {
        "fiberh", "[3] Fiber-H Scan",
        "256-bit Hamming hash + Trinity stream audit to stdout",
        "./verbovivo",
        "< <file>  (stdin mode)",
        {"./verbovivo", "-s", NULL, NULL},
        -1, -1  /* src piped via stdin — handled separately */
    },
    /* 3 */ {
        "recall", "[4] T^7 Recall",
        "Recall top-3 engrams most resonant with current context",
        "./verbovivo",
        "-r 3 < <file>",
        {"./verbovivo", "-r", "3", NULL, NULL},
        -1, -1
    },
    /* 4 */ {
        "chain", "[5] Full Chain",
        "compile → verbovivo in sequence (full proof chain)",
        NULL,   /* multi-step: handled in pipe_run */
        "<src>  (auto output names)",
        {NULL},
        1, -1
    },
    /* 5 */ {
        "bench", "[6] Benchmark",
        "Timed compile + verbovivo + log delta to raf_bench.log",
        NULL,
        "<src>",
        {NULL},
        1, -1
    },
    /* 6 */ {
        "sign", "[7] Sign + Audit",
        "CRC32 sign output file → <file>.sig chain-of-custody",
        NULL,
        "<file>",
        {NULL},
        1, -1
    },
    /* 7 */ {
        "logcat", "[8] Logcat Export",
        "Flush syslog ring buffer → raf_bench.log (append)",
        NULL,
        "(no args)",
        {NULL},
        -1, -1
    },
};

/* ── environment for child processes ─────────────────────────────── */
static char *_pipe_envp[] = { NULL };

/* ── spawn binary with src and out inserted at declared positions ── */
static inline int _pipe_exec_stage(const PipeStage *st,
                                   const char *src, const char *out)
{
    /* build argv (static, avoids heap) */
    static const char *av[PIPE_MAX_ARGS+1];
    int n=0;
    for(; n<PIPE_MAX_ARGS && st->argv[n]; n++) av[n]=st->argv[n];
    /* insert src/out if positions are within range */
    /* rebuild from scratch to place src/out correctly */
    n=0;
    for(int i=0; i<PIPE_MAX_ARGS; i++){
        if(i==st->arg_src_pos && src) { av[n++]=src; continue; }
        if(i==st->arg_out_pos && out) { av[n++]=out; continue; }
        if(st->argv[i]==NULL) break;
        av[n++]=st->argv[i];
    }
    av[n]=NULL;

    i32 pid=os_fork();
    if(pid<0) return -1;
    if(pid==0) {
        os_execve(st->binary,(char*const*)av,(char*const*)_pipe_envp);
        os_exit(127);
    }
    i32 st_val=0;
    os_wait4(pid,&st_val,0);
    return (st_val>>8)&0xff;
}

/* ── main dispatch ───────────────────────────────────────────────── */
static inline int pipe_run(int idx, const char *src, const char *out_apk, const char *out_svg)
{
    if(idx<0||idx>=PIPE_MAX_STAGES) return -1;
    const PipeStage *st=&_stages[idx];
    i64 t0=log_elapsed(); int rc=0;

    if(idx==0) { /* compile */
        log_push("apkc: dispatching compile stage");
        rc=_pipe_exec_stage(st,src,out_apk);
        char msg[LOG_COLS]; sz p=0;
        _cat(msg,&p,LOG_COLS,"apkc: exit="); char nb[22]; _itoa(nb,rc); _cat(msg,&p,LOG_COLS,nb);
        log_push(msg);
        if(rc==0 && out_apk) sig_write(out_apk,"apkc",log_elapsed()-t0);
    }
    else if(idx==1) { /* verbovivo */
        log_push("verbovivo: T^7 toroid analysis");
        rc=_pipe_exec_stage(st,src,out_svg?out_svg:out_apk);
        char msg[LOG_COLS]; sz p=0;
        _cat(msg,&p,LOG_COLS,"verbovivo: exit="); char nb[22]; _itoa(nb,rc); _cat(msg,&p,LOG_COLS,nb);
        log_push(msg);
        if(rc==0 && out_svg) sig_write(out_svg,"verbovivo",log_elapsed()-t0);
    }
    else if(idx==2) { /* fiberh — stdin mode */
        log_push("fiberh: fiber-H scan (stdin)");
        /* reopen src as stdin for child */
        i32 pid=os_fork();
        if(pid==0){
            i32 fd=os_open(src,O_RDONLY,0);
            if(fd>=0){ /* dup2 via close+open trick not available; use redirect */
                /* crude: just pass as arg instead */
                os_close(fd);
            }
            const char *av[]={"./verbovivo","-s",src,NULL};
            os_execve("./verbovivo",(char*const*)av,(char*const*)_pipe_envp);
            os_exit(127);
        }
        i32 sv=0; os_wait4(pid,&sv,0); rc=(sv>>8)&0xff;
        char msg[LOG_COLS]; sz p=0;
        _cat(msg,&p,LOG_COLS,"fiberh: exit="); char nb[22]; _itoa(nb,rc); _cat(msg,&p,LOG_COLS,nb);
        log_push(msg);
    }
    else if(idx==3) { /* recall */
        log_push("recall: top-3 engram resonance");
        i32 pid=os_fork();
        if(pid==0){
            const char *av[]={"./verbovivo","-r","3",src,NULL};
            os_execve("./verbovivo",(char*const*)av,(char*const*)_pipe_envp);
            os_exit(127);
        }
        i32 sv=0; os_wait4(pid,&sv,0); rc=(sv>>8)&0xff;
        char msg[LOG_COLS]; sz p=0;
        _cat(msg,&p,LOG_COLS,"recall: exit="); char nb[22]; _itoa(nb,rc); _cat(msg,&p,LOG_COLS,nb);
        log_push(msg);
    }
    else if(idx==4) { /* chain: compile then verbovivo */
        log_push("chain: stage 1/2 — compile");
        rc=pipe_run(0,src,out_apk,NULL);
        if(rc==0){
            log_push("chain: stage 2/2 — verbovivo");
            rc=pipe_run(1,out_apk,NULL,out_svg);
        }
        char msg[LOG_COLS]; sz p=0; char nb[22];
        _cat(msg,&p,LOG_COLS,"chain: done rc="); _itoa(nb,rc); _cat(msg,&p,LOG_COLS,nb);
        i64 dt=log_elapsed()-t0; _cat(msg,&p,LOG_COLS," dt="); _itoa(nb,dt); _cat(msg,&p,LOG_COLS,"s");
        log_push(msg);
    }
    else if(idx==5) { /* bench */
        log_push("bench: timed full chain");
        rc=pipe_run(4,src,out_apk,out_svg);
        char msg[LOG_COLS]; sz p=0; char nb[22];
        i64 dt=log_elapsed()-t0;
        _cat(msg,&p,LOG_COLS,"bench: total="); _itoa(nb,dt); _cat(msg,&p,LOG_COLS,nb); _cat(msg,&p,LOG_COLS,"s");
        log_push(msg);
        log_flush_file();
    }
    else if(idx==6) { /* sign */
        log_push("sign: computing CRC32");
        int sret=sig_write(src,"sign",log_elapsed()-t0);
        char msg[LOG_COLS]; sz p=0; char nb[22];
        _cat(msg,&p,LOG_COLS,"sign: "); _cat(msg,&p,LOG_COLS,src);
        _cat(msg,&p,LOG_COLS," rc="); _itoa(nb,sret); _cat(msg,&p,LOG_COLS,nb);
        log_push(msg);
    }
    else if(idx==7) { /* logcat */
        log_push("logcat: flushing to raf_bench.log");
        log_flush_file();
    }

    i64 dt=log_elapsed()-t0;
    char tbuf[64]; sz tp=0;
    _cat(tbuf,&tp,64,"elapsed: "); char nb[22]; _itoa(nb,dt); _cat(tbuf,&tp,64,nb); _cat(tbuf,&tp,64,"s");
    log_push(tbuf);
    return rc;
}
