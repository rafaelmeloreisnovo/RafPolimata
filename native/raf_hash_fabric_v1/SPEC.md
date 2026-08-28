# RAF Hash Fabric V1 — Normative Core

## 1. Purpose

Define a minimal, auditable control fabric around standard cryptographic primitives. The fabric owns routing, capability selection, bounded parallel scheduling, Merkle reduction, watchdog state and rollback semantics. It does not alter primitive mathematics.

## 2. Numeric control formats

```text
native arithmetic >= 64 bits -> Q32.32 control score
native arithmetic >= 32 bits -> Q16.16 control score
true 16-bit-only target       -> adapter required; TOKEN_VAZIO in V1
```

A Q-format is never inferred as a hash word width. Example: SHA-256 still operates on 32-bit words even if the control plane uses Q32.32.

## 3. Capability vector

```text
SCALAR      bit 0
NEON128     bit 1
AVX2_256    bit 2
AVX512_512  bit 3
ASM         bit 4
PAR16       bit 5
```

`lanes32(vector_bits) = vector_bits / 32`, yielding 4, 8 and 16 lanes for 128/256/512-bit vectors.

## 4. FSM

States:

```text
VOID PROBE BASELINE CANDIDATE VALIDATED DEGRADED FAILOVER ROLLBACK FAILBACK SAFE
```

Events:

```text
BOOT PROBE_OK PROBE_FAIL CANDIDATE_OK CANDIDATE_FAIL FAULT WATCHDOG_OK WATCHDOG_FAIL ROLLBACK_OK FAILBACK_OK
```

The implementation uses an explicit transition function/table. Unknown events preserve or move to SAFE according to the implementation's fail-closed rule.

## 5. Merkle batch

A V1 batch contains up to 16 independent 32-byte digests. A binary node is defined as:

```text
node = HASH(domain_node || left_digest || right_digest)
```

A leaf is:

```text
leaf = HASH(domain_leaf || payload)
```

`domain_leaf` and `domain_node` are caller-supplied domain separators. V1 does not hard-code a new cryptographic domain string because primitive adapters are not yet validated.

For exactly 16 leaves, scheduling is fixed/unrolled by level:

```text
L0: 16 leaves
L1: 8 parent hashes
L2: 4 parent hashes
L3: 2 parent hashes
L4: 1 root hash
```

The hash callback may execute each level scalar or in 4/8/16-way groups depending on validated capabilities.

## 6. Watchdog

The watchdog is a bounded counter plus epoch. A missed heartbeat transitions validated/candidate execution toward DEGRADED/FAILOVER; rollback is requested if fallback equivalence cannot be established. No watchdog event is silently converted to PASS.

## 7. Promotion rule

A backend may be `VALIDATED` only if:

```text
same input bytes
+ same domain separation
+ same primitive/version
+ bit-identical digest/root
+ recorded toolchain/ISA
+ reproducible test receipt
```

Otherwise state remains `CANDIDATE`, `DEGRADED`, or `TOKEN_VAZIO` at the evidence layer.
