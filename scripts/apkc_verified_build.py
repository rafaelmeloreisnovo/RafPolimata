#!/usr/bin/env python3
"""Run ApkC into a private temporary path and promote only a valid APK.

This wrapper closes the publication boundary even while internal ApkC source gaps
remain open: a failed, stale, truncated, structurally invalid or ABI-incomplete APK
is never moved to the requested final path.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from validate_apkc_formats import validate_apk

SCHEMA = "raf.apkc-verified-build.v1"
MAX_APK_BYTES = 256 * 1024 * 1024


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for block in iter(lambda: fh.read(131072), b""):
            h.update(block)
    return h.hexdigest()


def safe_command(command: list[str]) -> list[str]:
    redacted: list[str] = []
    sensitive_next = False
    for item in command:
        if sensitive_next:
            redacted.append("<redacted>")
            sensitive_next = False
            continue
        lower = item.casefold()
        if lower in {"--password", "--token", "--secret", "--keystore-pass", "--key-pass"}:
            redacted.append(item)
            sensitive_next = True
        elif any(word in lower for word in ("password=", "token=", "secret=")):
            redacted.append("<redacted-argument>")
        else:
            redacted.append(item)
    return redacted


def write_report(path: Path, report: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Executa ApkC e promove somente APK estruturalmente válido")
    parser.add_argument("--apkc", type=Path, required=True, help="binário ApkC executável")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--log", type=Path)
    parser.add_argument("--require-both", action="store_true", help="exige arm64-v8a e armeabi-v7a")
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("apkc_args", nargs=argparse.REMAINDER, help="argumentos adicionais após --")
    args = parser.parse_args(argv)

    root = Path.cwd().resolve()
    apkc = args.apkc.expanduser().resolve()
    source = args.input.expanduser().resolve()
    output = args.output.expanduser().resolve()
    report_path = (args.report or Path(str(output) + ".evidence.json")).expanduser().resolve()
    log_path = (args.log or Path(str(output) + ".build.log")).expanduser().resolve()

    report: dict[str, Any] = {
        "schema": SCHEMA,
        "state": "FAIL",
        "claim_allowed": False,
        "input": str(source),
        "output": str(output),
        "require_both": args.require_both,
        "errors": [],
        "build": {},
        "validation": {},
    }

    if args.timeout < 1 or args.timeout > 3600:
        parser.error("--timeout deve estar entre 1 e 3600 segundos")
    if not apkc.is_file() or not os.access(apkc, os.X_OK):
        report["state"] = "TOKEN_VAZIO"
        report["errors"].append("binário ApkC ausente ou não executável")
        write_report(report_path, report)
        return 2
    if not source.is_file():
        report["state"] = "TOKEN_VAZIO"
        report["errors"].append("arquivo de entrada ausente")
        write_report(report_path, report)
        return 2
    if source == output:
        report["errors"].append("entrada e saída não podem ser o mesmo arquivo")
        write_report(report_path, report)
        return 64

    extra = list(args.apkc_args)
    if extra and extra[0] == "--":
        extra = extra[1:]
    forbidden = {"-o", "--output"}
    if any(item in forbidden for item in extra):
        report["errors"].append("não passe -o/--output em apkc_args; o wrapper controla a saída")
        write_report(report_path, report)
        return 64

    output.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.parent.mkdir(parents=True, exist_ok=True)

    tmp_parent = Path(os.environ.get("TMPDIR", str(output.parent))).expanduser()
    if not tmp_parent.is_dir() or not os.access(tmp_parent, os.W_OK):
        tmp_parent = output.parent

    with tempfile.TemporaryDirectory(prefix="apkc-verified-", dir=tmp_parent) as tmp_name:
        tmp_dir = Path(tmp_name)
        candidate = tmp_dir / "candidate.apk"
        command = [str(apkc), *extra, "-o", str(candidate), str(source)]
        report["build"] = {
            "command": safe_command(command),
            "apkc_sha256": sha256_file(apkc),
            "input_sha256": sha256_file(source),
            "timeout_seconds": args.timeout,
        }

        try:
            proc = subprocess.run(
                command,
                cwd=root,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                check=False,
                timeout=args.timeout,
                env=os.environ.copy(),
            )
            raw_log = proc.stdout or b""
            log_path.write_bytes(raw_log)
            report["build"].update({
                "exit_code": proc.returncode,
                "log_sha256": hashlib.sha256(raw_log).hexdigest(),
                "log_size_bytes": len(raw_log),
            })
        except subprocess.TimeoutExpired as exc:
            raw_log = (exc.stdout or b"") + (exc.stderr or b"")
            log_path.write_bytes(raw_log)
            report["errors"].append("ApkC excedeu o timeout e foi encerrado")
            report["build"]["timeout"] = True
            write_report(report_path, report)
            return 1
        except OSError as exc:
            report["errors"].append(f"falha ao executar ApkC: {type(exc).__name__}: {exc}")
            write_report(report_path, report)
            return 1

        if proc.returncode != 0:
            report["errors"].append(f"ApkC terminou com exit {proc.returncode}")
            write_report(report_path, report)
            return 1
        if not candidate.is_file():
            report["errors"].append("ApkC retornou zero sem produzir o APK candidato")
            write_report(report_path, report)
            return 1

        candidate_size = candidate.stat().st_size
        report["build"]["candidate_size_bytes"] = candidate_size
        report["build"]["candidate_sha256"] = sha256_file(candidate)
        if candidate_size == 0 or candidate_size > MAX_APK_BYTES:
            report["errors"].append(f"tamanho do APK candidato fora do contrato: {candidate_size}")
            write_report(report_path, report)
            return 1

        validation = validate_apk(candidate, require_both=args.require_both)
        report["validation"] = validation
        if validation.get("state") != "PASS":
            report["errors"].append("APK candidato recusado pelo validador DEX/ELF/ABI")
            write_report(report_path, report)
            return 1

        # Atomic promotion inside the final directory. Copy into a sibling temp
        # first because TMPDIR and output may live on different filesystems.
        sibling = output.with_name(output.name + f".tmp.{os.getpid()}")
        try:
            shutil.copyfile(candidate, sibling)
            with sibling.open("rb") as fh:
                os.fsync(fh.fileno())
            os.replace(sibling, output)
        finally:
            try:
                sibling.unlink()
            except FileNotFoundError:
                pass

    report["state"] = "PASS"
    report["claim_allowed"] = True
    report["output_sha256"] = sha256_file(output)
    report["output_size_bytes"] = output.stat().st_size
    report["claim_scope"] = "O APK específico passou validação estrutural DEX/ELF/ABI; instalação e runtime em aparelho permanecem fora deste gate."
    write_report(report_path, report)
    print(f"apkc_verified_build: PASS output={output} report={report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
