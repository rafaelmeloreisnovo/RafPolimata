#!/usr/bin/env python3
"""Lower one portable RAF_KERNEL annotation from a hosted source to strict C.

This is a small semantic bridge, not a compiler for the source language. Input,
AST size and recursive depth are bounded before C emission.
"""
from __future__ import annotations

import argparse
import ast
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path

SCHEMA = "rafaelia.kernel.lower.v2"
MAX_SOURCE_BYTES = 1 << 20
MAX_EXPRESSION_CHARS = 4096
MAX_AST_NODES = 256
MAX_AST_DEPTH = 64
ALLOWED_LANGUAGES = {
    "rs", "kt", "java", "py", "sh", "pl", "js", "php", "jsx",
    "go", "rb", "swift", "groovy", "clj",
}
MARKER = re.compile(
    r"^RAF_KERNEL\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*=\s*(.+?)\s*$"
)
SIMPLE_BINOPS = {
    ast.Add: "+", ast.Sub: "-", ast.Mult: "*",
    ast.BitAnd: "&", ast.BitOr: "|", ast.BitXor: "^",
}
ALLOWED_UNARY = {ast.UAdd: "+", ast.USub: "-", ast.Invert: "~"}
COMMENT_PREFIXES = ("//", "#", "/*", "*", "<!--", "--")
COMMENT_SUFFIXES = ("*/", "-->")


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def read_bounded_text(path: Path) -> str:
    raw = path.read_bytes()
    if not raw:
        raise ValueError("source is empty")
    if len(raw) > MAX_SOURCE_BYTES:
        raise ValueError(f"source exceeds {MAX_SOURCE_BYTES} bytes")
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError("source is not valid UTF-8") from exc


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


def marker_lines(src: str) -> list[re.Match[str]]:
    matches: list[re.Match[str]] = []
    for raw_line in src.splitlines():
        line = raw_line.strip()
        for prefix in COMMENT_PREFIXES:
            if line.startswith(prefix):
                line = line[len(prefix):].strip()
                break
        for suffix in COMMENT_SUFFIXES:
            if line.endswith(suffix):
                line = line[: -len(suffix)].rstrip()
                break
        match = MARKER.fullmatch(line)
        if match:
            matches.append(match)
    return matches


def normalize_integer_suffixes(expr: str) -> str:
    return re.sub(
        r"(?<![A-Za-z0-9_\.])(0[xX][0-9A-Fa-f]+|0[bB][01]+|[0-9]+)[uUlL]+\b",
        r"\1",
        expr,
    )


def constant_int(node: ast.AST) -> int | None:
    if isinstance(node, ast.Constant) and isinstance(node.value, int):
        return node.value
    if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.UAdd, ast.USub)):
        value = constant_int(node.operand)
        if value is None:
            return None
        return value if isinstance(node.op, ast.UAdd) else -value
    return None


def render_expr(node: ast.AST, names: set[str], depth: int = 0) -> str:
    if depth > MAX_AST_DEPTH:
        raise ValueError(f"expression depth exceeds {MAX_AST_DEPTH}")
    if isinstance(node, ast.Expression):
        return render_expr(node.body, names, depth + 1)
    if isinstance(node, ast.Constant) and isinstance(node.value, int):
        if node.value < -(1 << 63) or node.value > (1 << 64) - 1:
            raise ValueError("integer literal outside 64-bit range")
        return str(node.value)
    if isinstance(node, ast.Name) and node.id in names:
        return node.id
    if isinstance(node, ast.UnaryOp) and type(node.op) in ALLOWED_UNARY:
        return f"({ALLOWED_UNARY[type(node.op)]}{render_expr(node.operand, names, depth + 1)})"
    if isinstance(node, ast.BinOp):
        left = render_expr(node.left, names, depth + 1)
        right = render_expr(node.right, names, depth + 1)
        if type(node.op) in SIMPLE_BINOPS:
            return f"({left} {SIMPLE_BINOPS[type(node.op)]} {right})"
        if isinstance(node.op, (ast.Div, ast.Mod)):
            divisor = constant_int(node.right)
            if divisor is None:
                raise ValueError("division/modulo requires a nonzero constant divisor")
            if divisor == 0:
                raise ValueError("division/modulo by zero")
            operator = "/" if isinstance(node.op, ast.Div) else "%"
            return f"({left} {operator} {right})"
        if isinstance(node.op, (ast.LShift, ast.RShift)):
            shift = constant_int(node.right)
            if shift is None or shift < 0 or shift >= 32:
                raise ValueError("shift count must be a constant in [0, 31]")
            operator = "<<" if isinstance(node.op, ast.LShift) else ">>"
            return f"({left} {operator} {right})"
    raise ValueError(f"unsupported expression node: {type(node).__name__}")


def lower(src: str, language: str) -> tuple[str, dict[str, object]]:
    if len(src.encode("utf-8")) > MAX_SOURCE_BYTES:
        raise ValueError(f"source exceeds {MAX_SOURCE_BYTES} bytes")
    language = language.lower()
    if language not in ALLOWED_LANGUAGES:
        raise ValueError(f"unsupported hosted language route: {language}")

    matches = marker_lines(src)
    if len(matches) != 1:
        raise ValueError(f"expected exactly one RAF_KERNEL annotation, found {len(matches)}")
    match = matches[0]
    name = match.group(1)
    args = [item.strip() for item in match.group(2).split(",") if item.strip()]
    if len(args) > 4:
        raise ValueError("at most four uint32 arguments are supported")
    if len(set(args)) != len(args):
        raise ValueError("duplicate argument name")
    for arg in args:
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", arg):
            raise ValueError(f"invalid argument: {arg}")

    expr_text = normalize_integer_suffixes(match.group(3).strip().rstrip(";"))
    if not expr_text:
        raise ValueError("kernel expression is empty")
    if len(expr_text) > MAX_EXPRESSION_CHARS:
        raise ValueError(f"expression exceeds {MAX_EXPRESSION_CHARS} characters")
    try:
        tree = ast.parse(expr_text, mode="eval")
    except RecursionError as exc:
        raise ValueError("expression parser recursion limit exceeded") from exc
    node_count = sum(1 for _ in ast.walk(tree))
    if node_count > MAX_AST_NODES:
        raise ValueError(f"expression AST exceeds {MAX_AST_NODES} nodes")
    expression = render_expr(tree, set(args))

    signature = ", ".join(f"uint32_t {arg}" for arg in args) or "void"
    zero_args = ", ".join("0u" for _ in args)
    c_source = f'''#include "raf_libc_emu.h"

RAF_EXPORT uint32_t {name}({signature}) {{
    return (uint32_t)({expression});
}}

RAF_EXPORT void ANativeActivity_onCreate(void *activity, void *saved_state, size_t saved_state_size) {{
    (void)activity;
    (void)saved_state;
    (void)saved_state_size;
}}

RAF_EXPORT int android_main(void *app) {{
    (void)app;
    return (int){name}({zero_args});
}}
'''
    manifest = {
        "schema": SCHEMA,
        "stage": "ANNOTATION_LOWERING_ONLY",
        "language": language,
        "kernel": name,
        "arguments": args,
        "expression": expression,
        "integer_model": "uint32_modulo",
        "division_policy": "constant_nonzero_divisor_only",
        "shift_policy": "constant_0_to_31_only",
        "source_limit_bytes": MAX_SOURCE_BYTES,
        "expression_limit_chars": MAX_EXPRESSION_CHARS,
        "ast_node_limit": MAX_AST_NODES,
        "ast_depth_limit": MAX_AST_DEPTH,
        "ast_nodes": node_count,
        "input_sha256": sha256_text(src),
        "output_sha256": sha256_text(c_source),
        "runtime": "none_in_generated_c",
        "claim_allowed": False,
        "promotion_gate": "STRICT_ELF_AUDIT_AND_REPRODUCIBILITY",
    }
    return c_source, manifest


def selftest() -> int:
    c_source, manifest = lower("# RAF_KERNEL mix(a,b) = ((a ^ b) + 7u) << 1", "py")
    assert "uint32_t mix(uint32_t a, uint32_t b)" in c_source
    assert manifest["arguments"] == ["a", "b"]
    assert manifest["claim_allowed"] is False
    assert manifest["ast_nodes"] > 0
    assert "<< 1" in c_source

    too_many_nodes = "# RAF_KERNEL f(a) = " + "+".join("a" for _ in range(300))
    for bad in [
        ("# no marker", "py"),
        ("# RAF_KERNEL f(a) = open(a)", "py"),
        ("# RAF_KERNEL f(a,a) = a", "py"),
        ("# RAF_KERNEL f(a) = a / a", "py"),
        ("# RAF_KERNEL f(a) = a << a", "py"),
        ("# RAF_KERNEL f(a) = a // 2", "py"),
        ("# RAF_KERNEL f(a) = a", "unknown"),
        ('x = "RAF_KERNEL fake(a) = a"', "py"),
        (too_many_nodes, "py"),
    ]:
        try:
            lower(*bad)
        except (ValueError, SyntaxError, RecursionError):
            pass
        else:
            raise AssertionError(f"must reject: {bad[1]} / {bad[0][:80]}")
    print("raf_kernel_lower selftest: PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--language")
    parser.add_argument("--source")
    parser.add_argument("--output")
    parser.add_argument("--manifest")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        return selftest()
    if not all((args.language, args.source, args.output)):
        parser.error("--language, --source and --output are required")
    try:
        src = read_bounded_text(Path(args.source))
        c_source, manifest = lower(src, args.language)
    except (ValueError, SyntaxError, RecursionError, OSError) as exc:
        print(f"raf_kernel_lower: FAIL — {exc}")
        return 65
    atomic_write_text(Path(args.output), c_source)
    if args.manifest:
        atomic_write_text(
            Path(args.manifest),
            json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        )
    print(f"raf_kernel_lower: PASS — {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
