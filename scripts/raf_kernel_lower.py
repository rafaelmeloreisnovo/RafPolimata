#!/usr/bin/env python3
"""Lower one portable RAF_KERNEL annotation from any hosted language to strict C."""
from __future__ import annotations

import argparse
import ast
import hashlib
import json
import re
from pathlib import Path

SCHEMA = "rafaelia.kernel.lower.v1"
MARKER = re.compile(
    r"RAF_KERNEL\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*=\s*([^\r\n]+)"
)
ALLOWED_BINOPS = {
    ast.Add: "+", ast.Sub: "-", ast.Mult: "*", ast.FloorDiv: "/", ast.Div: "/",
    ast.Mod: "%", ast.BitAnd: "&", ast.BitOr: "|", ast.BitXor: "^",
    ast.LShift: "<<", ast.RShift: ">>",
}
ALLOWED_UNARY = {ast.UAdd: "+", ast.USub: "-", ast.Invert: "~"}


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def render_expr(node: ast.AST, names: set[str]) -> str:
    if isinstance(node, ast.Expression):
        return render_expr(node.body, names)
    if isinstance(node, ast.Constant) and isinstance(node.value, int):
        if node.value < -(1 << 63) or node.value > (1 << 64) - 1:
            raise ValueError("integer literal outside 64-bit range")
        return str(node.value)
    if isinstance(node, ast.Name) and node.id in names:
        return node.id
    if isinstance(node, ast.BinOp) and type(node.op) in ALLOWED_BINOPS:
        left = render_expr(node.left, names)
        right = render_expr(node.right, names)
        return f"({left} {ALLOWED_BINOPS[type(node.op)]} {right})"
    if isinstance(node, ast.UnaryOp) and type(node.op) in ALLOWED_UNARY:
        return f"({ALLOWED_UNARY[type(node.op)]}{render_expr(node.operand, names)})"
    raise ValueError(f"unsupported expression node: {type(node).__name__}")


def lower(src: str, language: str) -> tuple[str, dict[str, object]]:
    matches = list(MARKER.finditer(src))
    if len(matches) != 1:
        raise ValueError(f"expected exactly one RAF_KERNEL marker, found {len(matches)}")
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
    expr_text = match.group(3).strip()
    expr_text = re.sub(r"\s*(?:\*/|-->)\s*$", "", expr_text).rstrip().rstrip(";")
    tree = ast.parse(expr_text, mode="eval")
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
        "language": language,
        "kernel": name,
        "arguments": args,
        "expression": expression,
        "input_sha256": sha256_text(src),
        "output_sha256": sha256_text(c_source),
        "runtime": "none",
        "claim_allowed": True,
    }
    return c_source, manifest


def selftest() -> int:
    c_source, manifest = lower("# RAF_KERNEL mix(a,b) = (a ^ b) + 7", "py")
    assert "uint32_t mix(uint32_t a, uint32_t b)" in c_source
    assert manifest["arguments"] == ["a", "b"]
    for bad in [
        "# no marker",
        "# RAF_KERNEL f(a) = open(a)",
        "# RAF_KERNEL f(a,a) = a",
    ]:
        try:
            lower(bad, "py")
        except (ValueError, SyntaxError):
            pass
        else:
            raise AssertionError(f"must reject: {bad}")
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
    src = Path(args.source).read_text(encoding="utf-8")
    try:
        c_source, manifest = lower(src, args.language.lower())
    except (ValueError, SyntaxError) as exc:
        print(f"raf_kernel_lower: FAIL — {exc}")
        return 65
    Path(args.output).write_text(c_source, encoding="utf-8")
    if args.manifest:
        Path(args.manifest).write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    print(f"raf_kernel_lower: PASS — {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
