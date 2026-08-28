# Security Assurance Schedule V1

**Status:** `CONTRACT_TEMPLATE / CONTROL_BASELINE`  
**Principle:** measurable controls + evidence + recovery; no absolute-security guarantee.

## 1. Assurance tiers

| Tier | Intended use | Minimum posture |
|---|---|---|
| R0 Research | isolated/non-production experiments | bounded data, no production secrets, reproducible evidence |
| S1 Standard | ordinary commercial service | identity, encryption, secure SDLC, backups, vulnerability/incident process |
| S2 High | confidential/personal/important workloads | S1 + stronger segregation, provenance, monitoring, recovery tests, supplier controls |
| S3 Critical | high-impact/regulated scope | S2 + customer-specific threat model, independent evidence/audit, tighter RTO/RPO and approval gates |

Selected tier: `TOKEN_VAZIO`.

## 2. Zero-trust operational identity

Required as applicable:
- unique human/service identities;
- least privilege and role separation;
- MFA for privileged and remote administrative access;
- credential lifecycle, rotation and rapid revocation;
- no plaintext secrets in repositories/logs;
- break-glass access logged, time-bounded and reviewed;
- periodic access recertification.

Evidence: `TOKEN_VAZIO`.

## 3. Cryptography and key management

- Use standardized, reviewed cryptographic primitives and protocols appropriate to the use case.
- Experimental mixers/fingerprints are never promoted as cryptographic primitives.
- Keys/secrets are separated from source and least-privilege accessible.
- Rotation/revocation/recovery procedures are defined for production secrets.
- Cryptographic parameters, algorithm versions and deprecation triggers are inventoried.
- Hashes prove byte identity/integrity only within their stated evidence boundary; they do not prove semantic truth.

Key owner/custody: `TOKEN_VAZIO`.

## 4. Secure development lifecycle

The service should map its development controls to recognized references such as NIST SSDF, OWASP ASVS where application controls are relevant, and supply-chain frameworks such as SLSA/OpenSSF. Reference alignment does not imply certification.

Minimum gates:

```text
requirements/threats
-> code provenance
-> review
-> deterministic/reproducible tests where feasible
-> dependency/license scan
-> secret scan
-> security tests
-> artifact identity/SBOM
-> signed/authorized release
-> deploy receipt
-> monitoring/recovery
```

Evidence/tooling: `TOKEN_VAZIO`.

## 5. Supply-chain security

For every production dependency/provider:
- pin/version identity where technically feasible;
- origin and maintainer/vendor identity;
- license/terms;
- integrity/provenance evidence;
- vulnerability/advisory monitoring;
- transitive dependency awareness;
- replacement/rollback path;
- data/subprocessor implications;
- lifecycle/EOL risk.

Unreviewed critical dependency = `TOKEN_VAZIO`, not implicit trust.

## 6. Environment segregation

Production, development and test boundaries must be explicit. Production data is not copied to lower environments unless authorized, necessary, minimized and equivalently protected. Debug/test interfaces and credentials must not silently remain enabled in production.

## 7. Network and service exposure

- inventory externally reachable services;
- default-deny where practical;
- authenticated administrative interfaces;
- rate/resource abuse controls appropriate to the service;
- secure transport;
- restricted management plane;
- patch/deprecation route;
- logging for material administrative/security events.

## 8. Data security

Apply classification and minimization. Define encryption, pseudonymization, access, retention, backup and deletion controls by data class. Security telemetry should avoid unnecessary collection of content or secrets.

## 9. Logging and evidence integrity

Security/audit records should have:
- stable event identity/time source;
- actor/action/resource/outcome where appropriate;
- integrity/tamper controls proportionate to risk;
- access restrictions;
- retention period;
- privacy minimization/redaction;
- export/receipt capability for investigations.

Logs are evidence, not automatically truth; clock/source gaps are recorded.

## 10. Vulnerability management

### Severity route
Use a documented severity method (e.g., CVSS-informed plus exploitability/business context) rather than raw scanner count.

### Target remediation windows
Default contractual targets unless a SOW selects stricter values:
- Critical, actively exploited/materially exposed: containment as soon as practicable; remediation/workaround target `24-72h` depending on operational safety.
- Critical otherwise: target `7 days`.
- High: target `30 days`.
- Medium: target `90 days`.
- Low: risk-based backlog.

These are targets, not fabricated guarantees. Exceptions require owner, reason, compensating controls and expiry/review date.

## 11. Coordinated vulnerability disclosure

Maintain a private reporting channel, acknowledgment process, triage, safe handling of researcher data and a coordinated disclosure route. Do not require researchers to expose unnecessary personal information. Bug-bounty terms, if any, are separate and explicit.

## 12. Incident response

State machine:

```text
DETECT -> TRIAGE -> CONTAIN -> PRESERVE -> ERADICATE
-> RECOVER -> VERIFY -> NOTIFY(if required) -> LEARN
```

Provider-to-customer personal-data incident notice follows the DPA. Regulatory notifications are determined under applicable law and actual facts.

Incident record includes detection source, timeline, affected identities/assets/data, containment, evidence hashes/locations, cause, decisions, notifications, recovery verification and corrective actions.

## 13. Watchdog / fail-safe / failover / rollback / failback

For critical runtime paths, define:
- health signal and deadline;
- failure threshold;
- safe degraded state;
- failover target;
- rollback artifact/state;
- integrity check before failback;
- maximum data-loss/recovery objectives where contracted;
- evidence proving recovery tests.

A failover route that has never been tested remains `IMPLEMENTED/TOKEN_VAZIO`, not `PASS`.

## 14. Backup and recovery

Define backup scope, encryption, isolation, retention, restore procedure, RPO and RTO. Test restores at a frequency proportional to tier and preserve evidence. A successful backup job does not prove restorability.

## 15. Business continuity and dependency failure

Document single points of failure and provider dependencies. Where material, maintain alternatives/export paths for identity, code, CI, cloud/storage, DNS, communications and key custody. Failure of a BigTech provider is treated as a dependency failure, not as an impossible event.

## 16. Security testing

Possible gates by scope:
- static/source review;
- unit/property/negative tests;
- dependency/SBOM checks;
- fuzzing for parsers/unsafe boundaries;
- configuration review;
- authenticated application tests;
- penetration testing for exposed production surfaces;
- recovery/failure injection;
- independent review for S3/high-impact claims.

Scope/date/method/findings determine what a test proves.

## 17. Customer responsibilities

Customer remains responsible for its identities/endpoints, lawful instructions/data, customer-managed keys/configuration, prompt reporting of suspected compromise, and controls expressly allocated to Customer in the SOW/shared-responsibility matrix.

## 18. Security change control

Material architecture, crypto, authentication, exposure, storage region, critical dependency or privileged-access changes require risk review before production promotion, except emergency containment followed by post-event review.

## 19. Assurance evidence pack

For contracted scope, evidence may include:
- architecture/threat model;
- secure-development controls;
- source/artifact hashes;
- SBOM/licensing;
- CI/test receipts;
- vulnerability register;
- access review;
- backup/restore receipt;
- incident tabletop/failure-injection receipt;
- supplier register;
- independent report/certification if actually held.

## 20. Claims gate

`SECURITY_ASSURED=true` is prohibited as a blanket statement. Instead record specific controls/tests and their state. `CERTIFIED` may be used only with an actual valid certification and its exact scope/issuer/date.
