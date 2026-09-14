# Matrix Compose V1

State: `IMPLEMENTED_UNTESTED` until the workflow for the exact candidate commit passes.

## Contract

`Matrix Compose V1` is the bounded execution core for deterministic N-dimensional `uint8`
matrix overlays. It emits four distinct artifacts:

1. `matrix.output.json` — composed logical matrix;
2. `compose.output.hex` — packed 10-bit-cell stream represented in hexadecimal;
3. `ifdex.v1.json` — Index of Files, Digests, Extents and Cross-layer composition;
4. `receipt.v1.json` — execution receipt with `claim_allowed=false`.

The 10-bit cell is a layout contract, not a general ECC claim:

`7 payload bits + 2 parity bits + 1 extension/MSB bit`.

The unit gate exhaustively checks 256 byte round-trips and all 2,560 possible
single-bit flips across encoded 10-bit cells.

## Operators

- `xor`
- `sum_mod_256`
- `weighted_mod_256`
- `majority_bit`

All layers must have the same rectangular shape. Ragged input and path traversal are rejected.
Binary sources are read only inside `--source-root`, at the declared offset and exact matrix extent.

## Evidence boundary

`SOURCE != IMPLEMENTATION != EXECUTION != EVIDENCE != CLAIM`

A passing hosted CI run proves only the bounded Python implementation and fixture on that exact
commit. It does not prove Android/Termux/Frida/device execution.

F_gap: physical runtime and cross-repository consumer receipts remain `TOKEN_VAZIO`
under `closure=CLOSURE_L12` until a current-artifact runtime/device receipt exists.
