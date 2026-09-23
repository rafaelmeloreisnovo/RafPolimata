# Receipt successor R2 — ZIPRAF Hardware / Crypto / FS Canonical V1

parent: canonical/zipraf-hw-v1/receipts/2026-09-23_ZIPRAF_HW_CANONICAL_V1.md
timestamp_local: 2026-09-23T06:38:00-03:00
claim_allowed: false

Delta:
- added hosted C installer with explicit prefix;
- installer performs no network fetch;
- installer performs no privilege escalation;
- installer copies only registry/policy assets in this gate.

Observed local:
- clang -std=c11 -Wall -Wextra -Werror tools/zipraf_installer.c: PASS
- install to /tmp/zipraf-install-test: PASS
- expected installed files: 2/2

Remaining:
- Rust cargo test: NOT_RUN locally;
- repository exact-head CI: PENDING;
- physical ARMv7/AArch64 runtime: TOKEN_VAZIO;
- external security audit: TOKEN_VAZIO.
