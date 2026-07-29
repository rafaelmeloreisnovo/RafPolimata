# Voynich Reproducible Toolkit — operational draft V1

This directory materializes the **reproducible-method layer** of the protected
`Voynich implementation` object. It does **not** claim a decipherment, authorship,
linguistic truth, physical attractor, or validated interpretation.

```text
status        = IMPLEMENTED_SYNTHETIC_TESTABLE
claim_allowed = false
real_dataset  = TOKEN_VAZIO_CONTRACT_PENDING
```

## Components

- `ToroidalMap`: canonical data + metadata → exact seven-coordinate `uint64` point on `T^7`;
- `CHState` / `update_ch`: EMA updates with `alpha=0.25` by default;
- `trajectory_signal`: deterministic `Psi(t)` from seven-dimensional trajectories;
- pure-Python radix-2 FFT, `S(omega)`, declared cardioid window `H_cardio`, and Pearson `R`;
- SHA-256, CRC-32, Merkle root, and optional FNV-1a 64-bit checksum;
- `entropy_milli` and the explicitly provisional `bits_geom` capacity proxy;
- `iterate_map`, Floyd cycle detection, and independent hash-table detection;
- fixed-seed synthetic experiment and JSON report;
- optional deterministic conversation chunker plus repository index;
- unit tests in `pytest`.

## Reproduce

From this directory:

```bash
python -m venv .venv
. .venv/bin/activate
python -m pip install -e . pytest
pytest
python scripts/run_synthetic.py \
  --seed 144000 \
  --steps 128 \
  --alpha 0.25 \
  --output reports/synthetic_report.json
sha256sum reports/synthetic_report.json
```

Termux uses the same commands after installing Python:

```bash
pkg install python
python -m pip install -e . pytest
pytest
python scripts/run_synthetic.py --seed 144000
```

## Contracts and limitations

1. Canonical ingestion rejects non-finite floats and unsupported object types.
2. Exact toroidal identity is the seven-element `q64` tuple; floating angles are views.
3. `H_cardio[n]=0.5*(1-cos(2*pi*n/(N-1)))` is a **declared convention** awaiting domain validation.
4. `bits_geom=log2(occupied quantized cells)` is a capacity proxy, not a universal information theorem.
5. Cycle detection operates on a declared quantization; a detected cycle is a numerical candidate.
6. Hashes prove byte identity/integrity, not semantic truth.
7. Synthetic PASS does not validate the historical Voynich manuscript or any interpretation.

## Minimum completion gate

```text
G1 deterministic ingestion
G2 C/H alpha sensitivity
G3 synthetic spectral recovery
G4 integrity mutation tests
G5 cycle controls with multiple seeds
G6 frozen real-data contract
G7 out-of-sample experiment
G8 independent reproduction
```

Only G1–G5 are represented here; G6–G8 remain `TOKEN_VAZIO` until receipts exist.
