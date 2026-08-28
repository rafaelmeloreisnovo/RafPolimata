# RAFCODE IP & License Policy V1

**Status:** `SCOPED_OWNER_POLICY / COUNSEL_REVIEW_REQUIRED`  
**Applies to:** original RAFAELIA/RafPolimata material expressly included by a versioned license-scope manifest.

## 1. Scoped decision

For newly authored original modules whose intended public permission is noncommercial research/study/modification/distribution with commercial rights reserved, the preferred standardized route is:

```text
PolyForm-Noncommercial-1.0.0
+ explicit Required Notice/provenance
+ versioned LICENSE_SCOPE manifest
+ separate written commercial license/service agreement
+ third-party licenses preserved
```

This policy does **not** change the license of the repository as a whole. Existing files with their own SPDX/license declarations and all third-party material remain under their applicable terms. Root-wide licensing stays `TOKEN_VAZIO` until authorship, third-party inventory and compatibility are closed.

## 2. Why standardized rather than bespoke

A standardized source-available license reduces drafting ambiguity and gives downstream users a stable identifier. PolyForm Noncommercial 1.0.0 is used here only where its actual permissions match the selected scope. It is not represented as OSI-approved open source.

Authorial protection is layered through provenance, notices, scope manifests, signatures/hashes and commercial contracts rather than by altering standardized license language.

## 3. RAF Hash Fabric V1 decision

`native/raf_hash_fabric_v1/` receives a module-scoped `LICENSE.md` pointing to PolyForm Noncommercial 1.0.0 and a `LICENSE_SCOPE_V1.json` enumerating the original files covered at this cut.

Commercial/production use outside the noncommercial grant requires a separate written commercial agreement. Third-party material that may later be added to the module is excluded from the module grant and must retain its upstream license.

## 4. License routing

| Material | Route |
|---|---|
| Original material listed in a PolyForm license-scope manifest | PolyForm Noncommercial 1.0.0 for permitted noncommercial use; separate commercial agreement for uses outside the grant |
| Existing material with another SPDX/license | Existing license controls |
| Copied/vendorized third-party source | Upstream license + notices + compatibility gate |
| Independently written implementation of a public algorithm/spec | Project code license may apply to original expression; algorithm/spec/patent/trademark rights remain separate |
| Customer confidential code/data | Customer/contract rights; no absorption into project IP absent express written grant |
| Generated output | Governed by source/generator/data/content rights; generation alone does not establish ownership |
| Contributions | Require provenance and contribution policy/DCO/CLA review before relying on commercial relicensing rights |

## 5. BLAKE3 boundary

The official BLAKE3 repository publishes its implementations under a permissive multi-license expression including CC0-1.0, Apache-2.0, and Apache-2.0 with LLVM exception. If upstream source is copied/vendorized, preserve an applicable upstream option and notices. Independently written RAF adapters/scheduling may have their own copyright license, but no project claim extends to ownership of the BLAKE3 algorithm, specification, upstream source or marks.

## 6. Commercial license minimums

A production/commercial Order should identify:

- parties and authority;
- artifact/version/hash and covered modules;
- deployment/users/devices/instances/territory as relevant;
- source/binary/hosting/modification rights;
- redistribution/sublicensing boundaries;
- support, maintenance and SLA;
- confidentiality/trade-secret treatment;
- security/vulnerability obligations;
- data-protection roles, DPA and transfer schedule;
- third-party components/notices;
- fees/taxes/payment;
- warranty/indemnity/liability allocation;
- termination/transition/data return-deletion;
- governing law/dispute process;
- audit/evidence rules.

Unfilled fields remain `TOKEN_VAZIO`; rights are not silently inferred.

## 7. No false exclusivity

Copyright and contract do not create ownership of facts, mathematical ideas, public standards, third-party expression, independently developed material, or rights not recognized by applicable law. Precise provenance is stronger than exaggerated ownership claims.

## 8. Commercial release gate

No artifact may be marked `COMMERCIAL_CLEAR` unless:

```text
AUTHORITY
+ OWNERSHIP/PROVENANCE
+ THIRD_PARTY_INVENTORY
+ LICENSE_COMPATIBILITY
+ SECURITY_GATE
+ PRIVACY/DPA_GATE when applicable
+ COMMERCIAL_ORDER
+ RELEASE_RECEIPT
```

are closed for that exact artifact/version.

## 9. Counsel gate

This policy and related commercial templates organize engineering and contracting. Qualified counsel should review material production deployments, especially patent, consumer, tax, employment, export/sanctions, regulated-sector, insurance, dispute and cross-border matters.
