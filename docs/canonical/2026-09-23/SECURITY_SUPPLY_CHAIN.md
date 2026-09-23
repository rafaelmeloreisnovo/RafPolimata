# Security, provenance and supply-chain documentation

**Governance binding: CLOSURE_L11** — missing assurance remains a gap, not a certificate.  
**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92

## Security entry

The repository already has .github/SECURITY.md. Its current policy:
- scopes vulnerability reporting to code/scripts/workflows/artifacts/provenance;
- warns against publishing exploit-ready details or secrets;
- keeps private-reporting availability explicitly unverified where not observed;
- uses evidence states for triage;
- does not promise bounty or SLA;
- separates internal alignment from external certification.

This documentation cut preserves that policy.

## Supply-chain layers present

Observed control surfaces include:
- Git history and exact commit binding;
- workflow gates;
- artifact uploads from Actions;
- source-contract integrity checks in RAF Hash Fabric;
- hashes and receipts in multiple subsystems;
- compiler target registry;
- document-governance provenance;
- legal/supply-chain templates under docs/legal/.

These controls are heterogeneous. Their presence does not equal SLSA level, ISO certification, secure-build certification or complete SBOM coverage.

## RAF Hash Fabric source contract

native/raf_hash_fabric_v1 binds selected module documentation/license/scope bytes to source anchors using Git blob identity and verifies one-bit mutations are rejected.

Correct interpretation:
- integrity/binding mechanism for that module;
- not a replacement for digital signatures;
- not a repository-wide license;
- not a cryptographic-security proof of the algorithms.

## Legal & Service Assurance Pack

docs/legal/README.md describes a reference/template stack for:
- IP/license;
- service agreement/SOW;
- privacy/DPA;
- international transfers;
- security assurance;
- subprocessors;
- privacy notice;
- third-party crypto/license register;
- machine-readable legal controls.

Its own status is REFERENCE / CONTRACT_TEMPLATE / AUDIT_REQUIRED. A template does not become an executed contract or certification.

## Repository license boundary

GitHub did not report a repository-level license in the current metadata.

The RAF Hash Fabric module has its own LICENSE.md and LICENSE_SCOPE_V1.json. This does not relicense unrelated repository content.

No global LICENSE is created by this documentation pass because the rightsholder's repository-wide licensing decision is not established by source evidence alone.

## Branch and review controls

Observed administrative state:
- active repository ruleset blocks deletion and non-fast-forward;
- classic branch protection/required status checks were not reported for main;
- CODEOWNERS is added by this documentation branch for review routing;
- CODEOWNERS alone does not enforce approval.

Any requirement for signed commits, mandatory reviews, required checks or restricted pushes must be an explicit administrative decision.

## Secret handling

Documentation and receipts should record detector/state/identity needed for audit, never copy the secret itself.

## External standards

Repository documents reference NIST CSF, SSDF, SLSA, ISO and related standards as alignment/reference surfaces. Unless an external audit/certification exists for the exact scope, language must remain reference-only.

R3 = ⟨F_ok: security, license, source-contract and governance surfaces separated; F_gap: global license and stronger branch/release supply-chain controls require explicit owner/admin decisions; F_next: resolve legal scope and then choose enforceable GitHub rules without representing reference alignment as certification⟩.
