# International Data Transfer Schedule V1

**Status:** `CONTRACT_TEMPLATE / FAIL_CLOSED`  
**Scope:** cross-border transfers and remote access to personal data where LGPD and/or GDPR Chapter V applies.

> This schedule routes the legal mechanism and technical evidence. It does not replace mandatory official clauses. Where ANPD standard contractual clauses are selected, attach the official text integrally and without alteration. Where EU SCCs 2021/914 are selected, use the applicable official module(s) and complete the required annexes.

## 1. Transfer identity

- Transfer ID: `TOKEN_VAZIO`
- Exporter/legal entity: `TOKEN_VAZIO`
- Importer/legal entity: `TOKEN_VAZIO`
- Exporter role: `TOKEN_VAZIO`
- Importer role: `TOKEN_VAZIO`
- Origin country/region: `TOKEN_VAZIO`
- Destination country/region: `TOKEN_VAZIO`
- Storage location(s): `TOKEN_VAZIO`
- Remote-support/access location(s): `TOKEN_VAZIO`
- Onward transfers: `TOKEN_VAZIO`

## 2. Data and purpose

- Data-subject categories: `TOKEN_VAZIO`
- Personal-data categories: `TOKEN_VAZIO`
- Sensitive/special-category data: `TOKEN_VAZIO`
- Frequency/volume: `TOKEN_VAZIO`
- Purpose: `TOKEN_VAZIO`
- Retention: `TOKEN_VAZIO`
- Systems/services involved: `TOKEN_VAZIO`

## 3. Brazil / LGPD route

Select only after factual/legal review:

- [ ] Adequacy decision applicable.
- [ ] ANPD standard contractual clauses under Resolution CD/ANPD 19/2024.
- [ ] ANPD-approved specific contractual clauses.
- [ ] Binding corporate rules approved under the applicable ANPD framework.
- [ ] Other lawful LGPD mechanism documented with legal basis.
- [ ] Transfer not permitted / `TOKEN_VAZIO`.

**Selected mechanism:** `TOKEN_VAZIO`

### 3.1 ANPD standard contractual clauses gate

If selected:

1. Attach the official ANPD standard contractual clauses **integrally and without modification**.
2. Complete only the fields/annex information contemplated by the official instrument.
3. Record exporter/importer identities, processing description, security measures and onward-transfer facts.
4. Publish/provide the transparency information required by the applicable ANPD regulation in Portuguese through an accessible channel.
5. Preserve signed version, date, parties, annexes and change history as evidence.

Official clause artifact/hash: `TOKEN_VAZIO`

## 4. EU/EEA GDPR route

Select only after factual/legal review:

- [ ] Adequacy decision applicable.
- [ ] Commission SCCs 2021/914, Module 1 Controller→Controller.
- [ ] Commission SCCs 2021/914, Module 2 Controller→Processor.
- [ ] Commission SCCs 2021/914, Module 3 Processor→Processor.
- [ ] Commission SCCs 2021/914, Module 4 Processor→Controller.
- [ ] BCR/other lawful Chapter V mechanism.
- [ ] Transfer not permitted / `TOKEN_VAZIO`.

**Selected mechanism:** `TOKEN_VAZIO`

### 4.1 EU SCC gate

If selected:

1. Use the official Commission Implementing Decision (EU) 2021/914 clause text and appropriate module(s).
2. Complete Annex I parties/transfer description, Annex II technical and organizational measures, and Annex III subprocessors when applicable.
3. Do not add terms that contradict or undermine the SCCs or data-subject rights.
4. Perform/document transfer-impact assessment where required by the circumstances and applicable guidance.
5. Select supplementary safeguards proportionate to identified risk.

Official SCC artifact/hash: `TOKEN_VAZIO`

## 5. Supplementary technical safeguards

Select based on risk and actual architecture:

- [ ] Encryption in transit with current secure protocol configuration.
- [ ] Encryption at rest for stored personal data.
- [ ] Customer-controlled or segregated key option where justified.
- [ ] Pseudonymization/tokenization before transfer where feasible.
- [ ] Data minimization and field-level exclusion.
- [ ] Least privilege and MFA for privileged remote access.
- [ ] Region/location restrictions enforced technically.
- [ ] Audit logging for administrative/export access.
- [ ] Short retention and verified deletion.
- [ ] Split processing so destination cannot reconstruct sensitive context alone.
- [ ] Emergency access procedure with evidence and review.

Implemented safeguards/evidence: `TOKEN_VAZIO`

## 6. Government/law-enforcement request handling

To the extent lawful and technically possible, importer/provider should:

- assess legal validity and scope of a request;
- challenge disproportionate/unlawful requests where there are reasonable grounds and legal means;
- disclose only what is legally required;
- preserve records of requests and response decisions;
- notify exporter/customer where legally permitted;
- report aggregate transparency information where appropriate and lawful.

No contractual promise overrides binding law.

## 7. Onward transfer/subprocessors

Every onward transfer must map to an approved subprocessor/vendor record and a valid transfer mechanism. A supplier's corporate headquarters do not establish the processing location; actual storage, backup, telemetry, support and administrative-access locations must be recorded.

## 8. Transparency/public information gate

Where required, the controller must provide clear, accessible information about international transfers, including relevant destination/purpose/responsibility/security/rights information. Customer-specific notice URL/artifact: `TOKEN_VAZIO`.

## 9. Transfer change control

A new destination, subprocessor, remote-support region, data category, purpose or transfer mechanism is a material change. It requires impact review before production activation unless a lawful emergency process applies.

## 10. Evidence receipt

A valid transfer receipt should record:

```text
transfer_id
parties + roles
origin/destination
purpose + data categories
selected legal mechanism
official clause/version/hash where applicable
TOMs/supplementary measures
authorized subprocessors/onward transfers
transparency artifact
approval/signatures
effective date
review/expiry trigger
```

## 11. Gate

`TRANSFER_ALLOWED=true` only when required fields, lawful mechanism, official clauses/annexes where applicable, security measures, subprocessor chain, transparency and authority/signature evidence are closed. Otherwise: `TRANSFER_ALLOWED=false`, gap=`TOKEN_VAZIO`.
