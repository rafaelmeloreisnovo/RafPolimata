# RAFAELIA — Virtual Bit Fabric / software syslog / watchdog V1

State: SOURCE CANDIDATE / FAIL-CLOSED / NO HARDWARE OR INDEPENDENT CLAIM

## Purpose and strict boundaries

A new authorial C11 freestanding L0 block adjacent to existing
`freestanding/arch/raf_fs_vector.h`. It does not replace ISA-specific vector
executors or alter them. The static pure-C kernel simulates a single-bit path
between eight caller-owned, abstract registers in an explicit 8/16/32/64-bit
model. No OS/ABI syscalls, libc, GC, heap, clock, thread, I/O, class or
native application APIs exist in the model. Compiler/CI/shell are hosted
**factory tools**, never target runtime dependencies.

A modeled register is NOT a real CPU register; a model profile is NOT
instruction-set emulation. The tool cannot observe real transistors,
undocumented microcode, caches, speculative execution, interrupts, analog
hardware, thermals, power consumption or physical clock cycles.

## Eleven explicitly modeled abstract profiles

MCU8 and MCU16 are generic logical word-width models, not specific chips.
i686, x86_64, ARMv7-A, AArch64, RV32, RV64, POWER64, s390x and LoongArch64
are **width labels only**. They do not implement instruction semantics,
privileged registers, vector width, memory model, topology, virtual machine
or hardware timing. No blanket claim of all CPU models is made.

## Pipeline and owned state

`caller input -> validate -> sample virtual source bit -> write one virtual destination bit -> update parity -> append bounded trace -> state checksum -> chain -> guard`

- `raf_fs_vbf_init`: typed profile validation and reset of caller-owned bytes.
- `raf_fs_vbf_step`: one transaction (`INPUT` or `ROUTE`), explicit bit and
  destination, fail-closed on corruption/invalid input/trace full.
- `raf_fs_vbf_check`: layered watchdog for model bounds, even/odd parity,
  state checksum, entire event chain and guard. It is deterministic, not
  cryptographic or tamperproof; keyed signatures and real-world chain-of-custody
  belong outside L0 in an explicitly trusted adapter.
- Sixteen **mandatory** trace entries, never silently overwritten; after
  that, `TRACE_FULL` with no state change. External continuation requires
  explicit ownership handoff, new model context and receipt.
- Logical `tick` is number of modeled transactions, NOT hardware cycles or time.

The `CLOSURE_L11`/`CLOSURE_L12` gates remain open for per-file legal rights,
independent audit, host-actual emulation, ARM device execution and physical
measurement. `TOKEN_VAZIO` is neither bit zero nor evidence of failure/success.

## Comment and compiler policy

`RAFAELIA-L0-FILE-CONTRACT` and `ANNOTATION:` are semantically reviewed
source contracts, **not an executable comment language**. Comments cannot
cause a compiler to remove loops or overlay registers. Optimization is
enforced by actual flags and read-back of codegen: `-ffreestanding`,
`-fno-builtin`, `-nostdinc`, `-fno-stack-protector`, `-O2`, optional
`-ffunction-sections -fdata-sections`, linker `--gc-sections` if supported.
Loop removal and inline ASM require measured evidence of equivalence and
register/ABI/clobber effects, not annotations alone.

## Falsifiers and replay

`sh freestanding/tests/verify_virtual_bit_fabric.sh`:
- 11 width-models x 16 seeded operations, independent scalar state oracle;
- single-bit error rejected by parity;
- two-bit error rejected by state digest when parity cannot detect it;
- event payload tampering rejected by replayed chain;
- guard tampering rejected by meta-watchdog;
- exhaustion/invalid input rejected without mutation;
- 63rd-bit path and six OS-neutral object builds with `nm -u` empty.

These tests can be defeated by untested implementation errors and deliberate
coordinated tampering; state checksum is not collision-resistant. Academic
falsification: use seeded mutation campaigns, state-space exploration,
independent oracle, concolic analysis and bounded property checking before
any stronger reliability claims.

## Privacy and zero trust

No real device identifiers, memory dumps, secrets, credential strings, personal
records, trace uploads, or hidden telemetry. The trace stores synthetic
virtual event metadata in caller-supplied memory only; user explicitly controls
later redaction/export. Never coerce `UNKNOWN` into `PASS`; an adapter cannot
borrow authority from a synthetic register trace.

## Rollback and acceptance

Additive 6-path module; rollback by closing draft PR. No target runtime,
vector executable, main branch, provider rule, secret, device or release
mutation. Acceptance needs exact HEAD hosted tests + six-object source gate;
independent review, rights and real hardware are separate blocked gates.
