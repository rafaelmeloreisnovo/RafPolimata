# Evidence Garden V1 - rigor falsificavel, enterprise, industrial e full-stack

**Lifecycle:** `ACTIVE / PASS_LIMITED_BY_EXECUTION`.  
**Area:** benchmark + evidence + data quality + security + research methodology.  
**Owner roles:** evidence-custodian, quality-assurance, ci-governance.  
**Implementation:** `scripts/evidence_garden.py`, `Benchmark/evidence_garden/`, `configs/evidence-garden-policy.v1.json`.

## 1. Objective

The system does not seek an absolute proof that a benchmark winner is universally superior. It seeks a bounded, reconstructible statement of what was observed, under which factors, with which evidence, which falsifiers remain, and which claim level is still blocked.

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
IMPLEMENTED_UNTESTED != PASS
```

## 2. Station model

A station is a controlled tuple:

```text
station = (source, build_profile, input, size, threads, arch, OS,
           virtualization, device, observer, repetition_policy)
```

Comparisons are valid only inside a declared compatible station family. A cross-station conclusion requires an explicit bridge and uncertainty treatment.

## 3. Four rigor profiles

### academic_falsifiable

Requires a declared hypothesis/null, falsifier, raw samples, effect size, uncertainty interval, predeclared minimum relevant effect, multiple-comparison treatment when several stations are tested, negative results, limitations and independent replication before a strong external claim.

### enterprise

Requires immutable source identity, artifact/input hashes, toolchain identity, audit log, access/secret boundaries, rollback, risk owner, nonconformity and corrective action. It is an auditability profile, not a certification claim.

### industrial

Requires a declared measurement system: timer/calibration, frequency/governor/affinity/thermal/load state where available, repeatability/reproducibility separation, raw tails, measurement-system limitations and controlled interventions.

### fullstack

Traces only the layers actually observed from source to device. Syscall equality does not prove cache equality; PMU equality does not prove identical physical switching; a VM cannot promote physical visibility that its hypervisor does not expose.

## 4. DMAIC + PDCA operating loop

```text
DEFINE  -> claim, CTQ, scope, authority, falsifier
MEASURE -> frozen inputs, baseline, raw observations, measurement envelope
ANALYZE -> variance, residuals, confounders, uncertainty, contradiction
IMPROVE -> smallest reversible intervention with largest expected information gain
CONTROL -> regression gate, receipt, hashes, rollback, drift watch
```

PDCA is used in parallel: plan the station and claim, execute, check against the gate, then act on the smallest verified gap. No sigma level is stated unless defects/opportunities and the statistical model are actually measured.

## 5. Standards alignment - no certification claim

The policy uses external standards as design references, not as a statement of formal conformity or certification.

| Reference | Evidence Garden use |
|---|---|
| ISO 9001 quality-management family | process approach, PDCA, risk-based thinking, corrective action and continual improvement |
| ISO/IEC 27001:2022 | ISMS-style risk treatment, integrity, controlled evidence and continual improvement |
| ISO 8000-61 / ISO 8000-100 | data-quality process, quality metrics, provenance and master-data quality discipline |
| NIST SP 800-218 SSDF v1.1 | secure-development practices and prevention of recurring root causes |
| NIST SP 800-53 Rev.5 | audit/accountability, configuration, assessment, integrity and supply-chain control families |
| RFC 2119 + 8174 | unambiguous MUST/SHOULD/MAY language when normative words are used |
| RFC 3339 | unambiguous timestamps |
| RFC 8785 | canonical JSON when stable hashing/signing of JSON is required |
| IEEE 1012-2024 | verification and validation across system/software/hardware and interfaces |
| ISO/IEC/IEEE 29119 series | software-test process, documentation and test-design structure |

## 6. Claim gate

A benchmark claim MUST be downgraded when its evidence is weaker than its wording. Core gates include exact source, build identity, artifact digest, input identity, raw observations, correctness oracle, statistics, environment envelope, observer separation, replication and physical/device evidence when that scope is claimed.

A single 95% interval that excludes parity at one of many tested stations is not promoted without the declared family-wise/FDR policy. Likewise, a faster median without a minimum-relevant-effect threshold remains an observation, not an engineering superiority claim.

## 7. Urgency and TOKEN_VAZIO closure

`U0` blocks reconstructibility or correctness: source/artifact/input identity, raw evidence or oracle. `U1` blocks rigorous attribution: commands, controls, tails, observer effect, multiple comparisons. `U2` blocks generalization: thread scaling, physical device and independent reproduction. `F_next` is the smallest reproducible action at the highest unresolved urgency.

The process stops only when every field material to the intended claim is determined as PASS, FAIL or an explicitly bounded `TOKEN_VAZIO` outside the authorized scope. A permanent physical limitation may remain `TOKEN_VAZIO`; the claim is narrowed instead of being fabricated.

## 8. BLAKE3 seed case

The first importer is intentionally narrow: it audits the supplied `RMR-BLAKE3-UPSTREAM-COMPARE-V2` archive. It does not infer that the PDF/Markdown benchmark and the V2 C-core archive are the same execution. Their evidence is kept as separate source cuts unless a bridge proves identity.

The imported audit is in `data/evidence/benchmark/blake3_rmr_upstream_v2/audit.json`; the import receipt is `docs/receipts/EVIDENCE_GARDEN_BLAKE3_IMPORT_20260929.md`.
