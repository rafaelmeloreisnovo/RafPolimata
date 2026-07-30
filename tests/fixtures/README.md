# Regression Corpus — Format Fixtures (audit lacuna L17)

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `quality-assurance`  
**Repositório:** [`rafaelmeloreisnovo/RafPolimata`](https://github.com/rafaelmeloreisnovo/RafPolimata) — `tests/fixtures/README.md`

This directory documents the **regressive corpus of formats** plan for the APKc
output (ELF / AXML / DEX / ZIP-APK). It closes audit lacuna **L17 — Corpus
regressivo de formatos** as far as is tractable on a host with **no ARM
toolchain and no ability to run `apkc`** (apkc is freestanding ARM64; this host
is x86_64).

## What L17 asks for

The original audit notes that there is proof for **one** minimal APK
(`hello.apk`, see `Apkc/proofs/out/` and `Apkc/proofs/runs/2026-06-14-tv/`) but
**no regression corpus** exercising format variation:

- manifests with multiple `<uses-permission>` entries,
- a larger / non-trivial DEX,
- different `lib/<abi>/<name>.so` library names and multiple ABIs,
- non-standard ZIP alignment,
- extra ELF sections,
- negative cases (a corrupt ZIP/APK *must* be rejected).

## Tractability split (honest)

| Need | Tractable on this host? | Why |
|---|---|---|
| Generate new APK fixtures | NO | needs `apkc` to **run** → ARM64 only |
| Generate ELF/.so with custom sections | NO | needs `apkc` / ARM cross-tooling to run |
| Golden-assert over **already committed** proof artifacts | YES | pure text/structure checks (`tests/test_format_fixtures.py`) |
| Negative ZIP-rejection contract | YES | pure Python `zipfile` round-trip (`tests/test_zip_negative.py`) |

So the corpus is split into: (a) **structural golden tests** over the artifacts
already committed under `Apkc/proofs/`, runnable anywhere; and (b) a documented
list of fixtures that stay `TOKEN_VAZIO` until someone runs `apkc` on an ARM64
host and commits the resulting artifacts.

## Corpus cases

Legend: **PASS** = backed by a real committed artifact that a test reads and
asserts on. **TOKEN_VAZIO** = case requires `apkc`-on-ARM to produce the
artifact; not yet present; tests SKIP it (never counted as a failure).

| # | Corpus case | What it proves | State | Backing artifact |
|---|---|---|---|---|
| 1 | `minimal.apk` (ELF32 ARM .so) | the minimal `libhello.so` is a valid `ELF32 / Machine ARM / EABI5` shared object | **PASS** | `Apkc/proofs/out/readelf-arm32.txt` (real content: `Class: ELF32`, `Machine: ARM`, `ABI: EABI5 soft-float`, `Status: PASS`) |
| 2 | sha256 ledger well-formedness | every proof artifact line is `(<64-hex>) (<name>) bytes=<N>` and parses cleanly | **PASS** | `Apkc/proofs/out/artifact-sha256.txt` (fenced ledger), `Apkc/proofs/runs/2026-06-14-tv/sha256.txt` (plain ledger) |
| 3 | `minimal.apk` ZIP membership | unzip lists `AndroidManifest.xml`, `classes.dex`, `lib/<abi>/lib*.so` | **TOKEN_VAZIO** | `Apkc/proofs/out/unzip.txt` is `TOKEN_VAZIO: hello.apk ausente`. `validation-summary.md` records "unzip lista 3 entradas" but the raw listing was not committed → cannot golden-assert. Needs apkc-on-ARM to regenerate. |
| 4 | `multi-abi` (arm64 + arm32) | APK carries both `lib/arm64-v8a/*.so` and `lib/armeabi-v7a/*.so` | **TOKEN_VAZIO** | `Apkc/proofs/out/readelf-arm64.txt` is `TOKEN_VAZIO` (no `lib/arm64-v8a/*.so` in the uploaded APK; see F6-arm64 SKIP in `validation-summary.md`). Needs apkc-on-ARM. |
| 5 | `with-permissions` (multi `<uses-permission>`) | AXML string pool + manifest carries N permissions, parses under aapt | **TOKEN_VAZIO** | `Apkc/proofs/out/aapt-xmltree.txt` is `TOKEN_VAZIO: aapt nao executado`. The committed proof only had a single-permission manifest (`com.rafael.teste`). Needs apkc-on-ARM + aapt. |
| 6 | `larger-dex` | a non-minimal `classes.dex` keeps a stable, verifiable SHA-1 | **TOKEN_VAZIO** | `Apkc/proofs/out/dex-sha1.txt` is `TOKEN_VAZIO`. (The 200-byte `_dex_buf` minimal DEX had SHA-1 `9ea7c00884bffbdbbab055ddc3ad6565050fc4e4` per `validation-summary.md` F5, but no larger-DEX artifact exists.) Needs apkc-on-ARM. |
| 7 | `custom-libname` | non-default `lib/<abi>/<name>.so` member name round-trips | **TOKEN_VAZIO** | no committed artifact varying the lib name; the only proof uses `libhello.so`. Needs apkc-on-ARM. |
| 8 | non-standard ZIP alignment | `.so` members remain 4-byte / page aligned at non-default offsets | **TOKEN_VAZIO** | no committed alignment-probe artifact. Needs apkc-on-ARM + `zipalign -c`. |
| 9 | extra ELF sections | adding a section keeps `sh_link` / `e_shstrndx` cross-refs valid | **TOKEN_VAZIO** | no committed multi-section ELF proof; the arm32 proof shows `Section headers: 7` only. Needs apkc-on-ARM + readelf. |
| 10 | `bad-zip` (negative) | a corrupt central directory **must** be detected/rejected | **PASS** | `tests/test_zip_negative.py` builds and rejects a corrupt ZIP locally (pure Python; documents the EOCD/central-dir contract apkc's `fmt_zip.h` writer must satisfy). |

## How to advance a TOKEN_VAZIO case to PASS

1. On an ARM64 host with the Android SDK build-tools, run `apkc` to generate the
   fixture (e.g. a manifest with multiple permissions, or a multi-ABI APK).
2. Capture the structural transcript (`readelf`, `unzip -l`, `aapt dump
   xmltree`, `sha1sum classes.dex`) into `Apkc/proofs/out/` or a new
   `Apkc/proofs/runs/<timestamp>/` directory.
3. Add an assertion branch in `tests/test_format_fixtures.py` that reads the new
   artifact. The test auto-SKIPs (`TOKEN_VAZIO`) whenever the artifact is absent
   or still carries the `TOKEN_VAZIO` sentinel, so it stays green until real
   evidence lands.

## Files

- `tests/test_format_fixtures.py` — golden/structural tests over committed proof
  artifacts (x86-runnable, no ARM, no apkc).
- `tests/test_zip_negative.py` — self-contained negative ZIP-rejection test.
