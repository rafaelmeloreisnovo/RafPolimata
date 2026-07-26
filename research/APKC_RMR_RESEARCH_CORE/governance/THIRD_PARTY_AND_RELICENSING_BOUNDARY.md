# Third-Party and Relicensing Boundary

**Coupling ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`

## Rule

Only copyright owned by the licensor can be offered under this nucleus’s research license.

The presence of a file in the same repository, build, archive, APK, ZIP, DEX, ELF, documentation bundle or custody capsule does not change that file’s original license.

## Explicit boundaries

- BLAKE3 remains under its upstream and file-level licenses.
- RMR files already carrying `Apache-2.0 OR CC0-1.0` remain under those terms unless the rights holder makes a separate explicit grant.
- OpenSSL and its derivatives remain under their applicable upstream/file-level terms.
- Android, LLVM/Clang, QEMU, Termux, Gradle, Actions and other tools remain third-party works.
- Generated binaries contain rights and notices inherited from their inputs.
- Standards, papers and public documentation are cited, not relicensed.

## Release gate

Before publication or commercial licensing:

1. generate an SBOM;
2. enumerate file-level SPDX identifiers;
3. detect incompatible or unknown licenses;
4. preserve copyright and notice files;
5. separate original from third-party material;
6. mark unresolved ownership as `TOKEN_VAZIO_OWNERSHIP`;
7. block commercial authorization until critical ownership gaps close.

## No contamination by implication

A top-level license never overrides a more specific file-level license or third-party notice.
