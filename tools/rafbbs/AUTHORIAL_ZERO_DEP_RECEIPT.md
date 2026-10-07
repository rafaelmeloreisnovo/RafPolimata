# RafBBS authorial zero-dependency receipt

Scope: the pure RafBBS V1 slice only. Hosted operators remain adapters and are
not included in this claim.

## Declared pure set

- rafbbs_types.h
- rafbbs_status.h
- rafbbs_time.h
- rafbbs_freestanding.h
- rafbbs_core.h
- rafbbs_context_core.h
- rafbbs_git_core.h
- rafbbs_command_core.h
- rafbbs_manifest_core.h
- rafbbs_filepicker_core.h
- rafbbs_theme.h
- rafbbs_log_core.h
- rafbbs_format_core.h
- rafbbs_runlog_core.h
- rafbbs_pipeline_core.h
- rafbbs_cli_core.h
- rafbbs_tui_core.h
- rafbbs_manifest_bin_core.h
- rafbbs_baremetal.h
- rafbbs_crc32_core.h
- rafbbs_sha256_core.h
- tests/rafbbs_authorial_probe.c
- tests/rafbbs_time_core_test.c
- tests/rafbbs_manifest_core_test.c
- tests/rafbbs_filepicker_core_test.c
- tests/rafbbs_theme_core_test.c
- tests/rafbbs_log_core_test.c
- tests/rafbbs_format_core_test.c
- tests/rafbbs_runlog_core_test.c
- tests/rafbbs_pipeline_core_test.c
- tests/rafbbs_cli_core_test.c
- tests/rafbbs_tui_core_test.c
- tests/rafbbs_context_core_test.c
- tests/rafbbs_command_core_test.c
- tests/rafbbs_git_core_test.c
- tests/rafbbs_manifest_bin_core_test.c

## Gate contract

freestanding/tests/verify_authorial_zero_dep.sh:

- rejects hosted/system includes, host/runtime calls, syscall instruction
  strings, inline assembly and external declarations in the declared set;
- compiles the caller-owned probe with -ffreestanding -nostdinc -fno-builtin;
- checks the six OS-neutral targets:
  x86_64, i686, armv7a, aarch64, riscv32, riscv64;
- rejects undefined object symbols and requires the exported probe symbol;
- proves that `RafContext` no longer requires a hosted time type;
- renders the text manifest without stdio and fails closed on caller-buffer overflow;
- checks exact manifest bytes plus invalid elapsed-time representation;
- proves typed operational detail formatting for the exact RafBBS value families (text, signed i32 and fixed-width hex32) without stdarg/vsnprintf/printf grammar;
- proves persisted run-log header/artifact/gap byte formatting without fprintf,
  snprintf, hosted headers or filesystem access; the adapter only persists already-rendered bytes;
- proves Git HEAD/ref/OID/packed-refs parsing without invoking Git, shell or libc;
- keeps POSIX/civil clock acquisition, filesystem byte observation, FILE persistence,
  terminal input/rendering and actual command execution in hosted adapters outside
  the pure set; Git filesystem bytes enter through a hosted adapter and are parsed by the authorial core; only caller-supplied observations enter the seeded context core
  and only caller-supplied execution outcome enters the command policy core.

`rafbbs_build.sh freestanding` compiles the monotonic-time and text-manifest
falsifiers with hosted headers disabled and executes the same deterministic
semantics as host-CI smokes.
The host execution is not device evidence and does not make the POSIX clock part
of the pure core.

rafbbs_build.sh freestanding remains a local known-vector and object-level
falsifier. The compiler, shell and nm are evidence/factory tools, not runtime
dependencies of the pure objects.

## Boundary

This receipt establishes a source/build gate for the named RafBBS slice. It does
not establish hosted-adapter purity, physical bare-metal execution, device
coverage, semantic equivalence on every target, or claim promotion.

Unobserved runtime/device state remains TOKEN_VAZIO (CLOSURE_L12).
Repository-wide migration beyond this named slice remains TOKEN_VAZIO
(CLOSURE_L11).
