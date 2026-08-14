#!/usr/bin/env python3
"""Fail-closed static audit for browser and TLS capability claims.

This tool intentionally distinguishes:
- a terminal/local file browser;
- an HTTPS transfer adapter backed by curl;
- a web browser with HTTP/TLS/certificate/rendering subsystems;
- an assembly implementation of that complete browser.

Static source evidence never becomes runtime certification by itself.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Iterable

SCHEMA = "raf.browser-tls-capability-audit.v1"
SOURCE_SUFFIXES = {
    ".c", ".h", ".cc", ".cpp", ".s", ".S", ".asm", ".py", ".sh",
    ".kt", ".java", ".rs", ".go",
}
TEXT_LIMIT = 2_000_000

PATTERNS: dict[str, tuple[re.Pattern[str], ...]] = {
    "terminal_rendering": (re.compile(r"VT100|tty_goto|terminal UI|TUI", re.I),),
    "local_directory_listing": (re.compile(r"dirbrowse|directory listing|file browser", re.I),),
    "keyboard_navigation": (re.compile(r"TAB=Panel|keyboard|KEY_UP|KEY_DOWN|ENTER=Run", re.I),),
    "url_parser": (re.compile(r"urlsplit|parse_url|URL parser|https?://", re.I),),
    "dns_resolution": (re.compile(r"getaddrinfo|res_query|DNS resolution|remote_ip", re.I),),
    "tcp_transport": (re.compile(r"socket\s*\(|connect\s*\(|TCP transport|curl", re.I),),
    "http_response_parser": (re.compile(r"HTTP/[123]|status[-_ ]line|response headers?|chunked", re.I),),
    "https_transport": (re.compile(r"HTTPS_TRANSPORT_ADAPTER|--proto '=https'|TLS handshake", re.I),),
    "tls_1_2_handshake": (re.compile(r"TLS1[_ .-]?2|TLSv1\.2|--tlsv1\.2", re.I),),
    "tls_1_3_handshake": (re.compile(r"TLS1[_ .-]?3|TLSv1\.3|--tlsv1\.3", re.I),),
    "x509_chain_validation": (re.compile(r"X509_verify_cert|x509 chain|certificate chain|ssl_verify_result", re.I),),
    "hostname_verification": (re.compile(r"hostname verification|X509_check_host|certificate_and_hostname_validation", re.I),),
    "certificate_time_validation": (re.compile(r"notBefore|notAfter|certificate time|expiry validation", re.I),),
    "trusted_root_store": (re.compile(r"CA bundle|root store|--cacert|system CA", re.I),),
    "redirect_policy": (re.compile(r"max-redirs|redirect policy|--proto-redir", re.I),),
    "content_renderer": (re.compile(r"HTML renderer|layout engine|DOM tree|render_html|WebView", re.I),),
    "navigation_history": (re.compile(r"navigation history|history stack|back_forward|goBack\s*\(", re.I),),
    "runtime_integration_test": (re.compile(r"browser.*integration.*PASS|tls.*transcript.*PASS|https.*runtime.*PASS", re.I),),
    "assembly_source_core": (re.compile(r"\.global|\.globl|section \.text|__asm__|asm volatile", re.I),),
    "abi_contract": (re.compile(r"AAPCS|ABI contract|callee-saved|caller-saved", re.I),),
    "memory_bounds_evidence": (re.compile(r"bounds test|buffer bounds|ASAN|memory safety.*PASS", re.I),),
    "crypto_known_answer_tests": (re.compile(r"known answer test|KAT.*PASS|RFC.*test vector", re.I),),
    "tls_transcript_tests": (re.compile(r"TLS transcript|handshake transcript", re.I),),
    "certificate_chain_fixtures": (re.compile(r"certificate fixture|test root CA|intermediate CA fixture", re.I),),
    "independent_security_review": (re.compile(r"independent security review|external security audit", re.I),),
}


def run_git(root: Path, args: list[str]) -> str | None:
    try:
        proc = subprocess.run(
            ["git", *args], cwd=root, check=False, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True,
        )
    except OSError:
        return None
    return proc.stdout if proc.returncode == 0 else None


def tracked_files(root: Path) -> list[Path]:
    raw = run_git(root, ["ls-files", "-z"])
    if raw is not None:
        return sorted(
            (root / item for item in raw.split("\0") if item),
            key=lambda p: p.as_posix(),
        )
    return sorted((p for p in root.rglob("*") if p.is_file()), key=lambda p: p.as_posix())


def read_text(path: Path) -> str:
    try:
        if path.stat().st_size > TEXT_LIMIT:
            return ""
        raw = path.read_bytes()
    except OSError:
        return ""
    if b"\0" in raw[:8192]:
        return ""
    return raw.decode("utf-8", errors="replace")


def collect_evidence(root: Path, paths: Iterable[Path]) -> dict[str, list[str]]:
    evidence: dict[str, list[str]] = {name: [] for name in PATTERNS}
    for path in paths:
        if path.suffix not in SOURCE_SUFFIXES:
            continue
        rel = path.relative_to(root).as_posix()
        if rel.startswith(("results/", "docs/generated/")):
            continue
        text = read_text(path)
        if not text:
            continue
        for capability, regexes in PATTERNS.items():
            if any(regex.search(text) for regex in regexes):
                evidence[capability].append(rel)
    return {key: sorted(set(value)) for key, value in evidence.items()}


def evaluate_level(name: str, requirements: list[str], evidence: dict[str, list[str]]) -> dict[str, object]:
    present = [item for item in requirements if evidence.get(item)]
    missing = [item for item in requirements if not evidence.get(item)]
    return {
        "level": name,
        "state": "STATIC_EVIDENCE" if not missing else ("SOURCE_PRESENT" if present else "ABSENT"),
        "requirements_total": len(requirements),
        "requirements_present": len(present),
        "present": present,
        "missing": missing,
        "evidence": {item: evidence[item] for item in present},
        "runtime_state": "TOKEN_VAZIO",
        "claim_allowed": False,
    }


def active_shell_option(text: str, option_pattern: str) -> bool:
    """Detect an option in executable shell lines, excluding comments/documentation.

    This intentionally avoids global substring scans: a safety comment such as
    `No -k/--insecure` is evidence *about* policy and must not be mistaken for an
    executed unsafe option. The match is restricted to option-shaped shell lines.
    """
    regex = re.compile(rf"(?m)^\s*(?:{option_pattern})(?:\s|\\|$)")
    for line in text.splitlines():
        if line.lstrip().startswith("#"):
            continue
        if regex.search(line):
            return True
    return False


def audit(root: Path, config_path: Path) -> dict[str, object]:
    config = json.loads(config_path.read_text(encoding="utf-8"))
    evidence = collect_evidence(root, tracked_files(root))
    levels: dict[str, dict[str, object]] = {}
    for level, requirements in config["capability_levels"].items():
        levels[level] = evaluate_level(level, list(requirements), evidence)

    # An HTTPS adapter is source-complete only when the audited script itself
    # contains the policy markers. A static contract still does not prove a TLS
    # handshake. Unsafe flags are searched only as executable shell options, not
    # as ambiguous global substrings in comments or help text.
    adapter_path = root / "scripts/raf_https_fetch.sh"
    adapter_text = read_text(adapter_path) if adapter_path.is_file() else ""
    insecure_option_active = active_shell_option(adapter_text, r"-k|--insecure")
    adapter_markers = {
        "https_only_policy": "--proto '=https'" in adapter_text and "--proto-redir '=https'" in adapter_text,
        "tls_1_2_request": "--tlsv1.2" in adapter_text,
        "tls_1_3_request": "--tlsv1.3" in adapter_text,
        "system_ca_validation": not insecure_option_active and "No -k/--insecure" in adapter_text,
        "hostname_verification": "certificate_and_hostname_validation_enabled" in adapter_text,
        "redirect_limit": "--max-redirs" in adapter_text,
        "timeout": "--connect-timeout" in adapter_text and "--max-time" in adapter_text,
        "atomic_output": "mv -f" in adapter_text,
        "evidence_report": "raf.https-fetch-evidence.v1" in adapter_text,
    }
    adapter_missing = sorted(key for key, value in adapter_markers.items() if not value)
    levels["HTTPS_TRANSPORT_ADAPTER"]["static_contract"] = adapter_markers
    levels["HTTPS_TRANSPORT_ADAPTER"]["unsafe_option_active"] = insecure_option_active
    levels["HTTPS_TRANSPORT_ADAPTER"]["state"] = (
        "STATIC_EVIDENCE" if adapter_path.is_file() and not adapter_missing else
        "SOURCE_PRESENT" if adapter_path.is_file() else "ABSENT"
    )
    levels["HTTPS_TRANSPORT_ADAPTER"]["missing"] = adapter_missing
    levels["HTTPS_TRANSPORT_ADAPTER"]["runtime_state"] = "TOKEN_VAZIO"

    tui_ok = not levels["TUI_FILE_BROWSER"]["missing"]
    transport_ok = adapter_path.is_file() and not adapter_missing
    web_ok = not levels["WEB_BROWSER_TLS"]["missing"]
    asm_ok = web_ok and not levels["ASM_WEB_BROWSER_TLS"]["missing"]

    summary_state = "PASS" if asm_ok else "REVIEW_REQUIRED" if (tui_ok or transport_ok) else "TOKEN_VAZIO"
    return {
        "schema": SCHEMA,
        "state": summary_state,
        "repository_commit": (run_git(root, ["rev-parse", "HEAD"]) or "TOKEN_VAZIO").strip(),
        "levels": levels,
        "facts": {
            "tui_file_browser_static_evidence": tui_ok,
            "https_transport_adapter_static_evidence": transport_ok,
            "web_browser_tls_static_evidence": web_ok,
            "asm_web_browser_tls_static_evidence": asm_ok,
        },
        "truth": {
            "raf_shell_classification": "TUI_FILE_BROWSER" if tui_ok else "TOKEN_VAZIO",
            "https_adapter_classification": "HTTPS_TRANSPORT_ADAPTER" if transport_ok else "TOKEN_VAZIO",
            "web_browser_tls": "TOKEN_VAZIO" if not web_ok else "RUNTIME_PENDING",
            "asm_web_browser_tls": "TOKEN_VAZIO" if not asm_ok else "RUNTIME_PENDING",
            "certified_tls": "TOKEN_VAZIO",
        },
        "claim_allowed": False,
        "next_actions": [
            "executar transferências controladas TLS 1.2 e TLS 1.3 e preservar relatórios",
            "implementar ou localizar parser HTTP, renderização, histórico e testes de integração",
            "para claim ASM, ligar fonte assembly ao transporte/TLS e executar KATs e transcripts",
            "submeter qualquer claim de certificação a escopo, autoridade e evidência nomeados",
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Audita claims de navegador, TLS e implementação ASM")
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--config", type=Path, default=Path("configs/browser-tls-capability.v1.json"))
    parser.add_argument("--write", type=Path)
    parser.add_argument(
        "--require",
        choices=["TUI_FILE_BROWSER", "HTTPS_TRANSPORT_ADAPTER", "WEB_BROWSER_TLS", "ASM_WEB_BROWSER_TLS"],
    )
    args = parser.parse_args(argv)
    root = args.root.resolve()
    config = args.config if args.config.is_absolute() else root / args.config
    report = audit(root, config)
    payload = json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.write:
        target = args.write if args.write.is_absolute() else root / args.write
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)

    if args.require:
        state = report["levels"][args.require]["state"]
        runtime = report["levels"][args.require]["runtime_state"]
        if state not in {"STATIC_EVIDENCE", "PASS"}:
            return 1
        if args.require in {"WEB_BROWSER_TLS", "ASM_WEB_BROWSER_TLS"} and runtime != "PASS":
            return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
