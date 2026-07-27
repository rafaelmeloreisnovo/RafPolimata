#!/usr/bin/env python3
"""Rewrite a bounded C/C++ subset onto RAFAELIA's freestanding layer.

A header is removed only when its required surface is provided by
``Apkc/raf_libc_emu.h``. Unknown includes and unsafe hosted operations fail
closed instead of being silently discarded.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path

SCHEMA = "rafaelia.c.rewrite.v2"
EMULATED_HEADERS = {
    "stddef.h",
    "stdint.h",
    "stdbool.h",
    "stdio.h",
    "stdlib.h",
    "string.h",
}
UNSUPPORTED_HOSTED_HEADERS = {
    "assert.h", "ctype.h", "errno.h", "inttypes.h", "limits.h",
    "setjmp.h", "signal.h", "strings.h", "time.h", "unistd.h",
}
EMULATED_CALLS = {
    "memcpy", "memmove", "memset", "memcmp", "memchr",
    "strlen", "strnlen", "strcmp", "strncmp", "strncpy",
    "strchr", "strrchr", "atoi", "strtoul", "putchar", "puts",
}
FORBIDDEN_CALLS = {
    "malloc", "calloc", "realloc", "free", "aligned_alloc",
    "strcpy", "strcat", "gets",
    "printf", "fprintf", "sprintf", "snprintf", "vprintf", "vfprintf",
    "fopen", "fdopen", "fread", "fwrite", "fseek", "ftell", "fclose",
    "system", "popen", "dlopen", "dlsym", "pthread_create", "fork",
    "execve", "setjmp", "longjmp", "atexit", "exit", "abort",
}
FORBIDDEN_TOKENS = {"FILE", "jmp_buf", "pthread_t", "va_list"}
INJECT = '#include "raf_libc_emu.h"\n'


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=str(path.parent))
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(tmp_name, path)
    except BaseException:
        try:
            os.unlink(tmp_name)
        except FileNotFoundError:
            pass
        raise


def mask_comments_and_literals(src: str) -> str:
    out = list(src)
    i = 0
    state = "code"
    while i < len(src):
        c = src[i]
        n = src[i + 1] if i + 1 < len(src) else ""
        if state == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
                continue
            if c == '"':
                out[i] = " "
                i += 1
                state = "string"
                continue
            if c == "'":
                out[i] = " "
                i += 1
                state = "char"
                continue
        elif state == "line":
            if c == "\n":
                state = "code"
            else:
                out[i] = " "
        elif state == "block":
            out[i] = " "
            if c == "*" and n == "/":
                out[i + 1] = " "
                i += 2
                state = "code"
                continue
        elif state in {"string", "char"}:
            out[i] = " "
            if c == "\\" and n:
                out[i + 1] = " "
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    return "".join(out)


def rewrite(src: str) -> tuple[str, dict[str, object]]:
    stripped_headers: list[str] = []
    unresolved_headers: list[str] = []
    lines: list[str] = []
    include_re = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]\s*(?://.*)?$')
    already_injected = False

    for line in src.splitlines(keepends=True):
        match = include_re.match(line.rstrip("\r\n"))
        if not match:
            lines.append(line)
            continue
        delimiter, header = match.groups()
        if header == "raf_libc_emu.h":
            already_injected = True
            lines.append(line)
        elif header in EMULATED_HEADERS:
            stripped_headers.append(header)
            lines.append(f"/* RAF_REWRITE emulated <{header}> */\n")
        elif header in UNSUPPORTED_HOSTED_HEADERS or delimiter == "<":
            unresolved_headers.append(header)
        else:
            unresolved_headers.append(header)

    if unresolved_headers:
        raise ValueError(
            "unsupported/unresolved include(s): " + ", ".join(sorted(set(unresolved_headers)))
        )

    body = "".join(lines)
    masked = mask_comments_and_literals(body)
    forbidden: list[str] = []
    emulated: list[str] = []
    for name in sorted(FORBIDDEN_CALLS):
        if re.search(rf"\b{re.escape(name)}\s*\(", masked):
            forbidden.append(name)
    for token in sorted(FORBIDDEN_TOKENS):
        if re.search(rf"\b{re.escape(token)}\b", masked):
            forbidden.append(token)
    for name in sorted(EMULATED_CALLS):
        if re.search(rf"\b{re.escape(name)}\s*\(", masked):
            emulated.append(name)

    if forbidden:
        raise ValueError("forbidden hosted/runtime token(s): " + ", ".join(forbidden))

    rewritten = body if already_injected else INJECT + body
    manifest = {
        "schema": SCHEMA,
        "stage": "SOURCE_REWRITE_ONLY",
        "input_sha256": sha256_text(src),
        "output_sha256": sha256_text(rewritten),
        "stripped_headers": sorted(set(stripped_headers)),
        "emulated_calls": emulated,
        "unresolved_headers": [],
        "forbidden_calls": [],
        "heap": False,
        "claim_allowed": False,
        "promotion_gate": "STRICT_ELF_AUDIT_AND_REPRODUCIBILITY",
    }
    return rewritten, manifest


def selftest() -> int:
    src = (
        '#include <string.h>\n#include <stdint.h>\n'
        'uint32_t f(void){char a[4]; memset(a,0,4); strncpy(a,"x",sizeof(a)); return strlen(a);}\n'
    )
    out, manifest = rewrite(src)
    assert out.startswith(INJECT)
    assert "RAF_REWRITE emulated" in out
    assert manifest["emulated_calls"] == ["memset", "strlen", "strncpy"]
    assert manifest["claim_allowed"] is False

    for bad, expected in [
        ("void *f(void){ return malloc(4); }\n", "malloc"),
        ('char *f(char *d){ return strcpy(d,"x"); }\n', "strcpy"),
        ("#include <ctype.h>\nint f(int x){return isalpha(x);}\n", "ctype.h"),
        ('#include "project_local.h"\nint f(void){return 0;}\n', "project_local.h"),
    ]:
        try:
            rewrite(bad)
        except ValueError as exc:
            assert expected in str(exc)
        else:
            raise AssertionError(f"must reject: {expected}")

    rewrite('int f(void){ const char *s="malloc(4)"; /* free(0) */ return s[0]; }\n')
    print("raf_c_rewrite selftest: PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?")
    parser.add_argument("output", nargs="?")
    parser.add_argument("--manifest")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        return selftest()
    if not args.source or not args.output:
        parser.error("source and output are required")
    src = Path(args.source).read_text(encoding="utf-8")
    try:
        rewritten, manifest = rewrite(src)
    except ValueError as exc:
        print(f"raf_c_rewrite: FAIL — {exc}")
        return 65
    atomic_write_text(Path(args.output), rewritten)
    if args.manifest:
        atomic_write_text(
            Path(args.manifest),
            json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        )
    print(f"raf_c_rewrite: PASS — {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
