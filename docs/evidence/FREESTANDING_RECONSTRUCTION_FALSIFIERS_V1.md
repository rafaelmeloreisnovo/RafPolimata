# RAFAELIA Freestanding Reconstruction Falsifiers V1

Status: `DOCUMENTED / NOT_RUN / FAIL_CLOSED`  
Baseline: `rafaelmeloreisnovo/RafPolimata@be3e1e3cdf11a6f99104f94915b7b47562090a61`  
Created: `2026-10-11T15:40:00-03:00`  
claim_allowed: `false`  
Closure: `CLOSURE_L13`

This file defines the adversarial tests required before promoting the freestanding reconstruction path. It is intentionally stricter than a happy-path checklist: a single triggered falsifier must either invalidate the claim or hold it at `TOKEN_VAZIO`.

## Hypothesis model

H1: the named L0/authorial slice is reconstructible as a freestanding, zero-hosted-dependency source/object path under its declared architecture/toolchain boundary.

H0: at least one required boundary is missing, non-reproducible, hosted/runtime-dependent, license-unclear, artifact-mismatched, or over-claimed.

Decision rule: accept only the smallest H1 subclaim whose exact gate passed with source/ref/command/env/hash/log attached. Otherwise keep H0 compatible and mark `TOKEN_VAZIO` or `INVALIDATED`.

## Fatal falsifier matrix

| ID | Claim attacked | Probe | Invalidates if | Required receipt |
| --- | --- | --- | --- | --- |
| F01-HOSTED-HEADER | L0 has no hosted dependency | `make authorial-freestanding` | `#include <...>` or hosted/system header appears in claimed L0 | command, ref, log |
| F02-ALLOC-SYSCALL | L0 has no heap/syscall | `freestanding/tests/verify_contract.sh` | malloc/calloc/realloc/free or syscall instruction appears in claimed L0 | command, ref, log |
| F03-SYNTHETIC-TAIL | no hidden scalar tail/shell in canonical L0 | `freestanding/tests/verify_contract.sh` | do/while macro shell, hidden tail, fallback annotation or weak production fallback appears | command, ref, log |
| F04-FILE-CONTRACT | every L0 header carries explicit file contract | `freestanding/tests/verify_contract.sh` | required file-contract marker is missing | path list + log |
| F05-ZERO-DEP-OBJECT | claimed authorial slice has no unresolved helper | `make authorial-zero-dependency` | `nm -u` returns unresolved symbols for claimed targets | object hashes + nm log |
| F06-LANGUAGE-STRICT | strict language route is bounded | `make language-contract` | strict files are missing, hosted runtime appears, matrix invalid or selftests fail | command, log, report |
| F07-ARTIFACT-ABI | artifact corresponds to source/ref | `make strict-elf ELF=<artifact>` | ABI/profile mismatch, undefined symbols, unsupported profile or hash mismatch | artifact hash + ABI receipt |
| F08-LICENSE | authorship/license claim is bounded | license map probe | root or subtree license chain is absent while a license claim is made | license map + unresolved paths |
| F09-PROVIDER | CI/provider claim is exact-head | provider readback | run id/ref/status/log cannot be bound to the commit | provider run URL + status |
| F10-PHYSICAL | physical runtime claim is same-artifact | device/Termux/ARM run | device log uses different artifact/ref/env or no log exists | device, artifact hash, log |
| F11-INDEPENDENCE | independent reproduction exists | separate host/provider rerun | the rerunner cannot reconstruct source, command, input, artifact or expected observation | independent receipt |
| F12-CROSS-REPO | federated PASS is not inherited | RafGitTools custody bridge validation | producer receipt is treated as consumer PASS without validation | producer+consumer receipts |

## Minimal command tranche

Run only on an authorized checkout and preserve raw logs:

```sh
git rev-parse HEAD
make authorial-freestanding
make authorial-zero-dependency
make language-contract
```

Optional artifact gate, only after an artifact exists:

```sh
make strict-elf ELF=<artifact> ELF_PROFILE=exec
```

## Academic hardening requirements

A future paper, release, benchmark or architectural promotion must provide:

1. exact source ref and artifact hash;
2. exact command line and full environment metadata;
3. raw logs, not only summaries;
4. negative tests and malformed/adversarial cases;
5. independent reproduction or explicit `TOKEN_VAZIO_INDEPENDENT_REPRODUCTION`;
6. preservation of mismatches and failures;
7. no claim broader than the passed gate.

## Expected classifications

| Result | Classification |
| --- | --- |
| source files only written/read | `DOCUMENTED` |
| command ran locally once with logs | `MEASURED_LOCAL` for that environment only |
| same command rerun independently | `REPRODUCED` or `CROSS_HOST`, depending on host independence |
| falsifier triggers | `INVALIDATED` for the attacked claim |
| missing source/env/log/artifact/license | `TOKEN_VAZIO` |

## R3

F_ok:
- falsifiers are explicit and bound to existing repository gates where possible.

F_gap:
- no falsifier was executed by this documentation transaction;
- exact CI/provider, physical device and independent reproduction receipts remain open.

F_next:
- execute the minimal tranche on an authorized checkout and store raw logs/hashes as a receipt before any claim promotion.
