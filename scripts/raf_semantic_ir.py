#!/usr/bin/env python3
"""Lower a strict multi-language scalar subset to deterministic RAF Semantic IR.

V1 is intentionally small: one unsigned u32/u64 return expression, no heap,
I/O, calls, loops, exceptions or hidden runtime. Unsupported syntax fails closed.
The x32/x64 command emits freestanding ELF specimens; it never installs or
launches Android applications.
"""
from __future__ import annotations

import argparse
import datetime as dt
import errno
import hashlib
import json
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess
from typing import Any

CONTRACT_SCHEMA = "rafaelia.semantic-source-set.v1"
IR_SCHEMA = "rafaelia.semantic-ir.v1"
LANGUAGES = {
    "asm", "c", "cpp", "rs", "kt", "java", "py", "sh", "pl", "js",
    "php", "jsx", "go", "rb", "swift", "groovy", "clj", "glsl", "cl",
    "hlsl", "wgsl", "dsp", "tflite",
}
FRONTENDS = {"c", "cpp", "rs", "kt", "java", "py", "js", "php", "go", "rb", "swift", "groovy"}
BINARY = {"+": "add", "-": "sub", "*": "mul", "&": "and", "|": "or", "^": "xor", "<<": "shl", ">>": "shr"}
UNARY = {"~": "not", "-": "neg", "+": "pos"}
PRECEDENCE = {"|": 10, "^": 20, "&": 30, "<<": 40, ">>": 40, "+": 50, "-": 50, "*": 60}
COMMUTATIVE = {"add", "mul", "and", "or", "xor"}
TOKEN_RE = re.compile(r"\s*(?:(0[xX][0-9A-Fa-f]+|[0-9]+)|([A-Za-z_][A-Za-z0-9_]*)|(<<|>>|[()+\-*~&|^]))")


class SemanticError(RuntimeError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode()


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def canonicalize(node: tuple) -> tuple:
    if node[0] in {"var", "const"}:
        return node
    children = tuple(canonicalize(child) for child in node[1:])
    if node[0] in COMMUTATIVE and len(children) == 2:
        children = tuple(sorted(children, key=canonical_bytes))
    return (node[0], *children)


class Parser:
    def __init__(self, expression: str, params: set[str]):
        self.expression = expression
        self.params = params
        self.tokens = self._tokens(expression)
        self.index = 0

    @staticmethod
    def _tokens(expression: str) -> list[tuple[str, str, int]]:
        result: list[tuple[str, str, int]] = []
        offset = 0
        while offset < len(expression):
            match = TOKEN_RE.match(expression, offset)
            if not match:
                raise SemanticError(f"unsupported syntax at {offset}: {expression[offset:offset+24]!r}")
            number, identifier, operator = match.groups()
            result.append(("number" if number else "identifier" if identifier else "operator", number or identifier or operator, offset))
            offset = match.end()
        result.append(("eof", "", offset))
        return result

    def current(self) -> tuple[str, str, int]:
        return self.tokens[self.index]

    def advance(self) -> tuple[str, str, int]:
        token = self.current()
        self.index += 1
        return token

    def parse(self) -> tuple:
        root = self.expression_at(0)
        if self.current()[0] != "eof":
            raise SemanticError(f"unexpected token {self.current()[1]!r}")
        return canonicalize(root)

    def expression_at(self, minimum: int) -> tuple:
        kind, value, offset = self.advance()
        if kind == "number":
            left = ("const", int(value, 0))
        elif kind == "identifier":
            if value not in self.params:
                raise SemanticError(f"unknown identifier: {value}")
            left = ("var", value)
        elif kind == "operator" and value in UNARY:
            left = (UNARY[value], self.expression_at(70))
        elif kind == "operator" and value == "(":
            left = self.expression_at(0)
            close = self.advance()
            if close[:2] != ("operator", ")"):
                raise SemanticError(f"missing ')' near {close[2]}")
        else:
            raise SemanticError(f"unexpected token {value!r} at {offset}")
        while self.current()[0] == "operator" and self.current()[1] in PRECEDENCE:
            operator = self.current()[1]
            precedence = PRECEDENCE[operator]
            if precedence < minimum:
                break
            self.advance()
            left = (BINARY[operator], left, self.expression_at(precedence + 1))
        return left


def extract_expression(language: str, source: str) -> str:
    if language not in FRONTENDS:
        raise SemanticError(f"frontend {language!r} is registered but not executable in v1")
    text = re.sub(r"/\*.*?\*/", " ", source, flags=re.S)
    text = re.sub(r"#.*$" if language in {"py", "rb"} else r"//.*$", "", text, flags=re.M)
    found = re.search(r"\breturn\s+(.+?)(?:;|\n|\}|$)", text, flags=re.S)
    if found:
        expression = found.group(1)
    elif language == "kt":
        end = text.find(")")
        equals = text.find("=", end + 1)
        if end < 0 or equals < 0:
            raise SemanticError("Kotlin subset requires return or expression body")
        expression = text[equals + 1:]
    elif language == "rb":
        body = [line.strip() for line in text.splitlines() if line.strip() and not line.strip().startswith(("def ", "end"))]
        if not body:
            raise SemanticError("Ruby expression body absent")
        expression = body[-1]
    else:
        raise SemanticError(f"frontend {language!r} requires one return expression")
    expression = expression.strip().rstrip(";}").strip()
    if language in {"kt", "java", "groovy"}:
        for word, token in {"xor": "^", "and": "&", "or": "|", "shl": "<<", "shr": ">>"}.items():
            expression = re.sub(rf"\b{word}\b", token, expression)
    if language == "php":
        expression = re.sub(r"\$([A-Za-z_][A-Za-z0-9_]*)", r"\1", expression)
    return expression


def validate_contract(contract: dict[str, Any]) -> tuple[int, list[str]]:
    if contract.get("schema") != CONTRACT_SCHEMA or contract.get("claim_allowed") is not False:
        raise SemanticError("invalid schema or claim_allowed")
    integer = contract.get("integer", {})
    bits = integer.get("bits")
    if bits not in {32, 64} or integer.get("signed") is not False or integer.get("overflow") != "wrap":
        raise SemanticError("v1 requires unsigned u32/u64 wrap semantics")
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]{0,63}", str(contract.get("kernel", ""))):
        raise SemanticError("invalid kernel name")
    params = contract.get("params")
    if not isinstance(params, list) or not params:
        raise SemanticError("params required")
    names: list[str] = []
    for item in params:
        name = item.get("name") if isinstance(item, dict) else None
        if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name) or name in names:
            raise SemanticError(f"invalid or duplicate parameter: {name!r}")
        if item.get("type") != f"u{bits}":
            raise SemanticError(f"parameter {name} must be u{bits}")
        names.append(name)
    sources = contract.get("sources")
    if not isinstance(sources, dict) or len(sources) < 2:
        raise SemanticError("at least two source variants required")
    if any(language not in LANGUAGES or not isinstance(source, str) or not source.strip() for language, source in sources.items()):
        raise SemanticError("invalid language/source entry")
    vectors = contract.get("vectors")
    if not isinstance(vectors, list) or not vectors:
        raise SemanticError("vectors required")
    for vector in vectors:
        if not isinstance(vector, dict) or set(vector.get("inputs", {})) != set(names):
            raise SemanticError("vector inputs differ from params")
        if any(isinstance(value, bool) or not isinstance(value, int) for value in vector["inputs"].values()):
            raise SemanticError("vector inputs must be integers")
    return bits, names


def evaluate(node: tuple, inputs: dict[str, int], bits: int) -> int:
    mask = (1 << bits) - 1
    if node[0] == "var": return int(inputs[node[1]]) & mask
    if node[0] == "const": return int(node[1]) & mask
    values = [evaluate(child, inputs, bits) for child in node[1:]]
    op = node[0]
    if op == "add": value = values[0] + values[1]
    elif op == "sub": value = values[0] - values[1]
    elif op == "mul": value = values[0] * values[1]
    elif op == "and": value = values[0] & values[1]
    elif op == "or": value = values[0] | values[1]
    elif op == "xor": value = values[0] ^ values[1]
    elif op == "shl": value = values[0] << (values[1] & (bits - 1))
    elif op == "shr": value = values[0] >> (values[1] & (bits - 1))
    elif op == "not": value = ~values[0]
    elif op == "neg": value = -values[0]
    elif op == "pos": value = values[0]
    else: raise SemanticError(f"unsupported op: {op}")
    return value & mask


def lower_ssa(root: tuple, params: list[str]) -> dict[str, Any]:
    nodes: list[dict[str, Any]] = []
    memo: dict[bytes, str] = {}
    def visit(node: tuple) -> str:
        key = canonical_bytes(node)
        if key in memo: return memo[key]
        if node[0] == "var": ref = "@" + node[1]
        elif node[0] == "const": ref = "#" + str(node[1])
        else:
            ref = f"%{len(nodes)}"
            nodes.append({"id": ref, "op": node[0], "args": [visit(child) for child in node[1:]]})
        memo[key] = ref
        return ref
    return {"params": params, "nodes": nodes, "result": visit(root)}


def compile_source_set(contract: dict[str, Any]) -> dict[str, Any]:
    bits, params = validate_contract(contract)
    roots: dict[str, tuple] = {}
    languages: dict[str, Any] = {}
    for language, source in sorted(contract["sources"].items()):
        expression = extract_expression(language, source)
        root = Parser(expression, set(params)).parse()
        roots[language] = root
        languages[language] = {"expression": expression, "source_sha256": sha256_bytes(source.encode()), "tree_sha256": sha256_bytes(canonical_bytes(root))}
    if len({canonical_bytes(root) for root in roots.values()}) != 1:
        raise SemanticError("semantic mismatch across frontends")
    root = next(iter(roots.values()))
    identity = {"integer": contract["integer"], "params": contract["params"], "tree": root}
    kernel_id = "rafk1-" + sha256_bytes(canonical_bytes(identity))
    mask = (1 << bits) - 1
    vectors = []
    for index, vector in enumerate(contract["vectors"]):
        observed = evaluate(root, vector["inputs"], bits)
        expected = vector.get("expected", observed)
        if isinstance(expected, bool) or not isinstance(expected, int) or (expected & mask) != observed:
            raise SemanticError(f"vector {index} expected mismatch")
        vectors.append({"id": vector.get("id", f"v{index:03d}"), "inputs": vector["inputs"], "observed": observed, "state": "PASS"})
    return {
        "schema": IR_SCHEMA, "kernel": contract["kernel"], "kernel_id": kernel_id,
        "integer": contract["integer"], "params": contract["params"], "canonical_tree": root,
        "ssa": lower_ssa(root, params), "languages": languages, "vectors": vectors,
        "semantic_equivalence": "PASS", "frontend_scope": "STRICT_SINGLE_EXPRESSION_UNSIGNED_WRAP_SUBSET",
        "claim_allowed": False,
    }


def c_expression(node: tuple, ctype: str) -> str:
    if node[0] == "var": return node[1]
    if node[0] == "const": return f"(({ctype})0x{node[1]:x}u)"
    if node[0] in {"not", "neg", "pos"}:
        token = {"not": "~", "neg": "-", "pos": "+"}[node[0]]
        return f"({token}{c_expression(node[1], ctype)})"
    token = {value: key for key, value in BINARY.items()}[node[0]]
    return f"(({c_expression(node[1], ctype)}) {token} ({c_expression(node[2], ctype)}))"


def emit_c(ir: dict[str, Any]) -> str:
    bits = ir["integer"]["bits"]
    ctype, base = ("raf_u32", "unsigned int") if bits == 32 else ("raf_u64", "unsigned long long")
    params = ", ".join(f"{ctype} {item['name']}" for item in ir["params"])
    return (
        "/* Generated from RAF Semantic IR. */\n"
        f"/* Kernel-ID: {ir['kernel_id']} */\n"
        f"typedef {base} {ctype};\n"
        f"_Static_assert(sizeof({ctype}) * 8u == {bits}u, \"unexpected width\");\n"
        f"{ctype} {ir['kernel']}({params})\n{{\n    return ({ctype})({c_expression(ir['canonical_tree'], ctype)});\n}}\n"
    )


def assembly_for_bound_vector(ir: dict[str, Any], inputs: dict[str, int], arch: str) -> str:
    if ir["integer"]["bits"] != 32 or arch not in {"i386", "x86_64"}:
        raise SemanticError("x32/x64 harness v1 requires u32 and i386/x86_64")
    push, pop_a, pop_c = (("pushl %eax", "popl %eax", "popl %ecx") if arch == "i386" else ("pushq %rax", "popq %rax", "popq %rcx"))
    lines = [".global _start", ".section .text", "_start:"]
    def emit(node: tuple) -> None:
        if node[0] in {"var", "const"}:
            value = (inputs[node[1]] if node[0] == "var" else node[1]) & 0xFFFFFFFF
            lines.extend([f"    movl $0x{value:08x}, %eax", f"    {push}"])
            return
        if node[0] in {"not", "neg", "pos"}:
            emit(node[1]); lines.append(f"    {pop_a}")
            if node[0] == "not": lines.append("    notl %eax")
            elif node[0] == "neg": lines.append("    negl %eax")
            lines.append(f"    {push}"); return
        emit(node[1]); emit(node[2]); lines.extend([f"    {pop_c}", f"    {pop_a}"])
        instruction = {"add":"addl %ecx, %eax","sub":"subl %ecx, %eax","mul":"imull %ecx, %eax","and":"andl %ecx, %eax","or":"orl %ecx, %eax","xor":"xorl %ecx, %eax","shl":"shll %cl, %eax","shr":"shrl %cl, %eax"}.get(node[0])
        if not instruction: raise SemanticError(f"no x86 lowering for {node[0]}")
        lines.extend([f"    {instruction}", f"    {push}"])
    emit(ir["canonical_tree"]); lines.append(f"    {pop_a}")
    lines.extend(["    movzbl %al, %ebx", "    movl $1, %eax", "    int $0x80"] if arch == "i386" else ["    movzbl %al, %edi", "    movl $60, %eax", "    syscall"])
    return "\n".join(lines) + "\n"


def command_version(name: str) -> str:
    path = shutil.which(name)
    if not path: return "TOKEN_VAZIO_COMMAND_ABSENT"
    result = subprocess.run([path, "--version"], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return f"{path}: {(result.stdout.strip().splitlines() or ['unknown'])[0][:200]}"


def run(command: list[str], cwd: Path) -> dict[str, Any]:
    result = subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return {"command": shlex.join(command), "exit_code": result.returncode, "stdout": result.stdout[-2000:], "stderr": result.stderr[-2000:]}


def execute_elf(path: Path, arch: str) -> dict[str, Any]:
    emulator = shutil.which("qemu-i386" if arch == "i386" else "qemu-x86_64")
    if emulator: command, executor = [emulator, str(path)], Path(emulator).name
    elif arch == "x86_64" and platform.machine().lower() in {"x86_64", "amd64"}: command, executor = [str(path)], "NATIVE_X86_64_FALLBACK"
    else: command, executor = [str(path)], "NATIVE_COMPATIBILITY_PROBE"
    try:
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except OSError as exc:
        return {"state": "TOKEN_VAZIO_EMULATOR_ABSENT" if exc.errno == errno.ENOEXEC else "FAIL_EXECUTOR", "executor": executor, "error": str(exc)}
    return {"state": "EXECUTED", "executor": executor, "exit_code": result.returncode}


def x3264_harness(ir: dict[str, Any], out_dir: Path) -> dict[str, Any]:
    out_dir = out_dir.resolve(); out_dir.mkdir(parents=True, exist_ok=True)
    cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc: raise SemanticError("local compiler driver absent")
    vectors, states = [], {"i386": [], "x86_64": []}
    for index, vector in enumerate(ir["vectors"]):
        item = {"id": vector["id"], "expected_exit": vector["observed"] & 0xFF, "architectures": {}}
        for arch, flag in (("i386", "-m32"), ("x86_64", "-m64")):
            asm, elf = out_dir / f"{index:02d}-{arch}.S", out_dir / f"{index:02d}-{arch}.elf"
            asm.write_text(assembly_for_bound_vector(ir, vector["inputs"], arch))
            compile_result = run([cc, flag, "-nostdlib", "-nostartfiles", "-nodefaultlibs", "-no-pie", "-Wl,--build-id=none", str(asm), "-o", str(elf)], out_dir)
            record: dict[str, Any] = {"assembly_sha256": sha256_file(asm), "compile": compile_result}
            if compile_result["exit_code"] or not elf.is_file(): record["state"] = "FAIL_COMPILE"
            else:
                record["elf_sha256"] = sha256_file(elf)
                record["readelf"] = run(["readelf", "-h", str(elf)], out_dir)
                execution = execute_elf(elf, arch); record["execution"] = execution
                if execution["state"] != "EXECUTED": record["state"] = execution["state"]
                elif execution["exit_code"] != item["expected_exit"]: record["state"] = "FAIL_SEMANTIC_MISMATCH"
                else: record["state"] = "PASS_VIRTUALIZED" if str(execution["executor"]).startswith("qemu-") else "PASS_NATIVE"
            item["architectures"][arch] = record; states[arch].append(record["state"])
        vectors.append(item)
    i386 = all(state in {"PASS_VIRTUALIZED", "PASS_NATIVE"} for state in states["i386"])
    x64 = all(state in {"PASS_VIRTUALIZED", "PASS_NATIVE"} for state in states["x86_64"])
    virtual = all(state == "PASS_VIRTUALIZED" for state in states["i386"] + states["x86_64"])
    if i386 and x64 and virtual: state = "PASS_X32_X64_VIRTUALIZED"
    elif i386 and x64: state = "PASS_X32_X64_MIXED_EXECUTION"
    elif x64 and all(value == "TOKEN_VAZIO_EMULATOR_ABSENT" for value in states["i386"]): state = "PARTIAL_X64_PASS_X32_TOKEN_VAZIO_EMULATOR_ABSENT"
    else: state = "FAIL_OR_INCOMPLETE"
    return {
        "schema":"rafaelia.semantic-x3264-receipt.v1", "created_at":now_utc(), "state":state,
        "kernel":ir["kernel"], "kernel_id":ir["kernel_id"], "semantic_ir_state":ir["semantic_equivalence"],
        "host":{"platform":platform.platform(),"machine":platform.machine()},
        "toolchain":{"cc":command_version(Path(cc).name),"readelf":command_version("readelf"),"qemu_i386":command_version("qemu-i386"),"qemu_x86_64":command_version("qemu-x86_64")},
        "vectors":vectors, "install_executed":False, "android_launch_executed":False, "claim_allowed":False,
    }


def load(path: Path) -> dict[str, Any]:
    try: value = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc: raise SemanticError(f"cannot read contract: {exc}") from exc
    if not isinstance(value, dict): raise SemanticError("contract root must be object")
    return value


def write_json(path: Path | None, value: Any) -> None:
    text = json.dumps(value, indent=2, sort_keys=True) + "\n"
    if path: path.parent.mkdir(parents=True, exist_ok=True); path.write_text(text)
    print(text, end="")


def main() -> int:
    parser = argparse.ArgumentParser(); sub = parser.add_subparsers(dest="command", required=True)
    validate = sub.add_parser("validate"); validate.add_argument("contract", type=Path); validate.add_argument("--out", type=Path)
    emit = sub.add_parser("emit-c"); emit.add_argument("contract", type=Path); emit.add_argument("--out", type=Path, required=True)
    cross = sub.add_parser("x3264"); cross.add_argument("contract", type=Path); cross.add_argument("--out-dir", type=Path, required=True); cross.add_argument("--receipt", type=Path, required=True); cross.add_argument("--require-virtualized-both", action="store_true")
    args = parser.parse_args()
    try:
        contract = load(args.contract); ir = compile_source_set(contract)
        if args.command == "validate":
            receipt = {"schema":"rafaelia.semantic-lowering-receipt.v1","created_at":now_utc(),"state":"PASS_SEMANTIC_EQUIVALENCE","contract_sha256":sha256_file(args.contract),"kernel_id":ir["kernel_id"],"language_count":len(ir["languages"]),"vector_count":len(ir["vectors"]),"semantic_ir":ir,"claim_allowed":False}
            write_json(args.out, receipt); return 0
        if args.command == "emit-c":
            args.out.parent.mkdir(parents=True, exist_ok=True); args.out.write_text(emit_c(ir)); write_json(None,{"state":"PASS","path":str(args.out),"sha256":sha256_file(args.out),"kernel_id":ir["kernel_id"],"claim_allowed":False}); return 0
        receipt = x3264_harness(ir, args.out_dir); write_json(args.receipt, receipt)
        return 0 if (receipt["state"] == "PASS_X32_X64_VIRTUALIZED" if args.require_virtualized_both else receipt["state"] != "FAIL_OR_INCOMPLETE") else 1
    except SemanticError as exc:
        output = getattr(args, "out", None) or getattr(args, "receipt", None); write_json(output,{"state":"FAIL","error":str(exc),"claim_allowed":False}); return 1


if __name__ == "__main__":
    raise SystemExit(main())
