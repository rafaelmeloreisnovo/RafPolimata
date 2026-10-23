# AGENTS.md — RAFAELIA field_lab L0 boundary

This subtree adds *independently authored* C freestanding source for the
RafPolimata field laboratory. Before edits read root AGENTS.md,
docs/AGENTES.md and docs/field-lab/APKC_FIELD_LAB_BRIDGE_V1.md.

## Authority and provenance
- The user owns local authorial design; this file does not prove exclusive
  authorship of standardized algorithms. SHA3-256 follows public FIPS 202
  mathematics, ZIP32 follows the published container layout; no third-party
  source or implementation body may be pasted here without rights review.
- Do not copy EstudioAudio/BLAKE3/RLL implementation bodies or licenses.
  Those repositories are architecture comparisons and separate producers.
- Preserve root license and preexisting C04 fixtures. No upstream overwrite.
- For edits run exact-head CI, SHA3 known-answer (empty, abc, multiblock),
  Python ZIP/WAV oracles, PCM boundaries, spectrum fixtures, and both ABI
  ELF/undefined-symbol/NEEDED checks. Add at least one negative falsifier
  for any new capability.
- Hard limit: no libc, malloc, heap, hidden global mutable state, OS headers,
  Android headers, syscalls, file descriptors, network or JNI in L0.
  Compiler/runner/Python may be hosted only.
- Caller owns every output/state buffer. Preserve no-write-on-failure and
  bounds checking for all ingress.
- The `libraffieldlab.so` module being present in an APK does not mean the
  app loads it. Manifest + DEX + Android UI/loader/native activity/IME are
  separate contracts; `TOKEN_VAZIO_NOT_CONNECTED` until tested.
- No private material, PAT value, unsafe network test, release signing,
  automatic promotion or device installation. Protect children and dignity;
  avoid personally identifying or sensitive observations entirely.

## Evidence
`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`.
`TOKEN_VAZIO != 0`, `PASS_HOST != PASS_DEVICE`, `claim_allowed=false`.

When HEAD and artifact are unequal, a previous receipt does not transfer.
No merge or release without separate authorized human review. Rollback
reverts only added field_lab source/tests/workflow and preserves history.
