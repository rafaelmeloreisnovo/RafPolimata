# Runtime state, targets and temporal truth

**Governance binding: CLOSURE_L11** — runtime and device unknowns remain unknown until observed.  
**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92

## ECOSYSTEM_RUNTIME_STATE.json is a snapshot

ECOSYSTEM_RUNTIME_STATE.json declares:

- schema raf.ecosystem-runtime-state.v1;
- observed_at 2026-07-18T20:04:33-03:00;
- component-specific implementation/evidence states;
- historical CI availability assumptions.

Its CI section says Actions execution was out of scope because the account had no Actions credit at that time. In September 2026 this is observably stale: GitHub Actions runs are executing on the repository.

Therefore:

~~~text
runtime snapshot = historical evidence
runtime snapshot != automatic current HEAD state
~~~

Do not rewrite that historical file merely to make it look current. Produce a successor snapshot if the runtime-state schema remains authoritative.

## Current compiler target registry

compiler/targets.v1.json declares:
- claim_allowed false;
- offline-only compiler policy for the registered route;
- install/launch forbidden until applicable receipt;
- target entries for RafPolimata compiler/semantic cores and paired ecosystem repositories.

The registry is a build/target contract, not proof that every target's build ran in this cut.

## Seven active architecture policy

compiler/architectures.v2.json declares exactly seven active architectures:

| ID | Role |
|---|---|
| aarch64 | primary |
| armv7a | physical compatibility |
| x86_64 | host/virtualization |
| riscv64 | research |
| mips64r6el | research compatibility |
| s390x | research / big-endian |
| loongarch64 | research |

i386/IA-32/x86-32/80386 is explicitly retired by that registry.

Each architecture carries an execution field that remains receipt-gated. Cross-compilation and metadata do not become device validation.

## Android boundary

For current-artifact Android runtime claims, docs/AGENTES.md requires a continuous chain:

~~~text
source
-> ARM artifact
-> current APK
-> identity of embedded ELF
-> current signature/verification
-> install
-> launch/dlopen
-> runtime observation
-> logcat/exit/receipt
~~~

Missing links preserve the corresponding unknown state.

## Hosted vs freestanding

Hosted development paths may use host services declared by their contract. That does not relax the freestanding L0 or target-specific no-runtime constraints.

## Temporal reconciliation rule

When two state surfaces disagree:
1. compare observed_at/date;
2. compare commit/revision;
3. identify whether each is source, artifact, receipt or narrative;
4. prefer the freshest authoritative evidence for current state;
5. preserve historical state rather than overwriting it;
6. write a new receipt/snapshot for the delta.

R3 = ⟨F_ok: runtime snapshot, compiler targets and architecture registry separated by temporal role; F_gap: no successor ECOSYSTEM_RUNTIME_STATE snapshot was produced by this documentation-only cut; F_next: generate a new runtime-state snapshot through its owning process when current runtime truth is required⟩.
