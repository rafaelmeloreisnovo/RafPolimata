# TOKEN_VAZIO & Gap Contract v1

`TOKEN_VAZIO` is an explicit epistemic state: required information is not yet supported by sufficient evidence. It is never equivalent to zero, false, approval, absence of risk, waiver, public domain, ownership, or permission.

## Canonical record

```yaml
gap_id: GAP-...
state: TOKEN_VAZIO # TOKEN_VAZIO|DISPUTED|RESOLVED|SUPERSEDED
observed_at: RFC3339
scope: path/module/claim/contract
missing_fact: "..."
why_unknown: "..."
blocked_claims: []
risk: LOW|MEDIUM|HIGH|P0_HOLD
next_verifiable_action: "..."
evidence_refs: []
authority_required: "..."
resolver: TOKEN_VAZIO
resolved_at: TOKEN_VAZIO
resolution_receipt: TOKEN_VAZIO
supersedes: TOKEN_VAZIO
```

## Invariants

- Never fill an unknown author, owner, license, implementation, signature, algorithm identity, consent, contract authority or provenance source by inference alone.
- A gap may be resolved only by append-only evidence that names the resolving fact and authority.
- Resolution creates a new record/receipt; the original gap remains historically visible.
- Contradictory evidence changes state to `DISPUTED`; it does not silently select a preferred answer.
- `TOKEN_VAZIO` in a commercial-authorization field means **commercial permission is not established**.
- `TOKEN_VAZIO` in a third-party license field means **do not relicense or redistribute that component under RAFAELIA terms until resolved**, except to the extent a separately verified upstream grant already permits the contemplated act.
- `TOKEN_VAZIO` in a security claim means the claim is blocked, not failed or passed.

## Gap classes

- `GAP-OWNERSHIP`: author/titular/authority unresolved.
- `GAP-LICENSE`: applicable license or notice unresolved.
- `GAP-ORIGIN`: upstream source/commit/path unresolved.
- `GAP-ALGO`: algorithm/primitive identity unresolved.
- `GAP-CONTRACT`: assent/scope/signature/party authority unresolved.
- `GAP-DATA`: personal-data purpose/necessity/legal basis unresolved.
- `GAP-EVIDENCE`: receipt/hash/signature/test missing.
- `GAP-SECURITY`: security property unverified.

## Claim gate

A claim is allowed only when every fact required for that claim is either evidenced or explicitly non-required. Gaps unrelated to a claim may remain open. This prevents both overblocking and invented certainty.
