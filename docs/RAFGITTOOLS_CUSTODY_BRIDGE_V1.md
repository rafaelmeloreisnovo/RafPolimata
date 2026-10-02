# RafGitTools Custody Bridge V1

Status: `IMPLEMENTED_UNTESTED` until exact-head CI executes this branch.

RafPolimata consumes custody envelopes produced by RafGitTools. It may validate structure, classify uncertainty/gaps and emit its own consumer receipt. It does not inherit source, provider or execution authority from the producer.

## Invariants

- `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`
- `TOKEN_VAZIO != 0`
- `IMPLEMENTED_UNTESTED != PASS`
- `producer_receipt != consumer_validation`
- `capability_name != secret_value`

## Capability labels

Only non-sensitive labels may cross the bridge. Presentation is canonicalized by RafGitTools as `upper(first) + lower(rest)`, for example `pat_eNvir -> Pat_envir`. This is display metadata only. Provider Secret names and PAT values remain outside the envelope.

## Consumer sequence

`bridge envelope -> fail-closed validation -> uncertainty/gap classification -> consumer receipt -> provider/runtime readback when required`

`claimAllowed=false` is mandatory in V1.

## Files

- `configs/rafgittools-custody-consumer.v1.json`
- `scripts/validate_rafgittools_custody_bridge.py`
- `tests/test_rafgittools_custody_bridge.py`
- `examples/rafgittools-custody-bridge.example.json`

The bridge does not replace the existing Internal Custody Ledger; it supplies a typed cross-repository input boundary for it.
