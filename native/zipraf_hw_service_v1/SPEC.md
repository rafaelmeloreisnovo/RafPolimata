# ZIPRAF Hardware Service V1 — Technical Specification

## 1. Contract

The service maps a requested primitive/workload to a backend only when all mandatory gates are known and satisfied.

For backend `b`:

```text
eligible(b) = required_caps ⊆ observed_caps(b)
              AND validation_gate(b)
              AND security_gate_if_required(b)
```

Among eligible backends with complete required measurements:

```text
W = {w_l, w_c, w_t, w_z, w_s, w_e}
Σ W = 65536  (Q16.16 representation of 1)
C_b = {latency, cycles, inverse_throughput, code_size, stack, energy_proxy}
score(b) = Σ_i ((W_i * C_bi) >> 16)
```

All costs are normalized in `[0, 65536]`; lower is better. The normalization window and benchmark corpus are external evidence and must be recorded in the receipt. No normalization constant is universal.

## 2. Determinism

Given identical capability profile, policy, normalized measurements and versioned registry, selection must be deterministic. Ties are resolved by stable backend ID, never randomness or wall clock.

## 3. Portability

C11 freestanding is the semantic ABI baseline. Rust `#![no_std]` mirrors the scoring/registry contract. ASM is allowed only as a backend after golden equivalence and measured benefit.

Supported architectural families are intentionally open-ended. Current RafPolimata routes include ARMv7-A, AArch64, x86-64 and additional cross-architecture profiles elsewhere in the repository; physical validation remains per target.

## 4. Cryptography

This module is an adapter/registry, not a new cryptographic construction. Primitive constants, word sizes, rounds, nonces, signatures, tags and security requirements are inherited from their standards/reference specifications. `native/raf_hash_fabric_v1` remains the nearest execution-fabric authority.

States:

```text
REFERENCE -> IMPLEMENTED -> CANDIDATE -> VALIDATED
                              |             |
                              v             v
                           BLOCKED      LEGACY_ONLY
```

`TOKEN_VAZIO` means a required implementation or evidence edge is absent.

## 5. Filesystem capability composition

Decision inputs:

```text
subject = principal + group
object  = id + owner + group + base rights + policy flags
acl     = zero or more subject-specific allow/deny masks
cap     = optional subject/object-scoped rights + expiry
request = right mask
```

Effective rights:

```text
R0 = base(subject, object)
R1 = R0 ∪ ACL_ALLOW
R2 = R1 ∪ valid_capability
R3 = R2 \ ACL_DENY
R4 = R3 constrained by IMMUTABLE/APPEND_ONLY/NOEXEC
allow iff request ⊆ R4
```

This makes explicit deny final and makes object flags non-bypassable in this policy core.

## 6. Evidence gates

A backend performance claim requires at minimum:

```text
repo + commit + artifact hash
compiler/toolchain/version + flags
hardware/ISA/feature identity
input corpus/hash + size distribution
warmup/repetition methodology
ns/op + cycles/byte where measurable + MB/s
code size + stack bound
correctness oracle/KAT
exit status + raw result hash
```

Energy remains a proxy unless measured by an identified physical meter/counter with units and sampling method.

## 7. Security defaults

- fail closed when required crypto evidence is absent;
- no network or privilege escalation in installer;
- no secrets embedded in registry or receipt;
- no MD5 for security identity/signature/custody;
- no algorithm renamed as an original primitive;
- third-party source keeps its own license and notices;
- constant-time is a separate evidence gate, not inferred from branch count or language.
