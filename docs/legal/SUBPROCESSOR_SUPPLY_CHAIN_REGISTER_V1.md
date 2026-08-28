# Subprocessor & Supply-Chain Register V1

**Status:** `TEMPLATE / FAIL_CLOSED`

## Purpose

Normalize every external dependency/provider under the same evidence gates, regardless of brand size. This register covers software dependencies, cloud/SaaS, code hosting, CI/CD, AI services, observability, communications, payment, support and other suppliers that can affect service security, privacy, licensing, availability or exit.

## Trust rule

```text
brand != assurance
market-share != legal basis
certification badge != scope proof
vendor promise != observed control
```

A provider becomes approved only for the stated service/data/scope after the applicable evidence is closed.

## Register

| ID | Provider/component | Function | Contract owner | Personal-data role | Data categories | Processing/storage/support locations | International transfer basis | Subprocessors | License/terms | Security evidence | Exit/export path | State |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SUP-001 | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` | `TOKEN_VAZIO` |

## Approval gates

For a material production supplier, assess:

1. **Identity/authority** — exact contracting entity and service.
2. **Purpose/minimization** — why the supplier is necessary and what minimum data/code reaches it.
3. **Terms/license** — current contractual/API/software terms and redistribution/use restrictions.
4. **Privacy role** — controller/processor/operator facts, DPA and subprocessor mechanism.
5. **Location/transfer** — storage, backup, telemetry, support and administrative access locations; lawful transfer mechanism where required.
6. **Security** — access model, encryption, incident process, independent evidence/certification scope where actually available.
7. **Supply chain** — important subprocessors/transitive dependencies, provenance and update route.
8. **Availability/recovery** — outage model, backup/export/failover and service discontinuation risk.
9. **Exit/portability** — export format, deletion, key/credential revocation and replacement plan.
10. **Evidence freshness** — contracts, reports and terms have review dates/version identities.

## State model

- `CANDIDATE`: supplier identified, review incomplete.
- `APPROVED_LIMITED`: approved only for stated low-risk/non-sensitive scope.
- `APPROVED`: required gates closed for the stated production scope.
- `DEGRADED`: material control/terms/evidence changed or expired.
- `SUSPENDED`: no new use while issue is resolved.
- `EXITING`: migration/removal in progress.
- `TOKEN_VAZIO`: material evidence absent.

Approval is scope-specific; it is never a permanent endorsement of the entire vendor.

## Change triggers

Re-review on material change to:
- terms/DPA/privacy policy;
- processing location;
- subprocessor chain;
- security incident or material vulnerability;
- acquisition/control change relevant to contract/risk;
- product/service architecture;
- data categories/purpose;
- certification/report expiry;
- price/availability that affects continuity;
- license/version/EOL.

## BigTech normalization

If a service from Google, Microsoft, Amazon/AWS, Apple, GitHub, Cloudflare, Meta, OpenAI, Oracle or another large provider is actually selected, create a row for the **specific contracted service and legal entity**. Do not pre-approve a company family globally.

## Software/component supply chain

For libraries/toolchains/algorithms record separately:
- upstream repository/source;
- pinned version/commit when feasible;
- artifact/hash;
- license/SPDX;
- notices/source obligations;
- patent/standard concerns where applicable;
- vendored vs independently implemented vs system/toolchain reference;
- vulnerability status;
- replacement/deprecation path.

## Approval receipt

A supplier approval receipt should include reviewer, date, exact service/version, chosen scope, unresolved exceptions, expiry/review trigger and evidence references.
