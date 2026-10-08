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
