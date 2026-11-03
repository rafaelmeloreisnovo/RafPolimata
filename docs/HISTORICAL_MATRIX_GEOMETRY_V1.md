# Historical Matrix Geometry V1 — 2025 source → 2026 bounded compiler implementation

**Authority:** RafPolimata owns matrix semantics and low-level compiler code.
**Status:** `IMPLEMENTED_UNTESTED` until exact-head host/ARM CI or local execution.
**Claim:** `claim_allowed=false` for physical geometry, quantum, cosmology, compression, device and hardware.

## Historical anchors (primary user messages; not an automatic mathematical theorem)

- 25–26 March 2025 (BRT/UTC): eight mirrored perspectives, directional expansion, a central composition, three layers, two contextual oppositions.
- 11, 18, 19, 21 May 2025: eight matrices, recursive matrices, knowledge-area correlation, 10x10/four-fractal composition, matrix inside matrix, Voynich reference.
- 29 June: 10^3 plus four fractal/two parity labels and 1000^3 plus forty fractal/twenty parity labels, one ZIPRAF relationship per matrix.
- 14 July: 1000x1000x40f x2p overlaid in 33, reverse Fibonacci reference.

**Sources:** private Drive NOVOexport, `conversations-004..017.json`; never copy raw conversations into this repository. The exact meaning of 'fractal', 'opposition', and '33' remains `TOKEN_VAZIO` until defined.

## Implemented boundaries

- `compiler/raf_matrix_geometry_v1.h`: header-only C99 primitive, no system headers, libc, malloc, syscalls, global state or dynamic allocation. Fixed 8x3x2 slot index; reversible bit-axis mirror of 8 cube corners; integer cube corners/parity; 1..7D mixed-radix offset with overflow rejection; 10^3 and 1000^3 **symbolic offsets**, not memory allocation; Q16 approximation to sqrt(3)/2; XOR fold to three layers, two contexts and one central projection.
- `scripts/historical_matrix_geometry_v1.py`: hosted scoped adapter over the **existing** `scripts/matrix_compose_v1.py`, preserving its 10-bit-cell transport, IFDex, receipts and confined source paths. It requires all 48 typed positions: missing != zero. Maximum 65,536 aggregate input bytes for materialized matrices. A 1000^3 matrix is explicitly rejected as a materialized input.
- `tests/test_historical_matrix_geometry_v1.c` and `tests/test_historical_matrix_geometry_v1.py`: exact index and parity, mirrors, fixed point Q16, overflow/corruption, sorted slot mapping, complete layer/context projections, no false reversibility.
- The original `scripts/matrix_compose_v1.py`, its V1 codec/fixtures and `Apkc/apkc.c` are intentionally unchanged.

## Producer input

```json
{
  "schema": "rafaelia.historical-matrix-geometry.input.v1",
  "topology": {"positions": 8, "layers": 3, "oppositions": 2, "basis": "cube_parity_tetra"},
  "operator": "xor",
  "cells": [
    {"position": 0, "layer": 0, "opposition": 0, "matrix": [[1, 2]]}
  ]
}
```

The JSON above is an **illustrative incomplete fragment** and must fail; all 48 unique cells are mandatory. A real valid fixture supplies positions 0..7 × layers 0..2 × oppositions 0..1 with identical finite shapes. `mirror_axis` is optional (0,1,2), mapping positions by bitwise XOR with an axis bit. The three Q16 scales are metadata, not a deforming matrix resampler.

```sh
python3 -m unittest -v tests.test_matrix_compose_v1 tests.test_historical_matrix_geometry_v1
cc -std=c99 -O2 -Wall -Wextra -Werror -pedantic tests/test_historical_matrix_geometry_v1.c -o /tmp/raf_geo_v1
/tmp/raf_geo_v1
cc -std=c99 -ffreestanding -nostdlib -nostdinc -fsyntax-only -x c compiler/raf_matrix_geometry_v1.h
# Optional host cross-syntax gate (if clang exists):
clang --target=armv7-linux-gnueabihf -ffreestanding -nostdlib -nostdinc -fsyntax-only -x c compiler/raf_matrix_geometry_v1.h
clang --target=aarch64-linux-gnu -ffreestanding -nostdlib -nostdinc -fsyntax-only -x c compiler/raf_matrix_geometry_v1.h
```

Command-line producer takes `input --source-root . --out-dir ...`. It emits the four original Matrix Compose V1 outputs plus `geometry.output.v1.json` and `geometry.receipt.v1.json`. `matrix.output.json` is a lossy aggregate, not reconstruction of the 48 inputs. Original sources remain authoritative.

## Non-promotion gates

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`. No binary/ELF/DEX/APK is validated by adding this header. Existing compiler dispatch is unchanged; this core is compilable through C tooling, not a new language frontend. No claim that sqrt(3)/2 encodes a physical field, that 42 indices are attractors, that classical XOR equals a quantum gate, or that matrices are cosmological data.

**Gaps:** provider CI exact-head; dynamic runtime; ARMv7/AArch64 code generation; Android/Termux device; cross-repository RafGitTools consumer acceptance; independent review.

**Rollback:** close draft PR or revert additive commits. Do not mutate historical shards or legacy V1 producer. Supersede any incorrect finding via a new receipt.
