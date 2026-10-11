# RAFAELIA Freestanding Gap Falsifiers V1

Status: `SOURCE_DELTA / FAIL_CLOSED / CLAIM_ALLOWED_FALSE`  
Closure: `CLOSURE_L13`  
Baseline: `PR #429 follow-up after checks-green head 22925ed89413f881904f3bdf246888f30fad19e4`  
claim_allowed: `false`

This gate attacks the remaining uncertainty around the freestanding reconstruction tranche. It does not promote physical runtime, independent reproduction, license completeness, or production readiness.

## H1 / H0

H1: every explicit gap in the changed freestanding reconstruction material is typed, governed by closure, and prevented from becoming an over-broad claim.

H0: at least one changed file contains an untyped `TOKEN_VAZIO`, a gap without closure, a measured claim without source/run/job binding, or an evidence promotion such as `DOCUMENTED -> PASS`.

## Fatal falsifiers

| ID | Probe | Invalidates if |
| --- | --- | --- |
| FGF-01 | `TOKEN_VAZIO` scan | token has no governing `CLOSURE_*` |
| FGF-02 | token classifier | token cannot be typed as license, device, provider, reproduction, artifact or log gap |
| FGF-03 | claim scan | `claim_allowed=true` appears in changed material |
| FGF-04 | promotion scan | documentation language equates `DOCUMENTED` with `PASS` |
| FGF-05 | receipt binding scan | measured receipt lacks `head_sha`, `run` or `job` binding |

## Command

```sh
python3 tools/validate_freestanding_gap_falsifiers.py \
  --repo . \
  --changed-since <base-sha> \
  --output ci/reports/freestanding-gap-falsifiers.json \
  --summary --strict
```

## Limits

This gate is a lexical/source-side falsifier. It is not a full YAML AST, legal review, physical ARM execution, independent reproduction, or security certification.

## R3

F_ok:
- the next uncertainty-reduction layer is source-side, reproducible and fail-closed.

F_gap:
- physical ARM32/ARM64 execution remains open;
- independent reproduction remains open;
- full license map remains open;
- raw logs remain bounded to provider artifacts unless embedded later.

F_next:
- run the workflow at exact head;
- preserve the JSON artifact;
- promote only the scoped result of this falsifier.
