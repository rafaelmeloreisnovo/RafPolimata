# ApkC proof index

- [Custody proof](CHAIN_OF_CUSTODY_2026-06-14.md)
- [Validation matrix](VALIDATION_MATRIX.md)
- [Checklist](hello-apk-validation.md)
- [Gaps](GAPS.md)
- [Artifacts policy](ARTIFACTS.md)
- [Evidence summary](out/evidence-summary-2026-06-14.md)
- [Validation summary](out/validation-summary.md)
- [Hash ledger](out/artifact-sha256.txt)
- [Generate log](out/apkc-generate.txt)
- [Compile log](out/apkc-compile.txt)
- [ZIP log](out/unzip.txt)
- [AXML log](out/aapt-xmltree.txt)
- [DEX log](out/dex-sha1.txt)
- [ELF ARM32 log](out/readelf-arm32.txt)
- [ELF ARM64 log](out/readelf-arm64.txt)
- [Signature log](out/apksigner-verify.txt)
- [Android install log](out/adb-install.txt)
- [Runtime log](out/logcat-nativeactivity.txt)

## Current state

PASS: generated APK, ZIP parse, AXML parse, DEX hash, ARM32 ELF, signature check.
SKIP: ARM64 ELF.
TOKEN_VAZIO: runtime log and complete source build provenance.
