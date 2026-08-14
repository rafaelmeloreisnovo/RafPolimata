# ApkC Physical Gate Code-Binding — Main Closure — 2026-08-14

State: `LANDED_PROVIDER_VERIFIED / claim_allowed=false`

- PR: `#251`
- merge commit: `e71824569f34b244ef7fefb5b8704beb9fd2dc33`
- exact PR head: `d7b89aa365d5acbb0b0292a8372137f141415780`
- dedicated workflow: `ApkC Physical Gate Code Binding`
- run: `31845368551`
- job: `94910588865`
- job conclusion: `success`
- executed steps: checkout, syntax, gate-code custody falsifiers, preserve evidence — all `success`
- companion workflows on the same head: CI, ApkC First Part Closure, Document Governance, Formal Science Orchestrator — all `success`

## F_ok

The exact bytes of the transitive physical measurement scripts are SHA-256-bound before execution and recursively sealed with child evidence. PR #251 is now present in `main`.

## F_gap

Physical Android/Termux execution remains unobserved. Toolchain/environment identity is only partially captured by the lower-level physical gate and is not yet sealed as an independent provenance object.

## F_next

Add an additive environment/toolchain provenance layer that records the exact measurement environment without requiring arbitrary tool versions, seals it with the code-bound execution tree, and preserves `TOKEN_VAZIO` for unavailable optional fields.

`claim_allowed=false`
