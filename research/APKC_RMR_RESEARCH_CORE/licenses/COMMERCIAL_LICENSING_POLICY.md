# Commercial Licensing Policy

**Coupling ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`  
**Status:** `POLICY / NO COMMERCIAL LICENSE GRANTED`  
**Review:** `BRAZIL_AND_TARGET_JURISDICTION_COUNSEL_REQUIRED`

## 1. Core rule

Commercial use is not authorized by downloading, cloning, forking, communicating, filing a request, receiving an acknowledgment, funding research or opening negotiations.

\[
communication \ne authorization \ne executed\_license
\]

Commercial rights exist only after a separate written agreement is signed by authorized representatives.

## 2. Commercial use includes

Subject to the controlling research license, commercial use includes use primarily directed toward commercial advantage or monetary compensation, including:

- incorporation into a paid product or service;
- SaaS, API, hosted access or managed service;
- internal commercial R&D, product validation or procurement evaluation;
- paid consulting, integration, training or support;
- sale, sublicense, OEM, bundling or distribution for business advantage;
- use to reduce costs, increase revenue, satisfy a commercial contract or obtain regulated approval;
- deployment by or for a commercial affiliate.

## 3. Initial communication package

The applicant must provide:

- legal name, registration, beneficial-control structure and contacts;
- intended use, architecture, territories, users and deployment scale;
- source and binary integration map;
- safety and regulatory classification;
- personal/sensitive data flows;
- security model and threat assessment;
- complete SBOM and third-party license inventory;
- requested patent, support, maintenance and trademark scope;
- expected revenue, transaction volume or internal value;
- proposed insurance limits and audit budget;
- subcontractors, cloud providers and downstream distributors.

Submission does not obligate the licensor to negotiate or grant a license.

## 4. Due diligence and costs

Before any commercial authorization, the applicant may be required to fund, through an agreed advance or escrow:

- Brazilian and foreign legal review;
- intellectual-property and third-party clearance;
- technical reproducibility audit;
- secure-build and supply-chain audit;
- privacy and data-protection assessment;
- sector-specific regulatory analysis;
- penetration testing and incident-response review;
- certification, laboratory, travel, translation, filing, tax and insurance costs.

Unless the signed engagement says otherwise, these evaluation costs are non-refundable and do not guarantee a license.

## 5. Required commercial agreement sections

A commercial agreement must address at least:

1. parties and authority;
2. licensed files, versions, commits and hashes;
3. field of use, territory, term and deployment limits;
4. source, binary, SaaS and distribution rights;
5. patent scope;
6. trademarks and attribution;
7. fees, royalties, minimums, reporting and taxes;
8. audit rights and records;
9. support, maintenance, updates and end-of-life;
10. security, privacy and incident notification;
11. acceptance tests and service levels;
12. regulated/high-risk requirements;
13. export, sanctions and anti-corruption compliance;
14. confidentiality and publication;
15. warranties and disclaimers;
16. indemnities;
17. liability allocation;
18. insurance;
19. suspension, termination and post-termination duties;
20. escrow, continuity and disaster recovery where applicable;
21. governing law, venue or arbitration;
22. assignment, change of control and subcontracting;
23. notices and order of precedence.

## 6. Risk-tiered liability model

No universal US$1 or US$5 cap is promised. A flat nominal cap can be ineffective or unlawful under mandatory law and does not reflect the risk of medical, industrial or critical systems.

The commercial contract should select a tier after audit:

| Tier | Example | Default negotiation baseline |
|---|---|---|
| R0 | noncommercial research | “as-is”, maximum exclusion allowed by law |
| R1 | low-risk internal commercial tool | direct damages capped near fees paid in prior 12 months |
| R2 | regulated or materially consequential system | negotiated higher cap tied to fees, insurance and verified controls |
| R3 | life, health, critical infrastructure or catastrophic exposure | no standard license; bespoke certification, insurance, escrow and governance |

Any cap must preserve mandatory-law exceptions and negotiated carve-outs, commonly including fraud, intentional misconduct, gross negligence where non-excludable, death/personal injury where non-excludable, confidentiality, data protection, unpaid fees and intellectual-property obligations.

All numbers and carve-outs remain `TOKEN_VAZIO_COUNSEL_REVIEW` until inserted into a signed agreement.

## 7. Audit and enforcement

The commercial licensee must permit pre-license, periodic and event-driven audits proportionate to risk, preserve evidence for at least five years or longer when law/contract requires, and provide reproducible receipts tying:

```text
source → commit → build → artifact → deployment → incident → rollback
```

Remedies, liquidated damages, revenue-based amounts, legal fees and injunctive relief must be defined in the signed agreement and reviewed for proportionality and mandatory law. No criminal liability is created by private contract.

## 8. No automatic conversion

Academic publication, nonprofit status, public funding, government participation, contribution, bug reporting or citation does not automatically create commercial rights beyond the controlling research license.
