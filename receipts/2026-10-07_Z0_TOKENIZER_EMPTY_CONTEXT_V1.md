# Z0 Tokenizer Empty Context Receipt V1

Repository: `rafaelmeloreisnovo/RafPolimata`
Observed base: `main@32220b1ce8353566835fdee2301657ca79366da2`
Task class: code + evidence
Governance: `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`; absence remains `TOKEN_VAZIO`.
Gap ownership: `CLOSURE_L11` (operational gap topology governance only; the actual device/provenance gaps remain open and never become PASS through this reference).

## Intent

Create a smallest deterministic z0 observer for byte inputs such as empty input, blank input and `____`, without model context, attention, learned weights, heap, libc calls, network, package manager or external tables.

## Materialized Files

| Path | Role |
|---|---|
| `tools/raf_z0_tokenizer.h` | Header-only C byte observer. |
| `tests/test_raf_z0_tokenizer.c` | Regression test for z0 states. |
| `receipts/2026-10-07_Z0_TOKENIZER_EMPTY_CONTEXT_V1.md` | This bounded receipt. |

## Classification Boundary

| Input condition | State |
|---|---|
| `length == 0` | `RAF_Z0_ABSOLUTE_EMPTY` |
| `bytes == 0 && length > 0` | `RAF_Z0_MISSING_SOURCE` |
| only space, tab, newline or carriage return bytes | `RAF_Z0_BLANK_FIELD` |
| only `_` bytes | `RAF_Z0_UNDERSCORE_FIELD` |
| only nonblank, non-underscore bytes | `RAF_Z0_VISIBLE_FIELD` |
| mixed classes | `RAF_Z0_MIXED_FIELD` |

## Local Commands

```sh
gcc -std=c11 -Wall -Wextra -pedantic tests/test_raf_z0_tokenizer.c -o /tmp/raf_z0_tokenizer_test
/tmp/raf_z0_tokenizer_test
gcc -std=c11 -ffreestanding -fno-builtin -nostdlib -Wall -Wextra -pedantic -c tests/test_raf_z0_tokenizer.c -o /tmp/raf_z0_tokenizer_test.o
```

Observed locally in the Codex workspace on 2026-10-07:

| Gate | Exit |
|---|---:|
| hosted compile | 0 |
| hosted execution | 0 |
| freestanding object compile | 0 |

## Source Hashes

| Path | SHA-256 |
|---|---|
| `tools/raf_z0_tokenizer.h` | `4bffcc7c0e6d45bc711dce93004221bc5185dc41f4b372de9e3a574d415d3b0c` |
| `tests/test_raf_z0_tokenizer.c` | `1b3aa1454f2ba23920839dd40a8e7130dd352602ac3bfecb7fc54221e2e0cc60` |

## Expected Evidence Boundary

Passing the hosted executable demonstrates the declared test vectors only. Passing the freestanding object compile demonstrates source-level independence from libc calls and external dependencies in this slice; it is not physical-device proof, not tokenizer parity with any LLM tokenizer, and not a scientific claim.

R3 = <F_ok: z0 byte observer + regression vectors + freestanding object gate specified, F_gap: no CI/device/provider-independent execution yet, F_next: run current-commit CI gates on the PR and keep claim scope bounded>

---

## Append-only hotfix observation — 2026-10-07

μID: `RFP416-Z0-HEADER-WERROR-01`  
Task: surgical C11 source + test correction; no API enum/struct layout change.  
Original PR head: `fd8486ee86987baf09873afb8c06754dc7e270fd`  
Source/test hotfix branch head before this receipt: `6d82d7a04449676282b758bb5d986132fedccdd8`  
Owner: RafPolimata PR #416 / `tools/`, `tests/`, `receipts/`.  
Route: source → host compile → host execution → freestanding object → source hashes → provider CI (pending).

### Baseline failure retained

The original header used `static` definitions. Including it to use only the public enum or struct produced `-Werror=unused-function` for `raf_z0_observe_bytes` under GCC C11 `-Wall -Wextra -Werror -pedantic` (local baseline `FAIL`). The original hosted regression executable and freestanding object compilation both passed. This is consumer integration friction, not an observed failure in the original test.

### Minimal hotfix

- Change the two header-only helper definitions to `static inline`; do not change the public enum values or observation struct layout.
- Document that `VISIBLE_FIELD` means every other byte, including NUL; it **does not** mean printable Unicode/ASCII.
- Extend the original test with non-null empty input, missing source with nonzero declared length, blank+underscore combination, embedded NUL, and exhaustive ordered 2-byte inputs (`256 × 256 = 65,536`).
- Add `tests/test_raf_z0_include_only.c` to prevent the original `-Werror=unused-function` regression.

### Independent local gates (this session)

Environment: Linux x86_64; GCC `14.2.0` (Debian `14.2.0-19`); C11. Execution is **hosted only**. From repository root:

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic tests/test_raf_z0_tokenizer.c -o /tmp/raf_z0_test
/tmp/raf_z0_test
gcc -std=c11 -O2 -Wall -Wextra -Werror -pedantic tests/test_raf_z0_tokenizer.c -o /tmp/raf_z0_opt_test
/tmp/raf_z0_opt_test
gcc -std=c11 -Wall -Wextra -Werror -pedantic tests/test_raf_z0_include_only.c -o /tmp/raf_z0_include
/tmp/raf_z0_include
gcc -std=c11 -ffreestanding -fno-builtin -nostdlib -Wall -Wextra -Werror -pedantic -c tests/test_raf_z0_tokenizer.c -o /tmp/raf_z0_test.o
gcc -std=c11 -ffreestanding -fno-builtin -nostdlib -Wall -Wextra -Werror -pedantic -c tests/test_raf_z0_include_only.c -o /tmp/raf_z0_include.o
nm -u /tmp/raf_z0_test.o
nm -u /tmp/raf_z0_include.o
```

Observed: both hosted executables exit 0, including the embedded 65,536 ordered byte-pair loop; both freestanding object builds exit 0; both `nm -u` outputs are empty. This is *not* a bare-metal binary link or a device execution.

### Current source SHA-256 (supersedes original two file hashes above)

| Path | SHA-256 |
|---|---|
| `tools/raf_z0_tokenizer.h` | `b721bcc97ec7d547726b55d7bee2d5454d7eded61188d6f6a66bd6cbe057f4f5` |
| `tests/test_raf_z0_tokenizer.c` | `c3bd1fd61ab939e21b1be16c6065ee4eb6225634ec968974b0fe4255fa2db24e` |
| `tests/test_raf_z0_include_only.c` | `69976182b9319e813118716625e5fa19c44662c9a563295f3cd3a7a0b3991f30` |

The earlier two hashes and baseline observations in this receipt are **historical original-PR observations**, not current source identities.

### Gates, risks, rollback

- `PASS`: bounded hosted executions, freestanding object compile, warning-clean include-only consumer, no undefined object symbols.
- `TOKEN_VAZIO`: new PR HEAD GitHub Actions result, ARMv7/AArch64 object build, static freestanding ELF link, Android runtime, independent provider reproduction, full-repository regression and explicit source-path license clearance. Root `LICENSE` and `LICENSE.md` were not found in a targeted default-branch lookup; this is **not** proof of absent rights or of licensing terms.
- `claim_allowed=false` for model-tokenizer equivalence, platform parity, production readiness and scientific/general security claims.
- Risk: header behavior preserved; `static inline` may affect code generation; local tests do not cover all targets. No package/network/runtime dependency added.
- Rollback: revert the PR-branch source/test hotfix commits (header, test extension, include-only test) or close the still-open PR without merge. Preserve this negative baseline as evidence.

`R3 = <F_ok: source correction + explicit regression test + local evidence, F_gap: new-HEAD CI/device/licensing/provenance unresolved, F_next: observe new exact-head CI and evaluate a scoped merge only after required gates and human authorization>`
