# ZIPRAF Hardware Service V1 — Berne / IP / Provenance Profile

**Document state:** `ENGINEERING_LEGAL_PROFILE / QUALIFIED_REVIEW_REQUIRED_FOR_PRODUCTION`  
**Purpose:** bind authorship/provenance/license facts to source without pretending that source comments create rights by themselves.

## 1. Header model

Original project files may use a compact header such as:

```text
Copyright (c) 2026 Rafael Melo Reis.
RAFAELIA / ZIPRAF Hardware Service V1 original integration material.
License: see the module LICENSE_SCOPE / repository legal route.
Third-party algorithms, specifications, names and source retain their own rights.
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM.
```

The header is evidence/orientation. It is not a substitute for the actual license, provenance record, contract, applicable statute, signature or trusted timestamp.

## 2. Berne principle

The Berne framework is useful precisely because copyright protection is not supposed to depend on a filing formality. The engineering consequence is: preserve authorship and provenance rigorously, but do not falsely state that a special header, hash, registry entry or ZIPRAF wrapper is what creates copyright.

## 3. Brazil software boundary

Brazilian software protection has its own statute and uses the copyright/literary-work regime with specific rules. Protection is independent of registration. A repository receipt can strengthen factual evidence about chronology and exact bytes; it does not transform an unprotectable idea, method or mathematical concept into copyrighted expression.

## 4. What belongs to which legal layer

```text
original source expression/documentation -> copyright + chosen license
algorithm/specification identity         -> upstream/specification boundary
third-party copied/adapted source        -> upstream license/notices
brand/name/logo                           -> trademark/unfair-competition analysis
technical invention                      -> patent analysis when applicable
secret material                          -> confidentiality/trade-secret controls
personal data                            -> privacy/data-protection controls
commercial deployment                    -> contract + sector/export/compliance review
```

No layer silently grants rights held by someone else.

## 5. Cryptography rule

SHA, BLAKE, ChaCha, Ed25519, AES and related names in the module are compatibility/reference identities. A compatible implementation must preserve the specified behavior and must record whether its source is independent, copied, adapted or linked to an upstream implementation.

The project never claims authorship of third-party cryptographic algorithms merely because it schedules, dispatches, benchmarks or wraps them.

## 6. Evidence chain

Recommended append-only chain:

```text
source commit
-> exact file/blob identity
-> module inventory/license scope
-> build artifact hash
-> test/KAT receipt
-> hardware receipt where relevant
-> release manifest/signature where adopted
-> superseding receipt for corrections
```

Hash = byte identity/integrity evidence. Hash != proof of authorship, legal validity, safety or semantic truth.

## 7. Legal pack relationship

This profile is subordinate to the existing RafPolimata legal pack, including the IP/license policy, third-party crypto register, security assurance, privacy/data and supply-chain documents. Any conflict is escalated rather than silently resolved.

## 8. References

- WIPO — Berne Convention summary, Article 5 principles: https://www.wipo.int/en/web/treaties/ip/berne/summary_berne
- Brazil — Lei 9.609/1998 (software): https://www.planalto.gov.br/ccivil_03/leis/l9609.htm
- Brazil — Lei 9.610/1998 (copyright): https://www.planalto.gov.br/ccivil_03/leis/l9610.htm
- Existing repository route: `docs/legal/README.md`
- Existing crypto register: `docs/legal/THIRD_PARTY_CRYPTO_LICENSE_REGISTER_V1.md`

This file is an engineering/legal profile, not individualized legal advice or a certification of enforceability.
