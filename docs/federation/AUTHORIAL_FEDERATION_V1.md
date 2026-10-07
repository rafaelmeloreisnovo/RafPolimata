# Authorial Federation V1 — RafPolimata

**State:** `SOURCE_ARCHIVE / RIGHTS_GATED / NOT_IN_CANONICAL_BUILD`  
**Base:** `main@32220b1ce8353566835fdee2301657ca79366da2`  
**Governance closure:** `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY`

## Read this first

This route collects only material whose authorship and governing rights were observed strongly enough to permit copying. Imported source is **not** automatically part of RafPolimata's build, ABI, release, or PASS state.

Canonical invariant:

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`  
`TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY) != 0`  
`IMPORTED != INTEGRATED != TESTED != PASS`

## Human / AI route

1. Read `data/federation/authorial_sources.v1.json`.
2. Resolve the source repository + exact ref + original blob before modifying a copied file.
3. Read the closest `LICENSE.source.*` and provenance file.
4. Treat `federation/authorial/**` as a source archive unless a later receipt explicitly promotes a module.
5. Never infer that repository ownership proves authorship.
6. Never collapse GPL-2.0-only, GPL-3.0, custom LicenseRef, MIT-like text, or missing rights into one project license.

## Current routing

| Source | Rights/authorship state | Action |
|---|---|---|
| Vectras `conjunto_de_conceitos` | explicit Rafael clean-room declaration + GPL-2.0-only | copied as source archive |
| Rafaelia_Private `Ordernar/ra/aether_core.h` | explicit Rafael author + subtree license | copied as source archive |
| PCR_Rafaelia_Code_seed | mixed Magisk derivative; RAFAELIA additions separately identified | routed to RafGitTools |
| GaiaPhiRafcode | no indexed file-level rights evidence observed | pointer only / blocked |
| “Rafcodifi” | exact identity unresolved; Rafcodephi SDK is only a plausible candidate | pointer only / blocked |

## Important technical boundaries

The imported Vectras clean-room bundle contains direct Linux syscall wrappers. That makes it useful as low-level authored reference, but **not** eligible for any RafPolimata gate that requires a no-syscall core.

The imported `aether_core.h` uses standard headers and `memcpy`; it is likewise not a freestanding L0 promotion candidate in its current form.

No build file outside the imported source archive was changed by this federation step.

## Destination semantics

`federation/authorial/` answers **where did this authored source come from?**

It does not answer **what should production use?** Production promotion needs a separate compatibility, dependency, build, test and evidence gate.
