# Evidence Garden V1 — Benchmark as measured evidence, not absolute proof

**Area:** `Benchmark / evidence-governance / runtime measurement`  
**Logical owner:** RafPolimata Benchmark group  
**State:** `IMPLEMENTED / CURRENT-COMMIT-CI-PENDING`  
**Canonical entry:** `Benchmark/Readme.md -> Benchmark/evidence_garden/README.md`  
**Machine contract:** `Benchmark/evidence_garden/stations.v1.json`

Evidence Garden is the reusable experiment layer for cases where two or more implementations, build profiles, runtimes, machines, virtual environments or interventions must be compared without turning a measurement into a stronger claim than the run supports.

The governing invariant is:

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
IMPLEMENTED_UNTESTED != PASS
```

A benchmark receipt can prove that a declared command ran and produced declared observations in a bounded environment. It does not prove universal superiority, physical causality, source equivalence, constant-time behavior, independent replication or visibility of every hardware signal.

## Three execution lanes

The same declared variants are observed through three separate lanes so the observer does not silently redefine the benchmark:

1. **correctness** — captures exit status and hashes stdout/stderr; may compare variant outputs and an optional predeclared reference digest;
2. **observability** — optional `strace`/`perf stat` probes; missing or rejected optional probes remain `TOKEN_VAZIO`;
3. **performance** — rotating interleaved rounds with stdout/stderr suppressed to reduce observer disturbance.

The three lanes are joined by config identity, repository HEAD, command identity and declared artifact SHA-256 values.

## Eight stations

| Station | Purpose | Promotion boundary |
|---|---|---|
| `S0_identity` | config/source/artifact/command identity | identity is not semantic truth |
| `S1_correctness` | executed-case output/exit comparison | case-bounded only |
| `S2_observability` | syscall/PMU-facing probes | only what the runner exposes |
| `S3_performance` | raw interleaved timing samples | run/environment bounded |
| `S4_intervention` | baseline-vs-variant paired deltas | `OBSERVED_UNPROMOTED`; one declared factor is not universal causality |
| `S5_statistics` | median, MAD, CV, p05/p95 and median CI | assumptions recorded; no universal theorem |
| `S6_reproduction` | compare multiple receipts | one receipt stays `PENDING` |
| `S7_claim_gate` | bounded statements + explicit gaps | V1 keeps `claim_allowed=false` |

`stations.v1.json` is the machine-readable inventory.

## Statistical station

For each successful variant the runner records:

```text
n
min
p05
median
mean
p95
max
sample standard deviation
median absolute deviation (MAD)
coefficient of variation (CV)
95% distribution-free order-statistic interval for the median, when n is sufficient
```

For every variant against the declared baseline it records paired round deltas:

```text
Delta_r = T_variant,r - T_baseline,r
```

and reports median delta, sign counts, relative median delta and the same non-parametric median interval over paired deltas.

The interval is an inference under its recorded assumptions; it is not converted into an absolute claim about all machines, builds or future executions.

## Observability probes

V1 supports these optional external probes:

- `strace_full` — full syscall trace (`-f -ttt -T`);
- `strace_summary` — aggregate syscall counts/times;
- `perf_stat` — task-clock/cycles/instructions/branches/branch misses/cache events when the environment permits access.

A virtual runner does **not** imply visibility of every physical wire, transistor, current path or unexposed PMU channel. That boundary remains:

```text
physical_signal_visibility = TOKEN_VAZIO
```

Existing in-process trace primitives under `Benchmark/raf_trace_*` remain separate. Evidence Garden complements them with process/environment receipts; it does not replace or silently merge their evidence scopes.

## Experiment contract

An experiment is declarative JSON. Commands are argv arrays and execute with `shell=False` semantics in the runner.

```json
{
  "schema": "rafpolimata.evidence-garden.experiment.v1",
  "experiment_id": "example-v1",
  "baseline_variant": "upstream",
  "artifacts": [
    {"path": "build/upstream/tool", "required": true},
    {"path": "build/candidate/tool", "required": true}
  ],
  "variants": [
    {
      "id": "upstream",
      "command": ["build/upstream/tool"],
      "factors": {"lto": false}
    },
    {
      "id": "candidate_lto",
      "command": ["build/candidate/tool"],
      "factors": {"lto": true}
    }
  ],
  "correctness": {
    "enabled": true,
    "args": ["--known-vector"],
    "compare_stdout": true,
    "expected_exit": 0
  },
  "observability": {
    "enabled": true,
    "args": ["--known-vector"],
    "probes": [
      {"kind": "strace_summary", "required": false},
      {"kind": "perf_stat", "required": false}
    ]
  },
  "performance": {
    "enabled": true,
    "args": ["--benchmark-vector"],
    "warmup": 3,
    "rounds": 31,
    "order": "rotating_interleaved"
  }
}
```

When a known golden output exists, add `correctness.reference_stdout_sha256`. Without it, cross-variant equality supports equivalence only for the executed case; external accuracy remains `TOKEN_VAZIO`.

## Run locally

```sh
python3 -m unittest -v tests.test_evidence_garden
python3 scripts/evidence_garden.py run \
  --config Benchmark/evidence_garden/demo_experiment.json \
  --out build/evidence-garden/local/receipt.json
```

Compare two or more receipts without inventing a performance verdict:

```sh
python3 scripts/evidence_garden.py compare \
  --out build/evidence-garden/reproduction.json \
  build/evidence-garden/run-a/receipt.json \
  build/evidence-garden/run-b/receipt.json
```

The GitHub workflow `.github/workflows/evidence-garden.yml` runs the framework on Ubuntu 22.04 and 24.04, uploads each receipt and then emits a bounded cross-environment reproduction summary.

## Scaling to many experiments

The intended scaling unit is **one config -> one receipt**. Thousands of experiments should be represented by many small immutable configs and receipts rather than one giant script with hidden branches. A higher-level catalog can later index them by:

```text
experiment_id
source_sha
config_sha
artifact_sha
factor vector
host fingerprint
station states
receipt_sha
```

This makes bit-level identity, statistics, failures and `TOKEN_VAZIO` independently queryable.

## Claim boundary

V1 deliberately emits `claim_allowed=false`. It can establish bounded execution facts and reproduction state, while leaving stronger promotion to an explicit domain-specific claim gate.

```text
benchmark exists        != benchmark executed
benchmark executed      != independent reproduction
same output             != same internal execution
same timing             != same microarchitecture
single-factor metadata  != isolated physical causality
VM observation          != bare-metal physical proof
```

## R3

`F_ok`: machine-readable stations, generic runner, three lanes, paired statistics, optional syscall/PMU probes, cross-receipt comparison and dual-Ubuntu CI are implemented in V1.  
`F_gap`: physical/device stations, calibrated energy, hardware-current visibility and independent-provider replication remain separate evidence gates.  
`F_next`: add domain adapters (for example exact-SHA BLAKE3 upstream/RMR builds) as small experiment configs plus artifact-production steps, without weakening the common receipt contract.
