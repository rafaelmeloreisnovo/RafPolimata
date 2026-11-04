# RAF Runtime Morph V1 — gated architecture and T7 semantic presence

**Status:** additive `SOURCE_IMPLEMENTED`, pending exact-head CI; `claim_allowed=false`.

**Authority:** `Benchmark/raf_runtime_router.h` stays the legacy route; this opt-in policy lives in `Benchmark/raf_runtime_morph_v1.h`. `freestanding/` remains a separate canonical L0 boundary. Never copy syscall wrappers from the legacy router into a freestanding hot core.

## Two bounded cycles

1. **Census → source → executable policy.** Inspect the Git tree recursively with `truncated=false`, classify at most five path components for operational ownership; explicitly report deeper paths as `OUT_OF_SCOPE_DEPTH5`, not silently omitted. Map capability to compiler output, callable implementation, exact-revision verification and fault quarantine. Selection is scalar and may compile to branchless selects; disassembly is necessary to claim branchless machine code.
2. **Fault injection → recovery → semantic alignment.** Feed a backend result and a generation-tagged decision into a caller-owned journal. A failure quarantines its selected acceleration and chooses eligible alternate or generic; stale feedback does not mutate the journal. Separately update seven Q16 toroidal coordinates with an explicit presence mask. Re-run falsifiers and record receipt/rollback. This is *policy failover*, not physical failover or persistent rollback.

## Capability gate

```text
eligible = detected & compiled & executable & verified & ~(degraded | quarantined)
selected = architecture_match(eligible) & phase(CANDIDATE|VALIDATED)
otherwise = GENERIC
```

- `detected` must come from a trustworthy platform adapter. Do not read ARM32 NEON only from `__arm__`; test NEON support and ABI independently.
- `compiled` means the implementation is built for exactly that ISA; `executable` means the symbol exists. **This patch ships neither ARM32 nor ARM64 vector executor.** Until both are implemented and tested, the adapter must set their bits to zero. This header cannot manufacture them.
- `verified` is scoped to a build/behavioral gate, not Android device proof; no self-attestation from a source file.
- GPU, syscall, and storage are intentionally not eligible in V1. The legacy router may still enumerate them; its previous behavior is preserved, not endorsed for new calls.
- Generic C is the always-available arithmetic reference. No heap/GC/syscall/libc in this header, and no probe or operating-system side effects.

## T7 and evidence

The seven positions `(u,v,psi,chi,rho,delta,sigma)` are Q16-grid coordinates. Missing data is represented by a separate bit in `present_mask`, not by `0`. V1 uses the exact existing integer recurrence `old-(old>>2)+(sample>>2)` modulo `2^16`, and updates seven lanes without a loop. Legacy `Benchmark/raf_toroid.h` is unchanged.

`42` here is only a finite index in the older state machine. The strong claim that there are 42 convergent fixed-point attractors was falsified; see `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md`. Neither wave mechanics nor acoustics nor translation meaning is inferred from numeric Q16 slots.

## Polyglot / acoustic future mapping, not executable physics

Keep a typed *orthogonal* semantic layer: `{language, script, direction, phoneme/IPA, syllable, prosody, stress, cadence, sample_rate_hz, channel_layout, ambisonic_order, window, source_ref, presence, uncertainty}`. Hebrew/Aramaic RTL, Greek, Chinese, Japanese and Portuguese/English should each preserve its own orthography, pronunciation, translation provenance and cultural context. A text token is not an audio waveform; Hz is a measured sampling/frequency unit, not a universal meaning carrier. Mixed-script glyphs and Tao/Yeshua metaphors remain versioned dictionary entries, not neuroscientific or quantum claims.

A future implementation should use a separate audio adapter, explicit PCM rate (e.g., 48 kHz), channel convention (ACN/SN3D for ambisonics), window/STFT size, synchronisation and unit tests. `1 kHz` is one possible tone frequency, **not** a speech or scripture invariant. NP vs P is a complexity-theory distinction, not a proof about language translation.

## Tests and rollback

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-builtin -I Benchmark Benchmark/raf_runtime_morph_v1_test.c -o /tmp/raf_morph_v1_test
/tmp/raf_morph_v1_test
```

A zero exit proves only these host policy/fixed-point test cases. Cross-target `-fsyntax-only`/`-c` prove code generation, not physical NEON. To roll back, stop opting into V1 and use the unchanged legacy route; do not merge without independent review, exact-head CI, architecture-specific executable gates and physical device proof where claimed. The journal stores no durable artifact and cannot restore external I/O.

**SOURCE ≠ ARTIFACT ≠ EXECUTION ≠ EVIDENCE ≠ CLAIM.** `TOKEN_VAZIO` is never imputed to zero. Receipt must record exact SHA and tests before promotion.
