# Non-Regression Policy v1

This policy protects legal, provenance, social-access and security semantics from silent weakening.

## Hard invariants

1. **Append-only evidence:** receipts, gap records and historical license versions are never rewritten to change past facts. Corrections use `supersedes`.
2. **No upstream erasure:** copyright notices, attribution and license notices required by third-party licenses must not be removed.
3. **No retroactive relicensing of third parties:** a RAFAELIA license change cannot narrow or expand rights already granted by an upstream rights holder.
4. **No guessed gaps:** `TOKEN_VAZIO` can only move to `RESOLVED` with a cited evidence/authority record.
5. **No implicit commercial elevation:** research, educational, community, repository or contribution access cannot silently become commercial authorization.
6. **Exact-scope contracts:** commercial permission must identify covered material/version/commit or a precise deterministic set.
7. **License version immutability:** published license text changes require a new version identifier.
8. **Social-access floor:** a future policy must not silently introduce student registration, literacy tests or socioeconomic proof for the fee-free noncommercial permissions granted by the current version. Any deliberate narrowing requires a new license version, rationale and legal review, and cannot retroactively retract permissions already validly granted where law/contract prevents it.
9. **Data minimization:** additional personal data requires documented purpose, necessity and authority. Convenience alone is insufficient.
10. **MD5 downgrade barrier:** MD5 must never replace a required modern digest or become the sole security/provenance anchor.
11. **Security/rights separation:** technical PASS does not prove ownership/license; license clearance does not prove cryptographic/security properties.
12. **Third-party boundary:** unresolved upstream rights block only the affected claims/operations, not unrelated original modules.

## Required change record

Every legal/provenance policy change should record:

```yaml
change_id: CHANGE-...
from_version: ...
to_version: ...
reason: ...
affected_paths: []
rights_expanded: true|false
rights_restricted: true|false
third_party_impact: NONE|TOKEN_VAZIO|...
data_impact: NONE|TOKEN_VAZIO|...
open_gaps: []
reviewer_or_authority: TOKEN_VAZIO
receipt_ref: TOKEN_VAZIO
rollback_or_supersession: ...
```

## Gate result

- `PASS`: invariants preserved and evidence attached.
- `PASS_WITH_GAPS`: non-blocking gaps explicitly recorded.
- `HOLD`: ownership/license/commercial authority/data necessity is unresolved for the proposed action.
- `FAIL`: an invariant is demonstrably violated.
