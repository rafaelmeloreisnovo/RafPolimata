# Runtime state, targets and temporal truth

**Governance binding:** CLOSURE_L11 for operational/topology gaps; CLOSURE_L12 for runtime/device evidence gaps.  
**Historical base:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Boundary hotfix parent:** main@d04137f76fffef215428c3c9f7ce12d3695ca390

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

## Hosted vs freestanding vs raw-syscall userspace

The execution boundary is four-way, not binary:

| class | OS/kernel dependency | valid description | evidence gate |
|---|---|---|---|
| `FREESTANDING_L0` | none | OS-neutral, no libc/heap/GC/syscall | source + OS-neutral compile/codegen gates |
| `RAW_SYSCALL_USERSPACE` | Linux/Android ABI | no-libc userspace with direct kernel traps | ABI compile + same-artifact userspace run |
| `HOSTED` | declared host runtime | libc/CRT/services permitted by route | route-specific host tests |
| `PHYSICAL_BARE_METAL` | no OS/kernel syscall | board/firmware owns startup, memory map and device access | board/startup/linker/device receipt |

`-ffreestanding`, `-nostdlib` and static linking are build properties; they do not by themselves prove physical bare-metal execution.

For the current Benchmark path:

```text
Benchmark/raf_sys.h     = RAW_SYSCALL_USERSPACE
Benchmark/raf_main.c    = RAW_SYSCALL_USERSPACE
freestanding/**         = FREESTANDING_L0
syscall/**              = optional Linux ABI adapter, outside L0
termux-app-rafacodephi  = runtime/evidence authority for its Android execution lane
```

Termux/RafCodePhi may execute or observe an artifact and emit device evidence. It does not import Android/Linux assumptions into `freestanding/` and does not promote a raw-syscall ELF into physical bare-metal firmware.

### VOID / TOKEN_VAZIO

`VOID` is a typed structural state for a placeholder/reference without sufficient body. `TOKEN_VAZIO` is an epistemic/evidence state. Neither is a universal numeric zero or a magic compiler flag. APIs may encode them only through the status contract of that API.

Physical bare-metal execution for the Benchmark raw-syscall artifact is structurally inapplicable as-is; a separate firmware/startup adapter would be required. Device evidence for any such future adapter remains `TOKEN_VAZIO (CLOSURE_L12)` until observed.

### Modular specialist profiles

The existing specialized modules remain authoritative instead of introducing another architecture layer:

- scalar/OS-neutral matrix: `freestanding/tests/verify_matrix.sh`;
- fixed SIMD: `verify_profiles.sh`;
- scalable vectors: `verify_scalable.sh`;
- matrix registers: `verify_matrix_accel.sh`;
- register metadata: `verify_register_metadata.sh`;
- Linux syscall ABI: `syscall/tests/verify_matrix.sh`;
- cross-layer classification: `scripts/verify_execution_boundaries.sh`.

The generic L0 remains small; ISA-specific behavior stays in specialist profiles and the OS ABI stays outside it.

## Temporal reconciliation rule

When two state surfaces disagree:
1. compare observed_at/date;
2. compare commit/revision;
3. identify whether each is source, artifact, receipt or narrative;
4. prefer the freshest authoritative evidence for current state;
5. preserve historical state rather than overwriting it;
6. write a new receipt/snapshot for the delta.

R3 = ⟨F_ok: runtime snapshot, compiler targets and architecture registry separated by temporal role; F_gap: no successor ECOSYSTEM_RUNTIME_STATE snapshot was produced by this documentation-only cut; F_next: generate a new runtime-state snapshot through its owning process when current runtime truth is required⟩.
