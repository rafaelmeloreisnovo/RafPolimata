# Data Processing Addendum — LGPD + GDPR V1

**Status:** `CONTRACT_TEMPLATE / NOT_EXECUTED`  
**Purpose:** govern personal-data processing performed in connection with a signed MSA/SOW.

> This DPA is a modular template. It does not itself establish that LGPD, GDPR, or any specific role applies. Roles and transfer mechanisms must be determined from the actual processing facts and completed annexes.

## 1. Parties and precedence

This DPA forms part of the applicable MSA/SOW between `TOKEN_VAZIO` (`Customer`) and `TOKEN_VAZIO` (`Provider`). For personal-data matters, this DPA prevails over conflicting general commercial terms. Mandatory statutory and mandatory transfer-clause requirements prevail as required by law.

## 2. Roles

The parties will identify each processing activity as applicable:

- LGPD: `Controlador`, `Operador`, or other legally applicable role;
- GDPR: `controller`, `processor`, `joint controller`, or independent controller.

Contract labels do not override factual control over purposes and essential means.

**Role matrix:** `TOKEN_VAZIO`

## 3. Processing details

Complete before production processing:

- subject matter: `TOKEN_VAZIO`
- duration: `TOKEN_VAZIO`
- nature/operations: `TOKEN_VAZIO`
- purposes: `TOKEN_VAZIO`
- data-subject categories: `TOKEN_VAZIO`
- personal-data categories: `TOKEN_VAZIO`
- sensitive/special-category data: `TOKEN_VAZIO`
- children/adolescents: `TOKEN_VAZIO`
- processing locations: `TOKEN_VAZIO`
- retention/deletion schedule: `TOKEN_VAZIO`
- approved subprocessors: `TOKEN_VAZIO`
- international-transfer mechanism(s): `TOKEN_VAZIO`

A material unresolved field blocks production processing when it is necessary to establish lawfulness or required safeguards.

## 4. Documented instructions

Where Provider acts as processor/operator, Provider will process personal data only on documented lawful Customer instructions, including with respect to international transfers, unless applicable law requires otherwise. Where legally permitted, Provider will inform Customer before processing required by law.

If Provider reasonably believes an instruction violates applicable data-protection law, Provider will notify Customer and may suspend the affected instruction while the parties resolve it, without silently converting the instruction into a different purpose.

## 5. Confidentiality and personnel

Provider will ensure persons authorized to process personal data are bound by appropriate confidentiality duties and receive access only to the extent needed for their function. Access rights will be reviewed and revoked when no longer required.

## 6. Security of processing

Provider will implement the Security Assurance Schedule selected for the service, considering the nature, scope, context, purpose and risks of processing. Controls may include, as applicable:

- identity and least privilege;
- MFA for privileged/remote administrative access;
- encryption in transit and at rest where appropriate;
- managed secrets and key rotation;
- separation of production/non-production;
- secure development, dependency and provenance controls;
- logging, monitoring and tamper-evident evidence;
- backup, restore, fail-safe/failover and tested recovery;
- vulnerability management and incident response;
- data minimization, retention and secure deletion.

Security controls are commitments to reasonable/risk-appropriate measures, not a guarantee that no incident will occur.

## 7. Subprocessors

Provider will use subprocessors only under the authorization mechanism selected below and will impose data-protection obligations appropriate to the services and applicable law.

Authorization model: `TOKEN_VAZIO: specific | general-written-with-notice`

Provider will maintain the `SUBPROCESSOR_SUPPLY_CHAIN_REGISTER_V1` or an equivalent customer-specific register including service, role, data, location and transfer basis. Customer objection rights and notice period: `TOKEN_VAZIO`.

## 8. Data-subject rights

Taking into account the nature of processing, Provider will reasonably assist Customer with requests and obligations relating to access, confirmation, correction, deletion/erasure, portability where applicable, restriction/opposition where applicable, information, consent withdrawal, automated-decision rights where applicable, and other mandatory rights.

Provider will not independently respond on Customer's behalf unless authorized or legally required.

## 9. DPIA/RIPD and prior consultation

Provider will reasonably assist with a DPIA, RIPD or regulator consultation where required and where the relevant information is within Provider's control, including system description, security measures, processing locations and subprocessors, subject to appropriate confidentiality/security protections.

## 10. Security incidents

### 10.1 Processor/operator to controller
Provider will notify Customer **without undue delay** after confirming a personal-data incident affecting Customer data and, as an operational target, will seek to provide an initial notice within **24 hours** of such confirmation when reasonably practicable. The 24-hour target is designed to preserve Customer response time; it does not replace a shorter mandatory deadline or create a false-completeness requirement where facts are still developing.

Initial notice should include available information on nature, systems/data affected, approximate scope, containment, likely impact and contact point, followed by staged updates as facts become known.

### 10.2 Controller regulatory duties
The controller remains responsible for determining regulator/data-subject notification requirements unless law assigns otherwise. Under Brazil's current ANPD incident regulation, qualifying incidents are generally communicated by the controller to the ANPD and affected holders within the regulatory three-business-day period, subject to the regulation and sector-specific rules. Under GDPR, applicable supervisory-authority/data-subject notification duties remain governed by the GDPR.

### 10.3 Evidence
Provider will preserve proportionate incident evidence, root-cause findings, remediation actions and relevant logs, while protecting other customers, secrets and investigation integrity.

## 11. Records and accountability

Each party will maintain records required for its role. Provider will provide information reasonably necessary to demonstrate its applicable processor/operator obligations, using a proportional evidence hierarchy such as independent reports, certifications where actually held, policies/control evidence, SBOM/provenance, and targeted audit responses.

No expired or out-of-scope report is represented as current certification.

## 12. Audit

Customer may audit compliance where required by applicable law or contract, subject to reasonable notice, scope, frequency, confidentiality and security constraints. The parties should prefer least-invasive evidence sufficient to answer the control question before onsite/source-system access.

Audit must not expose unrelated customer data, exploitable secrets, or security-sensitive information beyond what is reasonably necessary.

## 13. International transfers

No cross-border transfer is authorized merely because this DPA exists. The parties must select and document a valid transfer mechanism in `INTERNATIONAL_DATA_TRANSFER_SCHEDULE_V1` or equivalent.

For Brazil, when the ANPD standard contractual clauses are the selected mechanism, the official text must be incorporated integrally and without modification as required by the applicable ANPD regulation. For GDPR transfers relying on Commission Decision (EU) 2021/914 SCCs, the applicable module(s) and annexes must be completed and supplementary measures assessed where needed.

## 14. Data location and onward transfer

Provider will not materially change an agreed restricted data-location boundary or onward-transfer route without the notice/approval required by the SOW, this DPA and applicable law. Location claims must refer to actual processing/storage/backups/support access, not merely a billing region.

## 15. Return and deletion

At end of services, Provider will return or delete Customer personal data according to Customer's documented choice and the agreed schedule, unless applicable law requires retention. Backup deletion may follow documented lifecycle windows if data is isolated from ordinary use and protected until expiry.

Deletion evidence method: `TOKEN_VAZIO`.

## 16. Data minimization and purpose limitation

Provider will not use Customer personal data for unrelated advertising, sale, data-broker activity, or model training unless the Customer has expressly instructed/contracted that purpose and the processing is lawful. Default service telemetry should be minimized to what is needed for security, operation, billing or improvement as contractually disclosed.

## 17. AI/automated systems

If the service uses external AI/ML processing on personal/confidential data, the SOW must state provider/model/service, data flow, retention/training settings, location, subprocessor status, contractual terms, transfer mechanism and human-review requirements. Absent that evidence, external AI processing of Customer personal data is `TOKEN_VAZIO/NOT_AUTHORIZED`.

## 18. Annex A — processing specification

`TOKEN_VAZIO`

## 19. Annex B — technical and organizational measures

Reference the signed `SECURITY_ASSURANCE_SCHEDULE_V1` and any customer-specific controls.

## 20. Annex C — subprocessors

Reference the customer-specific approved register.

## 21. Annex D — international transfers

Reference the completed transfer schedule and, where applicable, attach the exact official ANPD standard clauses and/or EU SCCs without contradictory modification.

## 22. Signature gate

This DPA is not `PASS/EFFECTIVE` until party identities, roles, processing annex, security schedule, transfer mechanism where needed, subprocessor route and authorized signatures are complete.
