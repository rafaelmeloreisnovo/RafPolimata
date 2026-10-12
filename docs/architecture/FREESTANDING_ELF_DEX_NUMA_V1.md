# Freestanding ELF / DEX / NUMA — bounded structural tranche V1

Lifecycle: `IMPLEMENTED_UNTESTED` until same-HEAD CI. Owner: RafPolimata `freestanding/`. Globally `claim_allowed=false`.
Governed gaps: `CLOSURE_L11` binds uncertainty ownership/route; `CLOSURE_L12` preserves physical/device proof as open. A closure annotation never supplies the missing evidence.

Route: `START HERE -> CURRENT_STATE -> freestanding/README.md -> this contract -> freestanding/tests/verify_formats_numa.sh -> exact-HEAD CI -> receipt`.

## Why these three modules

- `include/raf_fs_binary_view.h`: reusable bound-tested byte ranges and byte-by-byte endian-safe 16/32/64-bit decoders. Avoids struct casts, unaligned loads, overflow-prone `offset+length` checks.
- `include/raf_fs_elf_view.h`: ELF32/ELF64 header and table bounds, big/little endian, machine, entry point, bounded PT_LOAD inspection, `memsz >= filesz`, file range and alignment congruence. **Not** a loader, relocator, linker, verifier of ISA code, ABI compatibility, signatures, complete ELF validity or executable runtime. ELF extended numbering and unsupported encodings are explicitly refused.
- `include/raf_fs_dex_view.h`: deliberately limited DEX v035/037/038/039/040 structural header, ID table bounds, map list bounds and endian tag. The subset refuses newer v041+ containers and reverse endian; refuses nonzero link area. Does **not** check Adler32/SHA-1, map semantic agreement, overlap between all sections, code item validity, dexopt, interpreter, ART or Dalvik compatibility. Even `RAF_FS_DEX_OK` means a **structural probe** only.
- `include/raf_fs_numa_plan.h`: choose among eight **caller-described** memory domains using allowed mask, available pages, origin/distances and deterministic tie policy; does not probe real NUMA topology or allocate/migrate/pin physical pages. Different outcomes: `TOPOLOGY_UNKNOWN`, `NO_ALLOWED`, `NO_FIT`, `BAD_INPUT`, `SELECTED`. Zero mask is not a missing snapshot.

## Boundary / memory authority

The byte buffers, sizes and topology snapshot are supplied by callers; the core has no allocator, heap, libc, API, syscall, JNI, JVM, Android/Linux-specific import, dynamic dependency or hidden compatibility fallback. Output structures are unchanged on rejected ELF/DEX input and NUMA rejection. C structs are internal output contracts, **not** C++/Java classes. Build runner and compiler are hosted tools only.

No arbitrary hostile C pointer can be authenticated by the kernel. Callers must guarantee the declared memory is readable and output does not overlap input. Freestanding structural parsing does not create process isolation.

## Falsifiers / evidence

`sh freestanding/tests/verify_formats_numa.sh` runs source contract, synthetic positives and negatives, host AddressSanitizer/UndefinedBehaviorSanitizer and Clang object builds for x86_64, i686, ARMv7, AArch64, RISC-V32 and RISC-V64, rejecting unresolved symbols. Tests use **synthetic fixtures**; not full real-world corpus, fuzz campaign, physical runtime or independently reproduced binary loading.

Required next gates:
1. exact-HEAD CI terminal result and immutable commit SHA;
2. differential checks against a well-defined real ELF/DEX fixture corpus, including cross-endian and malformed samples;
3. separate format semantic validators for ELF dynamic/relocations and DEX integrity, if higher claims are requested;
4. caller/OS adapter for actual NUMA topology and physical placement, with independent runtime receipt;
5. license/provenance P0 and independent review.

Existing `Apkc/fmt_elf.h` and `Apkc/fmt_dex.h` remain producer-side sources **untouched**; no reuse/relicensing or hidden native dependency is claimed. Existing JMV PR#431 is independent, not silently chained or merged.

References: [ELF gABI](https://gabi.xinuos.com/elf/07-pheader.html), [AOSP DEX format](https://source.android.com/docs/core/runtime/dex-format).

## R3

F_ok = source implementation and adversarial test scaffold, no existing core/generator replacement.
F_gap = exact-HEAD CI until terminal; external rights, real artifacts, physical NUMA/Android and independent evidence `TOKEN_VAZIO`.
F_next = CI readback, targeted falsifiers, reviewer, eventual governed merge; rollback closes draft or scopes a revert to this independent tranche.
