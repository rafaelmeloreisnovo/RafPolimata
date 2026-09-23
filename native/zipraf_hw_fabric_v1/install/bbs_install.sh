#!/bin/sh
# Governance binding: CLOSURE_L11 — unknown-state markers remain gaps.
# RAFCODE-IP-NOTICE
# Copyright (c) 2026 Rafael Melo Reis.
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
# Berne Convention orientation: provenance notice only; applicable law controls.
# Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.

set -eu

ESC='\033['
RESET="${ESC}0m"
CYAN="${ESC}36m"
MAG="${ESC}35m"
GREEN="${ESC}32m"
YELLOW="${ESC}33m"
RED="${ESC}31m"

printf '%b' "$CYAN"
printf '%s\n' '╔══════════════════════════════════════════════════════╗'
printf '%s\n' '║          ZIPRAF Ω HARDWARE FABRIC // BBS           ║'
printf '%s\n' '║         ASM · C · Rust · capability gate           ║'
printf '%s\n' '╚══════════════════════════════════════════════════════╝'
printf '%b' "$RESET"
printf '%b\n' "${MAG}[RAFCODE-Φ]${RESET} source ≠ execution ≠ evidence ≠ claim"

have() { command -v "$1" >/dev/null 2>&1; }
probe() {
  if have "$1"; then printf '%b\n' "${GREEN}[OK]${RESET} $1: $(command -v "$1")";
  else printf '%b\n' "${YELLOW}[TOKEN_VAZIO]${RESET} $1 unavailable"; fi
}

probe cc
probe clang
probe rustc
probe make
mkdir -p build

if have make && have cc; then
  printf '%b\n' "${CYAN}[GATE]${RESET} C selftest"
  if make selftest; then printf '%b\n' "${GREEN}[PASS]${RESET} C routing/permission selftest";
  else printf '%b\n' "${RED}[FAIL]${RESET} C routing/permission selftest"; exit 1; fi
fi

if have make && have rustc; then
  printf '%b\n' "${CYAN}[GATE]${RESET} Rust no_std syntax"
  if make rust-check; then printf '%b\n' "${GREEN}[PASS]${RESET} Rust no_std compile";
  else printf '%b\n' "${RED}[FAIL]${RESET} Rust no_std compile"; exit 1; fi
fi

if have make && have clang; then
  printf '%b\n' "${CYAN}[GATE]${RESET} ASM cross-syntax"
  if make asm-check; then printf '%b\n' "${GREEN}[PASS]${RESET} ARMv7/AArch64/x86_64 ASM syntax";
  else printf '%b\n' "${RED}[FAIL]${RESET} ASM syntax/target gate"; exit 1; fi
fi

printf '%b\n' "${CYAN}[DONE]${RESET} No package manager or privileged operation was performed."
