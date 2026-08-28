# Brazilian Normative Basis — reference map v1

**Purpose:** engineering/legal-reference map for repository governance. It is not a legal opinion and does not replace fact-specific review by qualified counsel.

Verified against official federal sources on 2026-08-28.

## Constitutional layer

**Constituição da República Federativa do Brasil de 1988, art. 5º, XXVII.** Recognizes the author's exclusive right to use, publish or reproduce the work, for the period fixed by law.

Repository consequence: authorship/titularity must still be evidenced; the constitutional rule is not a substitute for a chain of title or third-party clearance.

## Software layer

**Lei nº 9.609/1998 (Lei do Software).**

- Art. 2º: software intellectual-property protection follows the copyright regime subject to the Software Law; §3 states that protection does not depend on registration.
- Art. 6º, II: provides a statutory didactic quotation exception under its own conditions. This is independent from the broader voluntary educational permission granted by the RAFAELIA RCNC license.
- Art. 7º: commercialized software license/fiscal/support media must state the technical-validity period of the version where applicable.
- Art. 9º: software use in Brazil is the object of a license contract; the sole paragraph addresses proof of regular use where a contract is absent.
- Arts. 10–11: contain additional rules for commercialization rights of externally originated software and technology-transfer contracts.

Repository consequence: the custom RCNC license is structured as an express software/use permission for covered original material; commercial authorization is separated into an exact-scope instrument. Transfer-of-technology or other fact patterns may require additional treatment.

## Copyright layer

**Lei nº 9.610/1998 (Lei de Direitos Autorais).**

- Art. 22: moral and patrimonial rights belong to the author over the created work, subject to the law.
- Art. 28: recognizes the author's exclusive patrimonial right to use, enjoy and dispose of the literary, artistic or scientific work.
- Art. 29: enumerated uses depend on prior and express authorization, subject to the statute and applicable exceptions/limitations.
- Art. 31: different modes of use are independent; authorization for one does not automatically extend to another.

Repository consequence: commercial authorizations should enumerate scope rather than rely on an ambiguous blanket statement.

## Personal-data layer

**Lei nº 13.709/2018 (LGPD), consolidated text.**

- Art. 6º: purpose, adequacy, necessity/minimization, transparency, security, prevention, non-discrimination and accountability principles, among others.
- Art. 7º: personal-data processing requires an applicable legal basis; consent is only one possible basis.
- Art. 8º: where consent is the basis, the statute sets evidence, specificity and withdrawal requirements.
- Art. 14: processing personal data of children and adolescents must serve their best interests; the article contains specific child-data safeguards and prohibits conditioning covered activities on data beyond what is strictly necessary.

Repository consequence: `phone-only-if-sufficient` is a minimization design rule, not a universal legal maximum. If no personal-data contact is necessary, collect none; if additional data is legally/operationally necessary, document purpose, necessity and legal basis. Student data should not be placed in public provenance receipts.

## Interpretation gates

- `LAW_TEXT_VERIFIED`: above federal references checked against official sources on 2026-08-28.
- `CASE_LAW_REVIEW`: TOKEN_VAZIO.
- `CONTRACT_ENFORCEABILITY_FOR_SPECIFIC_TRANSACTION`: TOKEN_VAZIO until parties, ownership chain, purpose, jurisdiction and assent are known.
- `INPI_REGISTRATION_OR_TECH_TRANSFER_NEED`: TOKEN_VAZIO until transaction facts are known.
- `TAX/CONSUMER/EDUCATION-SECTOR_RULES`: TOKEN_VAZIO until the actual deployment is known.

The repository must not convert a statutory reference into an absolute statement that any particular claim, registration, priority, restriction or remedy is guaranteed.
