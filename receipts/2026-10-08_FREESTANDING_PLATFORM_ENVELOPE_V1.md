# Receipt — Freestanding Platform Dependency Envelope V1

Status: `AUDIT`
Date: `2026-10-08`
Repository: `rafaelmeloreisnovo/RafPolimata`
Base observed: `main@432d884c76f0229ce96c0bb19ca5004d885a76b6`
Branch: `architecture/freestanding-platform-envelope-v1-20261008`

## Intent

Create a low-friction route for Android, Linux, macOS, Windows and iPhone platform-dependent development across C, ASM, Java, shell, Python, Rust, C++ and Kotlin Multiplatform while preserving freestanding/low-level boundaries.

## Delta

| Path | Kind | Summary |
|---|---|---|
| `spec/FREESTANDING_PLATFORM_DEPENDENCY_ENVELOPE_V1.md` | `REFERENCE` | Human-readable platform/language/dependency matrix and promotion checklist |
| `schemas/freestanding-platform-envelope.v1.schema.json` | `SCHEMA` | Machine-readable envelope for platform/language/ABI/evidence classification |
| `docs/INDEX.md` | `ROUTE` | Adds the new envelope to the canonical navigation route |

## Evidence

| Gate | State | Reason |
|---|---|---|
| Repository readback | `PASS` | GitHub API read of repository metadata, `AGENTS.md`, `docs/AGENTES.md`, `docs/INDEX.md`, `README.md` |
| Branch isolation | `PASS` | Work was placed on non-main branch |
| Schema syntax | `NOT_RUN` | No local clone/test runner available in this scratch session |
| Document-governance check | `NOT_RUN` | No local clone/test runner available in this scratch session |
| Android device runtime | `TOKEN_VAZIO` | No APK/ELF/install/logcat chain executed |
| iOS/macOS/Windows/Linux build | `TOKEN_VAZIO` | No platform toolchain execution performed |

## Claim boundary

This delta defines a route and schema. It does not prove build success, app runtime, device execution, package validity, signing, installation or low-level equivalence.

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != PASS
```

## Rollback

Delete the three added files and remove the new route from `docs/INDEX.md`. No generated outputs or binary artifacts are changed.

## R3

`F_ok`: route and schema materialized for cross-platform low-level work.
`F_gap`: validation gates were not run in this session; runtime/device evidence remains absent.
`F_next`: add one minimal manifest instance for a real target and run the smallest corresponding gate.
