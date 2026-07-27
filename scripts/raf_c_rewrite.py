#!/usr/bin/env python3
"""Rewrite a bounded C/C++ subset onto RAFAELIA's freestanding compatibility layer."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

SCHEMA = "rafaelia.c.rewrite.v1"
REPLACED_HEADERS = {
    "assert.h", "ctype.h", "errno.h", "inttypes.h", "limits.h", "stdbool.h",
    "stddef.h", "stdint.h", "stdio.h", "stdlib.h", "string.h", "strings.h",
}
EMULATED_CALLS = {
    "memcpy", "memmove", "memset", "memcmp", "strlen", "strnlen", "strcmp",
    "strncmp", "strchr", "strrchr", "atoi", "strtoul", "putchar", "puts",
}
FORBIDDEN_CALLS = {
    "malloc", "calloc", "realloc", "free", "printf", "fprintf", "sprintf",
    "snprintf", "vprintf", "vfprintf", "fopen", "fdopen", "fread", "fwrite",
    "fseek", "ftell", "fclose", "system", "popen", "dlopen", "dlsym",
    "pthread_create", "fork", "execve", "setjmp", "longjmp", "atexit",
}
FORBIDDEN_TOKENS = {"FILE", "jmp_buf", "pthread_t"}
INJECT = '#include "raf_libc_emu.h"\n'


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


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
    lines: list[str] = []
    include_re = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]\s*(?://.*)?$')
    already_injected = False
    for line in src.splitlines(keepends=True):
        match = include_re.match(line.rstrip("\r\n"))
        if match:
            header = match.group(1)
            if header == "raf_libc_emu.h":
                already_injected = True
                lines.append(line)
                continue
            if header in REPLACED_HEADERS:
                stripped_headers.append(header)
                lines.append(f"/* RAF_REWRITE stripped <{header}> */\n")
                continue
        lines.append(line)

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
        "input_sha256": sha256_text(src),
        "output_sha256": sha256_text(rewritten),
        "stripped_headers": sorted(set(stripped_headers)),
        "emulated_calls": emulated,
        "forbidden_calls": [],
        "heap": False,
        "claim_allowed": True,
    }
    return rewritten, manifest


def selftest() -> int:
    src = '#include <string.h>\n#include <stdint.h>\nuint32_t f(void){char a[4]; memset(a,0,4); return strlen(a);}\n'
    out, manifest = rewrite(src)
    assert out.startswith(INJECT)
    assert "<string.h>" in out and "RAF_REWRITE stripped" in out
    assert manifest["emulated_calls"] == ["memset", "strlen"]
    try:
        rewrite("void *f(void){ return malloc(4); }\n")
    except ValueError as exc:
        assert "malloc" in str(exc)
    else:
        raise AssertionError("malloc must fail closed")
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
    Path(args.output).write_text(rewritten, encoding="utf-8")
    if args.manifest:
        Path(args.manifest).write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    print(f"raf_c_rewrite: PASS — {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
