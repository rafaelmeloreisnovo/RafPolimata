# RAFAELIA Legal & Service Assurance Pack V1

**Status:** `REFERENCE / CONTRACT_TEMPLATE / AUDIT_REQUIRED`  
**Area:** legal, commercial, privacy, security, supply-chain  
**Owner:** `human-authorizer` with `security-license` and privacy review  
**Scope:** service contracting and original RAFAELIA/RafPolimata IP only; third-party rights remain governed by their own licenses.

## Purpose

This directory turns legal/security promises into a versioned assurance system. It does not claim automatic compliance, certification, immunity, or legal advice. A contract becomes effective only when the parties, scope, jurisdiction, prices, processing roles, subprocessors and transfer mechanism are actually completed and executed.

## Contract and assurance stack

1. `RAFCODE_IP_LICENSE_POLICY_V1.md` — scoped IP/license policy and commercial boundary.
2. `MASTER_SERVICES_AGREEMENT_TEMPLATE_V1.md` — master commercial relationship.
3. `STATEMENT_OF_WORK_TEMPLATE_V1.md` — project scope, acceptance and delivery evidence.
4. `DPA_LGPD_GDPR_V1.md` — controller/processor obligations and privacy operations.
5. `INTERNATIONAL_DATA_TRANSFER_SCHEDULE_V1.md` — LGPD/ANPD and GDPR transfer mechanism routing.
6. `SECURITY_ASSURANCE_SCHEDULE_V1.md` — measurable security commitments, incident handling and evidence.
7. `SUBPROCESSOR_SUPPLY_CHAIN_REGISTER_V1.md` — vendor/subprocessor register and approval gates.
8. `PRIVACY_NOTICE_TEMPLATE_V1.md` — public transparency/data-subject notice template.
9. `THIRD_PARTY_CRYPTO_LICENSE_REGISTER_V1.md` — crypto algorithm/upstream/license boundaries.
10. `LEGAL_CONTROL_MATRIX_V1.json` — machine-readable fail-closed control state.

The RAF Hash Fabric module itself uses `native/raf_hash_fabric_v1/LICENSE.md` + `LICENSE_SCOPE_V1.json` for its original material at this cut. The repository root is not automatically relicensed.

## Seven assurance pillars

`IDENTITY -> IP/LICENSE -> DATA/PRIVACY -> SECURITY -> SUPPLY_CHAIN -> INCIDENT/RECOVERY -> EVIDENCE/AUDIT`

Each pillar uses the same epistemic states as the repository: `REFERENCE`, `IMPLEMENTED`, `PASS`, `FAIL`, and `TOKEN_VAZIO`. No legal or security claim becomes `PASS` merely because a template exists.

## International baseline

The pack is structured to support, where applicable:

- Brazil LGPD (Lei 13.709/2018);
- ANPD Resolution CD/ANPD 19/2024 for international transfers and standard contractual clauses;
- EU GDPR, including controller/processor duties, security, accountability and Chapter V transfers;
- EU SCCs under Commission Implementing Decision (EU) 2021/914;
- NIST SSDF SP 800-218 as secure-development reference;
- SLSA and OpenSSF Scorecard as supply-chain/provenance references;
- OWASP ASVS as an application-security verification reference;
- ISO/IEC 27001 and ISO/IEC 27701 as management-system references when actually adopted/audited.

Reference alignment is not certification.

## Critical transfer rule

When ANPD standard contractual clauses are the selected transfer mechanism, use the official ANPD text integrally and without modification. This repository stores routing, annex data, controls and evidence; it must not silently rewrite an official mandatory clause set.

For EU transfers relying on the 2021/914 SCCs, select the correct module(s), complete the applicable annexes, assess the destination/transfer risk where required, and bind supplementary technical/organizational measures without contradicting the SCCs.

## BigTech / supplier normalization

No provider receives a trust shortcut because of brand or scale. Cloud, code-hosting, AI, CI, observability, payment and communications suppliers all pass the same gates: purpose, role, data categories, location, transfer basis, security evidence, license/terms, subprocessor chain, exit/portability and incident obligations.

A company name is not evidence. Vendor status remains `TOKEN_VAZIO` until the actual service, contract and processing facts are recorded.

## Legal boundary

These documents are engineering/legal templates. Before production use, obtain qualified legal review for the chosen jurisdiction, consumer/sector rules, tax, employment, export controls, sanctions, regulated data, dispute forum, insurance and liability structure.
