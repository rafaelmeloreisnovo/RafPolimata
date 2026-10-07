# Z0 Contextless Tokenization V1

**State:** IMPLEMENTED / EXACT_HEAD_REVALIDATION_PENDING  
**Authority:** `rafaelmeloreisnovo/RafPolimata`  
**Lineage:** PR #415 is the consolidation target for the earlier PR #414 presence-state experiment; history is preserved rather than silently duplicated.  
**Claim boundary:** structural presence/byte observation only; `claim_allowed=false` for cognition, model behavior, physical-device runtime or scientific generalization.  
**Governance closures:** conceptual/implementation gaps bind to `CLOSURE_L11`; runtime/device/scientific evidence gaps bind to `CLOSURE_L12`.

## Question

What remains when tokenization is reduced to the smallest observable operation and we deliberately remove context, attention, learned weights, vocabulary, normalization, embeddings, synthetic BOS/EOS tokens and history?

This experiment does **not** attempt to build an intelligent model without weights. It isolates the lower boundary before model semantics: explicit input presence plus caller-provided bytes and deterministic rules.

## Z0 contract

For an input view `V=(provided,p,n)`:

- `provided=0` -> `ABSENT`;
- `provided=1, n=0` -> `EMPTY`, including `p=NULL`;
- `provided=1, n>0, p=NULL` -> `INVALID`;
- `provided=1, n=1, p[0]=0x00` -> `NUL`;
- `provided=1, n=1, p[0]=0x20` -> `SPACE`;
- `provided=1, n=1, other byte` -> `BYTE`;
- `provided=1, n>1` -> `SEQUENCE`.

The explicit `provided` bit prevents absence from being inferred from a pointer value. It therefore preserves the distinction between “no input was supplied” and “an intentionally supplied zero-length sequence.”

For every valid non-empty input, token `i` is exactly:

```text
(offset=i, byte=p[i], kind=NUL|SPACE|BYTE)
```

No token is generated outside `0 <= i < n`; an empty input emits no synthetic token.

The implementation declares exact feature counts of zero for:

`context_left | context_right | attention_edges | learned_weights | vocabulary_entries | normalization_passes | synthetic_tokens | history_bytes`.

These zeros are design facts for this module. They are **not** `TOKEN_VAZIO`. Missing runtime/device/scientific evidence remains `TOKEN_VAZIO` under `CLOSURE_L12`.

## Critical falsifiers

The gate requires all of these distinctions to survive:

```text
ABSENT  != EMPTY
EMPTY   != one NUL byte (0x00)
EMPTY   != one SPACE byte (0x20)
NUL     != SPACE
BYTE    != SEQUENCE
byte 0  != TOKEN_VAZIO
provided=0 != provided=1,size=0
```

A NUL byte is data and emits one token. A space is data and emits one token. A sequence emits one identity-preserving token per byte, with no vocabulary lookup.

## Information-loss projection ladder

The successor experiment does not create a special "nothing token". Instead, it asks when two already-defined Z0 input states become observationally indistinguishable after selected observables are removed.

The classifier-relevant observables are:

- `P` = explicit `provided` bit;
- `S` = size class `ZERO|ONE|MANY`;
- `D` = whether a data pointer is present;
- `U` = one-byte class `NUL|SPACE|BYTE` when safely observable.

`raf_fs_z0_equiv(a,b,mask)` defines equality relative to that explicit observation mask. It does not mutate either input.

For the seven canonical representatives `ABSENT|EMPTY|NUL|SPACE|BYTE|SEQUENCE|INVALID`, the gate requires:

| projection | observed dimensions | equivalence classes |
|---|---|---:|
| `Ω0` | `P+S+D+U` | 7 |
| `Ω1` | `P+S+D` | 5 |
| `Ω2` | `P+S` | 4 |
| `Ω3` | `S` | 3 |
| `Ω4` | none | 1 |

An orthogonal `P`-only projection must yield exactly 2 classes: `ABSENT` versus every supplied state.

The monotone destructive path is therefore:

```text
7 -> 5 -> 4 -> 3 -> 1
```

Concrete falsifiers:

- removing `U` collapses `NUL`, `SPACE` and ordinary one-byte data while `D` remains observable;
- removing `D` additionally collapses a supplied invalid one-byte/null-pointer view with supplied valid one-byte views at that projection;
- keeping only `S` collapses `ABSENT` with `EMPTY`, because both have size class `ZERO`;
- retaining only `P` preserves exactly "not supplied" versus "supplied";
- observing nothing makes all seven states one equivalence class.

This measures loss of distinguishability in a representation system. It does not claim that underlying states become physically identical, and `Ω4` is not a metaphysical assertion of absolute nothingness.

## Hypotheses

**H1:** after removing contextual/model machinery, deterministic identity-preserving presence and byte observation remain possible with zero external runtime symbols in the pure object.

**H0 falsifier:** any hidden vocabulary/state/history, synthetic token, hosted dependency, unresolved helper, collapse of the state distinctions above, or loss/reordering of a byte invalidates the Z0 contract.

## Gate

`freestanding/tests/verify_z0_token.sh` performs:

1. no-hosted-header freestanding object compilation;
2. unresolved-symbol rejection;
3. executable semantic smoke for the presence and byte distinctions above;
4. projection-collapse falsifiers requiring class counts `7 -> 5 -> 4 -> 3 -> 1` plus `presence-only=2`;
5. OS-neutral object compilation for x86_64, i686, ARMv7, AArch64, RV32 and RV64, including the projection-equivalence probe.

Build PASS proves only this bounded source/object/semantic contract. It does not prove bare-metal or physical-device execution.

## Relationship to PR #414

PR #414 established a useful whole-input presence classifier: `ABSENT/EMPTY/SPACE/NUL/BYTE/SEQUENCE/INVALID`. PR #415 retains that state vocabulary and the explicit `provided` signal, then adds byte-by-byte emission and explicit zero counts for the removed model mechanisms.

The intended topology is therefore predecessor -> consolidated successor, not two competing canonical Z0 implementations. Merge/promotion remains a separate human-authorized decision.

## Interpretation

At Z0, the core has no mechanism that can create linguistic meaning. If a later layer produces interpretation, prediction or a natural-language answer, that information originates outside this Z0 tokenizer and must be attributed to that later layer.

So the useful boundary is not “nothing becomes everything.” It is stricter:

```text
explicit presence
+ byte identity
+ order
+ deterministic rule
- model semantics
= reconstructible observation
```

This is an engineering boundary, not proof of “absolute nothingness” or an observer-free view of reality.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`; `TOKEN_VAZIO != 0`.
