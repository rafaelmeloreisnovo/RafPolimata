# Evidence Garden - BLAKE3 source import/audit receipt - 2026-09-29

**Governance:** `CLOSURE_L2` for runtime-evidence uncertainty.

**State:** `PASS_LIMITED`  
**claim_allowed:** `false`  
**Execution:** local audit of user-supplied evidence files; no BLAKE3 rebuild or rerun was performed in this transaction.

## Source identities

| Source | SHA-256 | Size |
|---|---|---:|
| `blake3_rmr_vs_oficial.pdf` | `603515e499a88d86dbe18bbf4e805e9685ad02a6dadaa30a36e517c6199a0c9d` | 68,502 bytes |
| `blake3_rmr_vs_oficial.md` | `73dc38a1ee1095bc28c9895592fe3e0a5788772af996d802892e7b64423fed8d` | 3,741 bytes |
| `rmr-blake3-upstream-compare-v2.zip` | `c10e509c41bb3659efd1071d7e3896660ba522294273d6c6d794d4d9d4ca4fd8` | 9,986 bytes |

These are three separate source objects. No identity bridge was present that proves the PDF/Markdown run and the V2 ZIP run are the same execution, so they are not merged.

## V2 archive observations actually checked

- 11 archive members total.
- Internal `SHA256SUMS` lists 9 evidence files; all 9 listed digests recomputed successfully.
- `build/rmr-source-authority.json` is inside the archive but is not covered by that internal `SHA256SUMS`; the outer ZIP digest above covers the package as received.
- Exact official commit: `6aab490a26124663329dfd3961b8469f8fdb158b`.
- Exact fork commit: `9571c928808b9aba96e7b9586c54c12ac66cdc3e`.
- Environment records Clang 18.1.3, CMake 3.31.6, 9 rounds and seven sizes: 64, 256, 1024, 4096, 65536, 1048576 and 16777216 bytes.
- `results.csv` has exactly 126 rows = 7 sizes x 9 rounds x 2 variants.
- Every size/round pair contains official+fork and the paired `digest` and `guard` values match.
- Execution order alternates official/fork by round.
- `summary.json` keeps `claim_allowed=false`; six sizes are `PARITY_OR_NOISE_NOT_SEPARATED`, while 256 bytes is `OBSERVED_FASTER_ON_THIS_RUNNER`.
- The fork configure log contains one `CMake Deprecation Warning`; the build logs themselves contain no warning line. Therefore a generic `zero warnings in all phases` claim is false for this archive unless configure warnings are explicitly out of scope.
- The receipt itself states independent third-party reproduction as `TOKEN_VAZIO` and limits the experiment to the fork C core versus official C core; RMR orchestration, I/O, metadata, custody and UI are outside scope.

## Derived statistics boundary

The importer can derive p95/p99 from raw rows, but there are only 9 observations per variant per size. Those tail metrics are therefore `PASS_LIMITED`, not a strong tail-distribution characterization. The archive's paired bootstrap interval is source evidence; its exact resampling implementation is not included in the archive and is not independently reconstructed here.

The 256-byte station's interval excluding parity is not promoted to a global speed claim: seven stations were inspected and no multiple-comparison policy or minimum practically relevant effect threshold is declared in the archive.

## Gaps by urgency

`U0`: exact benchmark executable hashes are absent, so source -> built artifact -> measured execution is not fully bound.  
`U1`: full build/link argv/effective flags, internal digest coverage of `rmr-source-authority.json`, runner governor/frequency/affinity/thermal/load envelope, calibrated observer overhead, multiple-comparison policy and robust tail sample count remain open.  
`U2`: multithread scaling, physical-device evidence and independent reproduction remain open.

## Correction to wording

The PDF/Markdown's statement that the small performance difference *comes from* the build profile should be treated as `OBSERVED_UNPROMOTED` unless a factorial intervention isolates LTO, inline and other changes. The V2 archive is a different source cut and cannot retroactively supply exact commits for the PDF run without an explicit bridge.

## Falsifiable next action

`F_next`: retain/hash the exact official and fork benchmark executables used by each lane, together with verbose build/link commands. That closes the highest-value reconstructibility gap before adding more repetitions.

## R3

- `F_ok`: archive digest integrity for listed members, exact source commits, raw sample shape, paired digest/guard equivalence and bounded descriptive statistics are checked.
- `F_gap`: artifact identity and attribution/generalization gates above remain open.
- `F_next`: bind exact binaries + build argv, then rerun the same frozen station matrix before adding lower-priority observability layers.
