# RafPolimata — Federated Repository Contract v1

**Federated role:** semantic governance, evidence compilation and cross-domain claim routing.  
**Local authority remains:** `docs/AGENTES.md`, canonical protocols, configs, CI gates and repository-specific decision logs.

## Concrete interface

```text
input: atomic claim + source locator + proposed model/test
output: classified claim + proof obligation + target repository + rollback rule
```

RafPolimata does not certify truth by vocabulary, symbolism, document length or cross-domain similarity. Its federated responsibility is to compile a statement into explicit evidence obligations.

## Non-negotiable invariants

1. Separate evidence, method, hypothesis and metaphor.
2. Every claim names its owner repository and source path.
3. `TOKEN_VAZIO` remains valid when tool, dataset, device, log or execution is absent.
4. Semantic convergence cannot replace dimensional, runtime, legal or scientific validation.
5. Agent conflicts are written to the decision log; they are not resolved by silent overwrite.
6. A federated route cannot weaken a stricter local gate.

## Claim compilation record

```yaml
claim_id: stable identifier
statement: atomic statement
class: VERIFIED | TESTED | PARTIAL | DECLARED_BY_AUTHOR | TOKEN_VAZIO | CONTRADICTION | BLOCKED
source:
  repository: owner/name
  commit: sha-or-TOKEN_VAZIO
  path: path-or-TOKEN_VAZIO
proof_obligation:
  command: command-or-TOKEN_VAZIO
  expected_artifact: artifact-or-TOKEN_VAZIO
falsifier: explicit failure condition
target_repository: owner/name
rollback_anchor: commit/tag/hash
```

## Fail-safe

When classification or routing is ambiguous:

- preserve the original statement verbatim;
- emit `TOKEN_VAZIO` or `BLOCKED` with the missing discriminator;
- do not choose a repository based only on theme similarity;
- do not generate implementation claims;
- require a smaller atomic claim.

## Failover

`Matem-tica-` may validate formal mathematical fragments. It cannot replace RafPolimata's legal, semantic, software or cross-repository governance duties. Conversely, RafPolimata cannot mark a mathematical statement proved without the formal repository's proof gate.

## Rollback

Protocol and config changes use a dedicated PR. Rollback restores the prior protocol/config commit and replays its validation report. Decision logs are append-only and must not be erased during rollback.

## Watchdog and temporal refusal

The semantic watchdog checks:

- duplicate claim IDs;
- missing source locators;
- circular repository routing;
- claims promoted without new evidence;
- relative-time words (`latest`, `today`, `current`) without an exact timestamp/commit;
- stale evidence reused as current proof.

## Blind tests

- permute claim-record order and require the same canonical digest;
- remove the source path from one hidden fixture and require `TOKEN_VAZIO`;
- present equivalent wording from different domains and require independent proof obligations;
- inject a contradiction after classification and require demotion to `CONTRADICTION`;
- route a mathematical metaphor and ensure it is not classified as theorem.

## Federated output

```text
F_ok: claims with complete evidence chain
F_gap: missing locators, contradictions and blocked tools
F_next: one atomic proof obligation
rollback_anchor: protocol/config commit
```
