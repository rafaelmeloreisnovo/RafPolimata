# GitHub Copilot Instructions — RafPolimata

These are repository-wide instructions. They are intentionally short.
Detailed rules live in `AGENTS.md` and `docs/AGENTES.md`; path-specific rules live in `.github/instructions/*.instructions.md`.

## Required read order

Before editing:

1. `AGENTS.md`
2. `docs/AGENTES.md`
3. `README.md` and the subsystem docs/tests for the path being changed
4. any matching `.github/instructions/*.instructions.md`

If an older document conflicts with current code, current receipts, or a current closure, do not silently choose the older claim. Record the conflict and preserve the stricter evidence boundary.

## Repository-wide invariants

```text
concept != implementation != execution != evidence != validated claim
```

- Never turn file existence, a checkbox, workflow YAML, or a merged PR into `PASS` without the applicable executed evidence.
- Use `TOKEN_VAZIO` for missing, stale, unavailable, or insufficient evidence.
- Historical receipts remain historical unless explicitly reproduced for the current commit/artifact.
- Do not invent test results, tool versions, device observations, hashes, benchmark values, citations, or runtime outcomes.
- Work on a non-protected branch; do not merge without explicit human authorization.
- Do not edit generated outputs in `docs/generated/` or `results/document-governance/` by hand.
- Do not expose secrets; report only the minimum detector/result needed for audit.
- Keep unrelated cleanup out of the current PR.

## ApkC scope

Do not use the obsolete statement "no libc anywhere in ApkC".

- ARM/freestanding routes must preserve their declared no-libc/no-heap contract.
- Hosted development routes may use libc when that boundary is explicitly implemented and documented.
- A hosted path never weakens the freestanding target.

Use repository targets/scripts rather than copying stale compiler commands. For the current hardened syntax path:

```sh
make syntax
```

## T^7 / 42

Do not state or reintroduce "42 fixed-point attractors" as a proven property.
Read `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` and the current falsifier before changing T^7 convergence language.
`42` may be an indexing/range/construction parameter without being a theorem about 42 dynamical attractors.

## Android evidence

Do not collapse these states:

```text
source != ELF != APK packaged != signed APK != installed APK != runtime proven
```

Claims about device execution require the device/current-artifact receipt appropriate to the claim.

## Testing and reporting

Run the smallest relevant baseline and post-change gate when the environment supports it. If a required tool/device is unavailable, record `TOKEN_VAZIO` rather than simulating a pass.

A PR/handoff should state:

- scope and why;
- files/subsystems changed;
- commands actually executed;
- observed `PASS` / `FAIL` / `TOKEN_VAZIO`;
- risks and rollback;
- any semantic/layout/identity change.

End with:

```text
F_ok
F_gap
F_next
```

Do not enlarge `F_ok` with unexecuted work.
