# Coupled Build Contract

**Coupling-ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`  
**Contract-Role:** `NORMATIVE_BUILD_DOCUMENT`  
**License-Role:** `CC-BY-NC-SA-4.0`

## Invariant

The build is valid only when the same generation binds:

```text
compiler source
+ public interface
+ normative source comments
+ policy stubs
+ documentation
+ software license
+ documentation license
+ commercial policy
+ high-risk boundary
+ custody governance
+ required notices
+ build verifier
= one artifact root
```

The comments are not ordinary decoration. Required comment markers are declared
in `contracts/coupling.v1.json` and checked before compilation.

## Build order

```text
verify exact sealed file identities
→ verify required comment and license markers
→ recompute SHA-256 artifact root
→ compare artifact root with lock
→ regenerate compiled coupling header
→ compile core and stubs
→ reject undefined external symbols
→ emit receipt
```

Compilation must never run after failed verification.

## Meaning of bit coupling

A changed bit does not break the C language by itself. It invalidates the
authorized build generation:

\[
\Delta bit_i
\Rightarrow
identity_i' \ne identity_i
\Rightarrow
artifact\_root' \ne artifact\_root
\Rightarrow
build=FAIL
\]

An intentional edit requires review, a new lock and a new coupling generation.

## Commands

```sh
make -C research/APKC_RMR_RESEARCH_CORE verify
make -C research/APKC_RMR_RESEARCH_CORE coupled-build
make -C research/APKC_RMR_RESEARCH_CORE test
```

## Stub rule

Every unavailable capability is compiled as `FORBIDDEN`, `TOKEN_VAZIO` or
`NOT_AUTHORIZED`. No placeholder returns success.

## Claim boundary

A successful coupled build proves that the sealed source generation, comments,
documents, policies and stubs agree byte-for-byte. It does not prove APK signing,
device installation, launch, performance, safety or regulatory approval.
