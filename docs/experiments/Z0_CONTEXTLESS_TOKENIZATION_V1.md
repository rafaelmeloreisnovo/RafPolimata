# Z0 Contextless Tokenization V1

**State:** IMPLEMENTED / CI_PENDING  
**Authority:** `rafaelmeloreisnovo/RafPolimata`  
**Claim boundary:** structural byte observation only; `claim_allowed=false` for cognition, model behavior, physical-device runtime or scientific generalization.

## Question

What remains when tokenization is reduced to the smallest observable operation and we deliberately remove context, attention, learned weights, vocabulary, normalization, embeddings, synthetic BOS/EOS tokens and history?

This experiment does **not** attempt to build an intelligent model without weights. It isolates the lower boundary before model semantics: caller-provided bytes plus deterministic presence/absence classification.

## Z0 contract

For an input view `V=(p,n)`:

- `p=NULL, n=0` -> `ABSENT`;
- `p!=NULL, n=0` -> `EMPTY`;
- `p=NULL, n>0` -> `INVALID`;
- `p!=NULL, n>0` -> `PRESENT`.

When present, token `i` is exactly `(offset=i, byte=p[i], kind=BYTE)`. No token is generated outside `0 <= i < n`.

The implementation declares exact feature counts of zero for:

`context_left | context_right | attention_edges | learned_weights | vocabulary_entries | normalization_passes | synthetic_tokens | history_bytes`.

These zeros are design facts for this module. They are **not** `TOKEN_VAZIO`. Missing runtime/device/scientific evidence remains `TOKEN_VAZIO` under the normal RAFAELIA evidence boundary.

## Critical falsifiers

The gate requires all of these distinctions to survive:

```text
ABSENT  != EMPTY
EMPTY   != one NUL byte (0x00)
EMPTY   != one SPACE byte (0x20)
NUL     != SPACE
byte 0  != TOKEN_VAZIO
```

An empty sequence emits no synthetic token. A NUL byte is data and emits one byte token. A space is data and emits one byte token.

## Hypotheses

**H1:** after removing contextual/model machinery, deterministic identity-preserving byte observation remains possible with zero external runtime symbols in the pure object.

**H0 falsifier:** any hidden vocabulary/state/history, synthetic token, hosted dependency, unresolved helper, or collapse of `ABSENT/EMPTY/NUL/SPACE` invalidates the Z0 contract.

## Gate

`freestanding/tests/verify_z0_token.sh` performs:

1. no-hosted-header freestanding object compilation;
2. unresolved-symbol rejection;
3. executable semantic smoke for the state distinctions above;
4. OS-neutral object compilation for x86_64, i686, ARMv7, AArch64, RV32 and RV64.

Build PASS proves only this bounded source/object/semantic contract. It does not prove bare-metal or physical-device execution.

## Interpretation

At Z0, the core has no mechanism that can create linguistic meaning. If a later layer produces interpretation, prediction or a natural-language answer, that information originates outside this Z0 tokenizer and must be attributed to that later layer.

So the useful boundary is not “nothing becomes everything.” It is stricter:

```text
no model semantics
+ explicit input presence
+ byte identity
+ explicit rules
= reconstructible observation
```

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`; `TOKEN_VAZIO != 0`.
