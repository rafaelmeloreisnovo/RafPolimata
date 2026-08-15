---
applyTo: "Apkc/**"
---

# ApkC — path-specific instructions

Read `AGENTS.md`, `docs/AGENTES.md`, `Apkc/PROTOCOL.md`, `docs/APKC_PROTOCOL.md`, and the tests/gates for the file being changed.

## Hosted and freestanding are different contracts

Do not repeat the obsolete blanket rule "no libc anywhere in ApkC".

- ARM/freestanding routes: preserve no-libc/no-heap requirements defined by their current gates.
- Hosted x86/x86_64 development routes: libc wrappers are allowed only inside the explicitly hosted boundary.
- Never make freestanding code depend on the hosted wrapper.

## Buffer and parser discipline

- No silent truncation.
- Validate capacity before advancing an output position.
- Treat NULL/unknown language/profile results as normal error paths and guard them.
- Do not weaken range/offset/alignment checks to make a fixture pass.
- Do not hide compiler or runtime failures with unconditional success paths.

## Formats and ABI

Changes to ELF, DEX, AXML or ZIP/APK require checking all affected offsets, counts, links, alignment, endianness, ABI assumptions and independent validators.

Do not mutate an established persisted/binary format in place if the change is incompatible; version it or provide a compatible reader/migration as appropriate.

## Languages and encoders

A language or instruction change is complete only when all required ends of the current contract are updated. A table entry or encoder function alone is not sufficient.

For ARM instruction work, preserve golden/roundtrip tests and semantics including side effects/flags where applicable.

## Current commands

Prefer repository entrypoints over copied historical commands:

```sh
make syntax
make compiler-contract
make compiler-selftest
```

Use additional subsystem gates only when applicable to the changed path.

## Android evidence boundary

Do not infer device execution from a generated ELF/APK.

```text
source != ELF != APK packaged != signature verified != installed != launched/runtime proven
```

Each promotion requires its own current-artifact evidence.

## Handoff

State whether the edited path is hosted, freestanding, or shared; list commands actually executed and preserve `TOKEN_VAZIO` for unavailable architecture/device/tool evidence.
