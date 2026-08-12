#!/usr/bin/env python3
from __future__ import annotations
import argparse
from pathlib import Path

OLD = '''    /* read source */\n    sz src_len = 0;\n    i32 fd = os_open(inpath, 0, 0);\n    if (fd<0) { pr_err("cannot open: "); pr_err(inpath); pr_err("\\n"); return 1; }\n    while (src_len < sizeof(_src_local)-1) {\n        i32 n = os_read(fd, _src_local+src_len, sizeof(_src_local)-src_len-1);\n        if (n<=0) break;\n        src_len += (sz)n;\n    }\n    os_close(fd);\n    _src_local[src_len] = 0;\n'''

NEW = '''    /* read source: fail closed on read error or capacity overflow.\n     * Reserving one byte keeps the legacy NUL terminator contract while the\n     * one-byte probe distinguishes exact-fit EOF from silent truncation. */\n    sz src_len = 0;\n    int src_eof = 0;\n    i32 fd = os_open(inpath, 0, 0);\n    if (fd<0) { pr_err("cannot open: "); pr_err(inpath); pr_err("\\n"); return 1; }\n    while (src_len < sizeof(_src_local)-1) {\n        i32 n = os_read(fd, _src_local+src_len, sizeof(_src_local)-src_len-1);\n        if (n<0) { pr_err("source read failed\\n"); os_close(fd); return 1; }\n        if (n==0) { src_eof=1; break; }\n        src_len += (sz)n;\n    }\n    if (!src_eof && src_len == sizeof(_src_local)-1) {\n        u8 extra = 0;\n        i32 n = os_read(fd, &extra, 1u);\n        if (n<0) { pr_err("source overflow probe failed\\n"); os_close(fd); return 1; }\n        if (n>0) { pr_err("source exceeds SRC_CAP\\n"); os_close(fd); return 1; }\n    }\n    os_close(fd);\n    _src_local[src_len] = 0;\n'''

def transform(text: str) -> str:
    count = text.count(OLD)
    if count != 1:
        raise ValueError(f"source-read anchor count must be 1, got {count}")
    out = text.replace(OLD, NEW, 1)
    if 'source exceeds SRC_CAP' not in out:
        raise AssertionError('overflow guard missing after transform')
    return out

def main() -> int:
    p=argparse.ArgumentParser()
    p.add_argument('src')
    p.add_argument('dst')
    a=p.parse_args()
    src=Path(a.src).read_text(encoding='utf-8')
    Path(a.dst).write_text(transform(src), encoding='utf-8')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
