#!/usr/bin/env python3
"""Generate the canonical runtime-hardened ApkC translation unit.

The upstream-like source remains readable and reviewable. This transformer applies a
small, fail-closed set of exact substitutions and refuses to generate output when an
anchor changes or appears more than once. The generated file is the source compiled by
the runtime-hardening proof; it is never edited by hand.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from dataclasses import dataclass, asdict
from pathlib import Path

SCHEMA = "raf.apkc.runtime-source-hardening.v1"


@dataclass(frozen=True)
class Change:
    change_id: str
    description: str
    occurrences: int = 1


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def replace_once(text: str, old: str, new: str, change: Change) -> str:
    count = text.count(old)
    if count != change.occurrences:
        raise ValueError(
            f"{change.change_id}: expected {change.occurrences} occurrence(s), found {count}"
        )
    return text.replace(old, new, change.occurrences)


HELPERS_ANCHOR = """static u8 _fork_out[0x100000]; /* fork+exec output buffer */
"""

HELPERS_REPLACEMENT = r"""static u8 _fork_out[0x100000]; /* fork+exec output buffer */

/* Runtime hardening helpers. They are generated into this translation unit so the
 * canonical source stays small while both ARM ABIs receive the same fail-closed
 * subprocess contract. No shell, no PATH interpolation and no heap are involved. */
#if defined(__arm__)
static __attribute__((always_inline)) i32
_apkc_sc1_32(i32 n, i32 a) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7) : "memory","cc");
    return r0;
}
static __attribute__((always_inline)) i32
_apkc_sc3_32(i32 n, i32 a, i32 b, i32 c) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    register i32 r1 __asm__("r1") = b;
    register i32 r2 __asm__("r2") = c;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7),"r"(r1),"r"(r2) : "memory","cc");
    return r0;
}
static __attribute__((always_inline)) i32
_apkc_sc4_32(i32 n, i32 a, i32 b, i32 c, i32 d) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    register i32 r1 __asm__("r1") = b;
    register i32 r2 __asm__("r2") = c;
    register i32 r3 __asm__("r3") = d;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7),"r"(r1),"r"(r2),"r"(r3) : "memory","cc");
    return r0;
}
static __attribute__((always_inline)) i32
_apkc_sc5_32(i32 n, i32 a, i32 b, i32 c, i32 d, i32 e) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    register i32 r1 __asm__("r1") = b;
    register i32 r2 __asm__("r2") = c;
    register i32 r3 __asm__("r3") = d;
    register i32 r4 __asm__("r4") = e;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7),"r"(r1),"r"(r2),"r"(r3),"r"(r4) : "memory","cc");
    return r0;
}
#endif

static int _apkc_has_slash(const char *p) {
    if (!p) return 0;
    for (sz i=0; p[i]; i++) if (p[i]=='/') return 1;
    return 0;
}

static const char *_apkc_basename(const char *p) {
    const char *name=p;
    if (!p) return p;
    for (sz i=0; p[i]; i++) if (p[i]=='/' && p[i+1]) name=p+i+1;
    return name;
}

static int _apkc_join_exec(char out[256], const char *prefix, const char *name) {
    sz p=0;
    if (!out || !prefix || !name || !name[0]) return 0;
    while (prefix[p]) { if (p>=255u) return 0; out[p]=prefix[p]; p++; }
    for (sz i=0; name[i]; i++) { if (p>=255u) return 0; out[p++]=name[i]; }
    out[p]=0;
    return 1;
}

static i32 _apkc_unlink(const char *path) {
#if defined(__aarch64__)
    /* unlinkat(AT_FDCWD, path, 0) */
    return (i32)_sc3(35LL, -100LL, (i64)(uptr)path, 0LL);
#else
    return _apkc_sc1_32(10, (i32)(uptr)path);
#endif
}

static i32 _apkc_fork(void) {
#if defined(__aarch64__)
    return os_fork();
#else
    /* clone(SIGCHLD, NULL, NULL, NULL, NULL) */
    return _apkc_sc5_32(120, 17, 0, 0, 0, 0);
#endif
}

static i32 _apkc_waitpid(i32 pid, i32 *status) {
#if defined(__aarch64__)
    return os_waitpid(pid, status, 0);
#else
    return _apkc_sc4_32(114, pid, (i32)(uptr)status, 0, 0);
#endif
}

static i32 _apkc_execve_raw(const char *path, char *const argv[], char *const envp[]) {
#if defined(__aarch64__)
    return _os_execve_raw(path, argv, envp);
#else
    return _apkc_sc3_32(11, (i32)(uptr)path, (i32)(uptr)argv, (i32)(uptr)envp);
#endif
}

static i32 _apkc_execve(const char *path, char *const argv[]) {
    static char env_path[]="PATH=/data/data/com.termux/files/usr/bin:/system/bin:/usr/bin:/bin";
    static char env_home[]="HOME=/data/data/com.termux/files/home";
    static char env_tmp[]="TMPDIR=/data/data/com.termux/files/usr/tmp";
    static char env_lang[]="LANG=C";
    static char *envp[]={env_path,env_home,env_tmp,env_lang,NULL};
    static const char *prefixes[]={
        "/data/data/com.termux/files/usr/bin/", "/system/bin/", "/usr/bin/", "/bin/"
    };
    if (!path || !path[0]) return -2;
    i32 last=-2;
    const char *name=path;
    if (_apkc_has_slash(path)) {
        last=_apkc_execve_raw(path,argv,envp);
        name=_apkc_basename(path);
    }
    char candidate[256];
    for (sz i=0; i<sizeof(prefixes)/sizeof(prefixes[0]); i++) {
        if (!_apkc_join_exec(candidate,prefixes[i],name)) return -36;
        last=_apkc_execve_raw(candidate,argv,envp);
    }
    return last;
}

static int _apkc_is_dex(const u8 *p, sz n) {
    return p && n>=0x70u && p[0]=='d' && p[1]=='e' && p[2]=='x' && p[3]=='\n' &&
           p[4]>='0' && p[4]<='9' && p[5]>='0' && p[5]<='9' &&
           p[6]>='0' && p[6]<='9' && p[7]==0;
}

static int _apkc_is_elf64_aarch64(const u8 *p, sz n) {
    return p && n>=64u && p[0]==0x7fu && p[1]=='E' && p[2]=='L' && p[3]=='F' &&
           p[4]==2u && p[5]==1u && r16(p+16)==3u && r16(p+18)==183u;
}
"""

OLD_FORK = r"""#ifdef __aarch64__
static sz fork_exec_wait(const char *compiler, char *const args[],
                         const char *outfile, u8 *outbuf, sz outbuf_cap)
{
    i32 pid = os_fork();
    if (pid < 0) { pr_err("fork failed\n"); return 0; }
    if (pid == 0) {
        /* child: exec compiler */
        char *const envp[] = { NULL };
        os_execve(compiler, args, envp);
        os_exit(127); /* exec failed */
    }
    /* parent: wait for child */
    i32 status = 0;
    os_waitpid(pid, &status, 0);
    if (((status >> 8) & 0xFF) != 0) {
        pr_err("compiler exited with error\n"); return 0;
    }
    /* read output file */
    i32 fd = os_open(outfile, 0, 0);
    if (fd < 0) { pr_err("cannot open compiler output\n"); return 0; }
    sz total = 0;
    while (total < outbuf_cap) {
        i32 n = os_read(fd, outbuf + total, outbuf_cap - total);
        if (n <= 0) break;
        total += (sz)n;
    }
    os_close(fd);
    return total;
}
#else
static sz fork_exec_wait(const char *compiler, char *const args[],
                         const char *outfile, u8 *outbuf, sz outbuf_cap)
{
    (void)compiler; (void)args; (void)outfile; (void)outbuf; (void)outbuf_cap;
    pr_err("fork_exec_wait: not supported on ARM32\n");
    return 0;
}
#endif
"""

NEW_FORK = r"""static sz fork_exec_wait(const char *compiler, char *const args[],
                         const char *outfile, u8 *outbuf, sz outbuf_cap)
{
    if (!compiler || !args || !outfile || !outbuf || !outbuf_cap) {
        pr_err("fork_exec_wait: invalid contract\n"); return 0;
    }

    /* A stale artifact must never satisfy a failed compiler invocation. ENOENT
     * (-2) is expected; every other unlink error is fail-closed. */
    i32 urc=_apkc_unlink(outfile);
    if (urc<0 && urc!=-2) { pr_err("cannot clear compiler output\n"); return 0; }

    i32 pid=_apkc_fork();
    if (pid<0) { pr_err("fork failed\n"); return 0; }
    if (pid==0) {
        _apkc_execve(compiler,args);
        os_exit(127);
    }

    i32 status=0;
    i32 waited=_apkc_waitpid(pid,&status);
    if (waited!=pid) { pr_err("waitpid failed\n"); return 0; }
    if ((status & 0x7f) != 0 || ((status >> 8) & 0xff) != 0) {
        pr_err("compiler exited with error or signal\n"); return 0;
    }

    i32 fd=os_open(outfile,O_RDONLY,0);
    if (fd<0) { pr_err("cannot open compiler output\n"); return 0; }
    sz total=0;
    int eof=0;
    while (total<outbuf_cap) {
        i32 n=os_read(fd,outbuf+total,outbuf_cap-total);
        if (n<0) { pr_err("compiler output read failed\n"); os_close(fd); return 0; }
        if (n==0) { eof=1; break; }
        total+=(sz)n;
    }
    if (!eof && total==outbuf_cap) {
        u8 extra=0;
        i32 n=os_read(fd,&extra,1u);
        if (n!=0) {
            pr_err("compiler output exceeds bounded buffer\n");
            os_close(fd); return 0;
        }
    }
    if (os_close(fd)<0) { pr_err("compiler output close failed\n"); return 0; }
    if (!total) { pr_err("compiler produced empty output\n"); return 0; }
    return total;
}
"""


TRANSFORMS: list[tuple[Change, str, str]] = [
    (
        Change("APKC-RH-001", "add cross-ABI process, unlink and artifact helpers"),
        HELPERS_ANCHOR,
        HELPERS_REPLACEMENT,
    ),
    (
        Change("APKC-RH-002", "replace subprocess runner with stale-output and truncation gates"),
        OLD_FORK,
        NEW_FORK,
    ),
    (
        Change("APKC-RH-003", "keep an explicit pointer for external arm64 ELF bytes"),
        "    const u8 *dex_buf_ptr = _dex_buf;\n",
        "    const u8 *dex_buf_ptr = _dex_buf;\n    const u8 *so64_buf_ptr = _so64_buf;\n",
    ),
    (
        Change("APKC-RH-004", "remove global /tmp dependency from compiler outputs"),
        """        /* JSX babel writes to /tmp/jsx_out.js; all others write to /tmp/apkc_out.so */
        static const char _tmpout_so[]  = "/tmp/apkc_out.so";
        static const char _tmpout_jsx[] = "/tmp/jsx_out.js";
""",
        """        /* Bounded single-process scratch files in the current private build directory. */
        static const char _tmpout_so[]  = ".apkc-out.so";
        static const char _tmpout_jsx[] = ".apkc-jsx-out.js";
""",
    ),
    (
        Change("APKC-RH-005", "use local D8 directory and clear classes.dex through runner"),
        """            static const char _d8_out[] = "/tmp/classes.dex";
""",
        """            static const char _d8_out[] = "classes.dex";
""",
    ),
    (
        Change("APKC-RH-006", "remove hard-coded D8 /tmp output directory"),
        """            d8args[nd++] = "/tmp/";
""",
        """            d8args[nd++] = ".";
""",
    ),
    (
        Change("APKC-RH-007", "validate DEX magic before packaging"),
        """            if (dexout) {
                /* d8 succeeded: use _fork_out directly (avoids 200B _dex_buf limit) */
""",
        """            if (dexout && _apkc_is_dex(_fork_out,dexout)) {
                /* d8 succeeded and emitted a structurally recognizable DEX. */
""",
    ),
    (
        Change("APKC-RH-008", "reject direct dex_output profiles without D8"),
        """        } else if (prof->dex_output) {
            /* dex_output but no d8 step: store JAR/DEX directly */
            m_cpy(_dex_buf, _fork_out, outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf));
            dexsz = outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf);

""",
        """        } else if (prof->dex_output) {
            pr_err("dex_output profile requires D8 and validated DEX output\n");
            return -1;

""",
    ),
    (
        Change("APKC-RH-009", "validate external native ELF and avoid 32KiB truncation"),
        """        } else {
            /* C/C++/Rust: output is a native .so */
            m_cpy(_so64_buf, _fork_out, outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf));
            so64sz = outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf);
        }
""",
        """        } else {
            /* C/C++/Rust: retain the bounded 1MiB result without a second truncating copy. */
            if (!_apkc_is_elf64_aarch64(_fork_out,outsz)) {
                pr_err("external compiler produced invalid AArch64 ET_DYN ELF\n");
                return -1;
            }
            so64_buf_ptr = _fork_out;
            so64sz = outsz;
        }
""",
    ),
    (
        Change("APKC-RH-010", "remove global /tmp dependency from SPIR-V output"),
        '        static const char _tmpout_spv[] = "/tmp/apkc_compute.spv";\n',
        '        static const char _tmpout_spv[] = ".apkc-compute.spv";\n',
    ),
    (
        Change("APKC-RH-011", "remove global /tmp dependency from DSP output"),
        '        static const char _tmpout_dsp[] = "/tmp/apkc_dsp.so";\n',
        '        static const char _tmpout_dsp[] = ".apkc-dsp.so";\n',
    ),
    (
        Change("APKC-RH-012", "package external ELF from its non-truncated buffer"),
        '            if (zip_add(&zw, (const char*)p64, _so64_buf, (u32)so64sz)<0)',
        '            if (zip_add(&zw, (const char*)p64, so64_buf_ptr, (u32)so64sz)<0)',
    ),
    (
        Change("APKC-RH-013", "detect source files that exceed the bounded source buffer"),
        """    while (src_len < sizeof(_src_local)-1) {
        i32 n = os_read(fd, _src_local+src_len, sizeof(_src_local)-src_len-1);
        if (n<=0) break;
        src_len += (sz)n;
    }
    os_close(fd);
    _src_local[src_len] = 0;
""",
        """    int src_eof=0;
    while (src_len < sizeof(_src_local)-1) {
        i32 n = os_read(fd, _src_local+src_len, sizeof(_src_local)-src_len-1);
        if (n<0) { pr_err("source read failed\n"); os_close(fd); return 1; }
        if (n==0) { src_eof=1; break; }
        src_len += (sz)n;
    }
    if (!src_eof && src_len==sizeof(_src_local)-1) {
        u8 extra=0;
        if (os_read(fd,&extra,1u)!=0) {
            pr_err("source exceeds 1MiB bounded input\n"); os_close(fd); return 1;
        }
    }
    if (os_close(fd)<0) { pr_err("source close failed\n"); return 1; }
    _src_local[src_len] = 0;
""",
    ),
]


def transform(source: str) -> tuple[str, list[Change]]:
    out = source
    applied: list[Change] = []
    for change, old, new in TRANSFORMS:
        out = replace_once(out, old, new, change)
        applied.append(change)
    banner = (
        "/* GENERATED by scripts/patch_apkc_runtime_source.py; do not edit.\n"
        " * Runtime hardening is fail-closed and anchor-verified. */\n"
    )
    return banner + out, applied


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, default=Path("Apkc/apkc.c"))
    parser.add_argument(
        "--output", type=Path, default=Path("build/generated/Apkc/apkc.runtime-hardened.c")
    )
    parser.add_argument(
        "--write-manifest",
        type=Path,
        default=Path("results/apkc-runtime-source-hardening.json"),
    )
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)

    raw = args.input.read_bytes()
    try:
        generated, applied = transform(raw.decode("utf-8"))
    except (UnicodeDecodeError, ValueError) as exc:
        print(f"apkc runtime source hardening: FAIL: {exc}", file=sys.stderr)
        return 1

    payload = generated.encode("utf-8")
    report = {
        "schema": SCHEMA,
        "state": "PASS",
        "claim_allowed": False,
        "input": str(args.input),
        "input_sha256": sha256_bytes(raw),
        "output": str(args.output),
        "output_sha256": sha256_bytes(payload),
        "changes": [asdict(item) for item in applied],
        "runtime_evidence": "TOKEN_VAZIO",
    }

    if args.check:
        if not args.output.is_file() or args.output.read_bytes() != payload:
            print("apkc runtime source hardening: generated output is stale", file=sys.stderr)
            return 1
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(payload)
        args.write_manifest.parent.mkdir(parents=True, exist_ok=True)
        args.write_manifest.write_text(
            json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )

    print(
        f"apkc runtime source hardening: PASS changes={len(applied)} "
        f"sha256={report['output_sha256']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
