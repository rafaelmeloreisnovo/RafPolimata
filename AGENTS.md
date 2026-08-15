# AGENTS.md — RafPolimata agent router

This file is the short, repository-wide entry point for coding agents.
It is intentionally smaller than the detailed protocol.

## Read order

Before changing code, documentation, configuration, tests, evidence, or generated artifacts:

1. Read this file.
2. Read `docs/AGENTES.md` for the detailed operational protocol.
3. Read `README.md` and `docs/INDEX.md` for repository orientation.
4. Read the subsystem documentation and tests for the files you will touch.
5. Check for a nearer `AGENTS.md`; when present, it scopes the subtree below it.

Tool adapters are additive only:

- GitHub Copilot: `.github/copilot-instructions.md` and matching `.github/instructions/*.instructions.md`.
- Claude Code: `CLAUDE.md`.
- OpenAI Codex: this `AGENTS.md` plus any nearer scoped `AGENTS.md`.
- ChatGPT working through GitHub/repository context: this file and `docs/AGENTES.md` are the repository contract; product-level behavior is not defined by repository files.

If an adapter conflicts with this file or `docs/AGENTES.md`, do not guess. Preserve the stricter evidence/safety rule and record the conflict.

## Repository purpose

RafPolimata combines low-level compiler/toolchain work, Android APK/DEX/ELF generation, deterministic indexing, evidence/governance tooling, and research experiments.

The stable engineering invariant is:

```text
concept != implementation != execution != evidence != validated claim
```

A file, comment, checkbox, workflow YAML, or historical receipt is not proof of the current commit by itself.

## Truth model

Use these states without inflating them:

- `REFERENCE`: specification or explanatory material.
- `IMPLEMENTED`: code exists.
- `PASS`: a named gate was executed and passed in the declared scope.
- `FAIL`: a named gate was executed and failed.
- `TOKEN_VAZIO`: evidence is absent, insufficient, stale, unavailable, or outside the executed scope.

When evidence is incomplete, preserve `TOKEN_VAZIO`. Do not turn absence into success or failure.

For runtime/security/scientific claims, prefer evidence tied to the same commit, artifact and environment. Historical evidence remains historical.

## Current high-risk corrections

### ApkC freestanding scope

Do not use the obsolete rule "no libc anywhere in ApkC".

Correct rule:

- ARM/freestanding routes must satisfy their no-libc/no-heap contract.
- Hosted development routes may deliberately use libc where the implementation declares that boundary (for example host x86/x86_64 development support).
- Never weaken a freestanding target because a hosted target exists.

### T^7 / 42 semantics

Do not claim that the implementation proves "42 fixed-point attractors".

The current falsifier/closure treats `42` as a bounded/indexing construction in the implemented path; the previous strong fixed-point convergence claim was falsified as stated. New claims require a new definition, falsifier, execution and evidence.

Read `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` before changing T^7 convergence language.

### Android current-commit evidence

Do not promote source/ELF/APK existence into device-runtime proof.

The strong chain is:

```text
source
-> current ARM32/ARM64 artifact
-> current APK packaging
-> embedded-artifact identity
-> current signature verification
-> install
-> launch/dlopen
-> ANativeActivity/runtime observation
-> log/exit receipt
```

A missing link remains `TOKEN_VAZIO`/blocked for the corresponding claim.

## Working rules

- Work on a non-protected branch. Do not commit directly to `main` unless the human explicitly requests it.
- Do not merge a PR without explicit human authorization.
- Keep changes scoped; unrelated cleanup belongs in another PR.
- Never erase contradictory or negative evidence to make the project look complete.
- Never edit generated outputs in `docs/generated/` or `results/document-governance/` by hand; change the source/policy/generator and regenerate.
- Never expose detected secrets. Record only the detector/result needed for audit.
- Preserve rollback and provenance when moving, replacing, or deleting files.
- Do not silently change binary layouts, persisted schemas, IDs, hashes, units, timestamp semantics, or record ordering.
- Do not use `|| true` or equivalent to hide a blocking gate failure.

## Before editing

Record or inspect:

```sh
git branch --show-current
git rev-parse HEAD
git status --short
```

Identify:

- task class: code | docs | data | compliance | research;
- canonical source(s);
- affected invariants;
- applicable tests/gates;
- unavailable tools/device/data as `TOKEN_VAZIO`.

Run the smallest relevant baseline before modification when execution is available.

## Subsystem minimums

### ApkC / compiler

Read `Apkc/PROTOCOL.md`, `docs/APKC_PROTOCOL.md`, and the tests/scripts named by the changed path.

For freestanding AArch64 syntax, use the repository target rather than inventing a new command:

```sh
make syntax
```

For compiler contract/self-test when applicable:

```sh
make compiler-contract
make compiler-selftest
```

### Conversation indexer

Read the path-specific Copilot instruction file when present and the canonical protocol referenced there. Preserve bounded streaming, source byte ranges, deterministic output, explicit-width serialization and corruption rejection.

### T^7 / Verbovivo

Read the current closure and falsifier before editing explanatory text or mathematical claims. A numerical index/range is not automatically a dynamical attractor theorem.

### Documentation/governance

New or changed canonical documentation must preserve navigation, lifecycle and evidence semantics. Run the document-governance checks when available:

```sh
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --check --print-summary
```

Do not commit regenerated governance outputs unless the task requires them and their provenance is clear.

## Evidence discipline

A valid technical receipt should identify, when applicable:

```text
repository/commit
input identity
command
compiler/tool versions
environment/architecture
exit code
stdout/stderr or their hashes
output identity
scope and limitations
```

`PASS` is scoped to the gate that actually ran. It does not imply production readiness, security, scientific validity, or independent replication.

## PR/handoff contract

PR descriptions should state:

- why the change exists;
- files/subsystems affected;
- invariants preserved or changed;
- commands actually executed;
- observed PASS/FAIL/TOKEN_VAZIO;
- risks and rollback;
- current-commit evidence boundaries.

Keep a PR draft while a material gate required for its stated claim is open.

## Session close

End work with:

```text
F_ok   = what was actually changed/executed/demonstrated
F_gap  = what remains unknown, blocked, contradicted or unexecuted
F_next = the smallest reproducible next action
```

Never invent merit to make `F_ok` look larger.
