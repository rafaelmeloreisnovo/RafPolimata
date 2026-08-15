# CLAUDE.md — Claude Code adapter for RafPolimata

@AGENTS.md
@docs/AGENTES.md

This file is a Claude Code adapter, not a second source of architectural truth.
The repository-wide contract is `AGENTS.md`; the detailed protocol is `docs/AGENTES.md`.

## Session start

Before editing:

1. Read the imported agent protocol above.
2. Read `README.md`, `docs/INDEX.md`, and the subsystem docs/tests for the touched path.
3. Inspect branch, HEAD and working tree.
4. Identify which statements are implementation facts, executed evidence, historical evidence, or `TOKEN_VAZIO`.

Do not merge without explicit human authorization.

## Project orientation

RafPolimata contains several integrated but independently gated bodies, including:

- `raf_compile.h` — high-level compiler/orchestration path;
- `Apkc/` — Android/compiler/toolchain work with both freestanding targets and explicitly hosted development paths;
- `rafaelia/` — Verbovivo/T^7 research/runtime code;
- `runtime/conversation_indexer/` — bounded deterministic indexing/serialization;
- `scripts/`, `contracts/`, `tests/`, `docs/` — validation, governance, evidence and documentation.

Do not reduce the whole repository to one subsystem.

## Critical truth corrections

### ApkC

Do **not** enforce the obsolete statement "no libc anywhere in ApkC".

Correct interpretation:

```text
ARM/freestanding path -> preserve its no-libc/no-heap contract
hosted development path -> libc may be used where explicitly declared
```

Hosted support must not leak into or weaken a freestanding target.

Use current repository targets such as `make syntax` rather than copying historical syntax commands blindly.

### T^7 / 42

Do **not** describe T^7 as having "42 fixed-point attractors" as a proven result.

Read `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` before editing convergence language. The strong historical fixed-point convergence claim was falsified as stated. `42` can still be a valid range/index/construction parameter where the implementation defines it, without implying a theorem about 42 dynamical attractors.

### Current-commit evidence

Do not inherit historical results into a new artifact silently.

```text
source != compiled artifact != packaged APK != signed APK != installed APK != runtime evidence
```

The same separation applies to scientific experiments: synthetic/local evidence does not imply external or independent validation.

## Coding discipline

- Preserve explicit bounds and capacity checks.
- Do not add hidden allocation to a freestanding core.
- Guard NULL/error lookup paths.
- Do not alter persisted binary layouts, schema versions, IDs, hashes, units, timestamps or record ordering silently.
- Correct portable/reference behavior comes before optimization.
- SIMD/ASM changes need an applicable equivalence/golden test.
- Do not suppress blocking failures with `|| true`, unconditional success, or weakened assertions.
- Do not edit generated governance outputs manually.
- Keep negative evidence and falsifiers intact.

## Documentation discipline

When editing prose:

- distinguish `REFERENCE`, `IMPLEMENTED`, `PASS`, `FAIL`, and `TOKEN_VAZIO`;
- bind a `PASS` statement to the gate/commit/environment that actually produced it;
- label heuristic/symbolic language separately from executable/scientific claims;
- update stale onboarding text when code/closures have superseded it;
- prefer one canonical statement plus links over duplicated instructions.

## Useful entrypoints

```sh
make syntax
make compiler-contract
make compiler-selftest
make verbovivo-demo
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --check --print-summary
```

Run only commands applicable to the task and available in the environment. If a tool/device is unavailable, record `TOKEN_VAZIO`; do not fabricate execution.

## Handoff

Finish with:

```text
F_ok   = changed/executed/demonstrated
F_gap  = unresolved/unexecuted/contradictory
F_next = smallest reproducible next action
```

A Claude Code session may propose a branch/PR and evidence package. Human review remains the merge authority.
