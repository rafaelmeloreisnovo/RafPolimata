# RafBBS authorial zero-dependency receipt

Scope: the named pure RafBBS V1 slice only. Hosted operators remain adapters and
are not included in this claim.

State: `SOURCE/BUILD_GATE_DECLARED`  
Claim ceiling: `claim_allowed=false` beyond the named source/build slice.

## Declared pure set — 14 modules

- rafbbs_types.h
- rafbbs_status.h
- rafbbs_time.h
- rafbbs_freestanding.h
- rafbbs_core.h
- rafbbs_manifest_core.h
- rafbbs_filepicker_core.h
- rafbbs_theme.h
- rafbbs_log_core.h
- rafbbs_pipeline_core.h
- rafbbs_baremetal.h
- rafbbs_manifest_bin_core.h
- rafbbs_crc32_core.h
- rafbbs_sha256_core.h

## Falsifiers / probe

- tests/rafbbs_authorial_probe.c
- tests/rafbbs_zero_dependency_test.c
- tests/rafbbs_time_core_test.c
- tests/rafbbs_manifest_core_test.c
- tests/rafbbs_filepicker_core_test.c
- tests/rafbbs_theme_core_test.c
- tests/rafbbs_log_core_test.c
- tests/rafbbs_manifest_bin_core_test.c
- tests/rafbbs_pipeline_core_test.c

## Gate contract

`freestanding/tests/verify_authorial_zero_dep.sh`:

- rejects hosted/system includes, host/runtime calls, syscall instruction
  strings, inline assembly and external declarations in the declared pure set;
- compiles the caller-owned probe with `-ffreestanding -nostdinc -fno-builtin`;
- checks six OS-neutral targets: x86_64, i686, armv7a, aarch64, riscv32, riscv64;
- rejects undefined object symbols and requires the exported probe symbol;
- exercises monotonic arithmetic, text/binary manifests, filepicker state,
  status/theme rendering, deterministic log-line rendering and pipeline-spec
  exact-byte lookup;
- keeps POSIX clock acquisition, varargs/detail formatting, command execution,
  provider/device access, console presentation and FILE/filesystem persistence
  outside the pure set.

`tools/rafbbs/rafbbs_build.sh freestanding` compiles the semantic falsifiers
with hosted headers disabled, audits unresolved object symbols and executes
deterministic host smokes. Compiler, shell, `nm` and CI are evidence/factory
tools, not runtime dependencies of the pure objects.

## Adapter boundary

Hosted adapters intentionally remain outside the claim:

- `rafbbs_time_posix.h`
- `rafbbs_crc32.h`
- `rafbbs_sha256.h`
- `rafbbs_manifest.h`
- `rafbbs_manifest_bin.h`
- `rafbbs_filepicker.h`
- `rafbbs_log.h`
- `rafbbs_pipeline.h`

The pipeline pure core owns only immutable specs/flags and exact-byte lookup.
Handler binding, `RafContext`, status transitions, command execution and
provider/file/hash orchestration remain hosted.

## Evidence boundary

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`

This receipt establishes a source/build gate for the named slice only. It does
not establish physical bare-metal execution, device coverage, hosted-adapter
purity, full toolchain self-hosting or repository-wide zero dependency.

Unobserved runtime/device state remains
`TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.
Repository-wide migration beyond the named slice remains
`TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)`.
