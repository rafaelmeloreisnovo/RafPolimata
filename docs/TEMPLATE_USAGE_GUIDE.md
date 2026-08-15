# Template Usage Guide: 5 Document Types for 360° Professional Documentation

**Date:** 2026-08-15  
**Purpose:** Explain how to use all 5 templates cohesively to close gaps and maintain professional standards  
**Lattice Position:** Documentation/Specifications  

---

## Quick Reference: Which Template to Use

| Question | Document Type | When | Example |
|----------|---|---|---|
| "What does this component do formally?" | **SPEC_*.md** | Defining algorithms, type systems, analysis | `SPEC_TYPE_SYSTEM.md` - formal inference rules |
| "How do we prove this works?" | **WITNESS_*.md** | Capturing evidence, audit trails, reproducibility | `WITNESS_DETERMINISM_20260824.md` - 3-build hash proof |
| "How do we deploy/recover/debug?" | **RUNBOOK_*.md** | Step-by-step operations procedures | `RUNBOOK_DEPLOY_TO_ANDROID.md` - deployment steps |
| "Why did we make this architectural choice?" | **ADR_*.md** | Explaining design rationale and trade-offs | `ADR_0001_FREESTANDING_NO_MALLOC.md` - why no malloc |
| "How do we track gap closure?" | **CLOSURE_*.md** | Mapping gaps to solutions and evidence | `CLOSURE_VOID_C1_COMPILER_PROVENANCE.md` - gap resolution |

---

## The 5-Template Ecosystem

### Overview: How Templates Work Together

```
Gap Discovered (from GAP_INDEX_COMPLETE_AUDIT.md)
    ↓
Design Phase
    ├─→ ADR_*.md: "Why this solution?"
    └─→ SPEC_*.md: "What are the formal requirements?"
    ↓
Implementation Phase
    └─→ Code + Tests (in Apkc/, tests/)
    ↓
Verification Phase
    ├─→ WITNESS_*.md: "Prove it works (reproducible evidence)"
    └─→ RUNBOOK_*.md: "Document how to verify/deploy"
    ↓
Gap Closure Phase
    └─→ CLOSURE_*.md: "Record solution + link all evidence"
    ↓
Documentation Complete
    ├─→ All docs linked in CONCEPT_LATTICE.md
    └─→ Gap marked as ✅ CLOSED in GAP_INDEX_COMPLETE_AUDIT.md
```

---

## Template 1: SPEC_* (Technical Specifications)

### Purpose
Define **what** a component does, formally and precisely, with invariants and proofs.

### When to Write
- Starting a new phase/component (Phases 21+)
- Formalizing an algorithm or type system
- Creating golden test cases
- Proving correctness properties

### Key Sections
1. **Formal Definition** — Math notation, type grammar, data structures
2. **Algorithm** — Pseudocode or step-by-step procedure
3. **Type Theory** — Soundness, completeness, proof sketch
4. **Examples & Tests** — Golden test cases with hand-traced results
5. **Implementation Notes** — Freestanding constraints, buffer sizes, complexity
6. **Verification** — Test coverage, regression tests, performance

### Template File
```
docs/SPEC_[ComponentName].md
```

### Example Flow
```
Implement Phase 21 (Type System)
    ↓
Write: SPEC_TYPE_SYSTEM.md (formal inference rules)
    ↓
Match implementation to spec (Apkc/sem_type_system.h)
    ↓
Create golden tests from SPEC examples
    ↓
Run verification: tests passing → mark SPEC ✅ PASS
```

### Linking
```markdown
- **Related Code:** Apkc/sem_type_system.h (365 lines)
- **Related Tests:** tests/test_phase21_type_checker.c (50+ tests)
- **Related ADR:** ADR_0003_HINDLEY_MILNER_INFERENCE.md (why this algorithm)
- **Lattice Position:** Compiler/Phases 21-45/SemanticAnalysis/Type-Symbol
```

---

## Template 2: WITNESS_* (Hardcoding Witness / Evidence)

### Purpose
**Prove** that something works through reproducible, auditable evidence (not claims).

### When to Write
- After implementation is complete
- When you want to prove reproducibility, security, or correctness
- Creating audit trail for compliance/verification
- Capturing performance baseline or determinism

### Key Sections
1. **Claim** — What this proves (one sentence)
2. **Setup** — Environment, prerequisites, reproducibility
3. **Test Command & Execution** — Exact commands + actual output
4. **Evidence Chain** — Cryptographic chain of provenance
5. **Interpretation** — What this proves + what it does NOT prove
6. **Failure Scenarios** — What to do if test fails
7. **CI Integration** — How to automate this witness
8. **Attestation** — Sign-off by implementer, reviewer, tech lead

### Template File
```
docs/proofs/WITNESS_[ComponentName]_[YYYYMMDD].md
```

### Example Flow
```
Implement compiler provenance chain (tools/witness_apkc_build.sh)
    ↓
Run command: bash tools/witness_apkc_build.sh > /tmp/witness.json
    ↓
Capture actual output in section 3.3 (exact terminal output)
    ↓
Write WITNESS_APKC_BUILD_20260824.md with evidence
    ↓
Verify: witness JSON contains {commit, date_utc, compiler_version, binary_hash}
    ↓
Get sign-offs → Commit witness as immutable record
    ↓
Mark gap VOID-C1 as ✅ CLOSED (proof exists)
```

### Key Insight
**Witness is immutable:** Once committed to git, cannot be changed. This makes witness valuable as audit trail.

### Linking
```markdown
- **Closes Gap:** VOID-C1 (Compiler Provenance)
- **Closure Document:** docs/closures/CLOSURE_VOID_C1_COMPILER_PROVENANCE.md
- **Related Runbook:** RUNBOOK_VERIFY_BUILD_REPRODUCIBILITY.md
- **CI Integration:** .github/workflows/ci.yml (witness auto-generated)
- **Lattice Position:** Documentation/Proofs/CompilerProvenance
```

---

## Template 3: RUNBOOK_* (Operational Procedures)

### Purpose
Provide **how-to** for deploying, debugging, recovering from failures.

### When to Write
- After code is production-ready
- Before deploying to critical environment
- When a procedure is manual and error-prone
- Creating on-call playbooks for incident response

### Key Sections
1. **Executive Summary** — What this does, when to use, success criteria
2. **Prerequisites & Safety Checks** — Must-have setup, abort conditions
3. **Step-by-Step Procedure** — Copy-paste-ready commands + verifications
4. **Troubleshooting** — Symptoms, diagnostics, fixes for each step
5. **Rollback Procedure** — How to recover if something goes wrong
6. **Verification & Monitoring** — Post-procedure health checks
7. **Communication** — What to tell stakeholders during/after
8. **Special Cases** — Offline mode, partial failure, concurrent ops
9. **Maintenance** — How to keep runbook current

### Template File
```
docs/runbooks/RUNBOOK_[Operation].md
```

### Example Flow
```
Implement Phase 50 deployment framework
    ↓
Write RUNBOOK_DEPLOY_TO_ANDROID.md with exact steps:
  - Prerequisites (adb, device, signed APK)
  - Step 1: adb install -r app.apk
  - Step 2: adb shell monkey -p com.example.app 1
  - Step 3: adb logcat | grep -i fatal
  - Verification: no crashes in logcat
    ↓
Test runbook in staging (dry-run, don't deploy to prod yet)
    ↓
Execute runbook step-by-step, update actual times + issues
    ↓
Get sign-offs from ops team
    ↓
Commit runbook + mark ready for production use
```

### Key Insight
**Runbooks are training documents:** A new engineer should be able to follow runbook without asking questions.

### Linking
```markdown
- **Related Spec:** SPEC_DEPLOYMENT_FRAMEWORK.md (design)
- **Related ADR:** ADR_0009_DEPLOYMENT_GATES.md (why this design)
- **Related Witness:** WITNESS_DEPLOYMENT_SUCCESS_[date].md (proof it worked)
- **Monitoring:** Performance_SLA_BASELINE.md (what metrics to watch)
- **Lattice Position:** Operations/Runbooks/Deployment
```

---

## Template 4: ADR_* (Architecture Decision Records)

### Purpose
Explain **why** we made a particular architectural choice, including trade-offs and alternatives rejected.

### When to Write
- Before implementing a major component
- When multiple approaches are possible
- To document design rationale for future maintainers
- Creating decision history for auditing

### Key Sections
1. **Context** — Problem statement, constraints, stakeholders
2. **Decision** — What we chose, how it works, the implementation
3. **Rationale** — Why this choice, what alternatives were rejected
4. **Consequences** — Positive + negative outcomes of this decision
5. **Related Decisions** — How this links to other ADRs
6. **Verification** — How we know this decision is working
7. **Future Improvements** — What could be better in Phase N+1

### Template File
```
docs/adr/ADR_[Number]_[Title].md
```

### Example Flow
```
Designing Phase 21 Type System
    ↓
Decide: Use Hindley-Milner inference instead of bidirectional checking
    ↓
Write ADR_0003_HINDLEY_MILNER_INFERENCE.md:
  - Context: 12 languages need unified type inference
  - Decision: HM algorithm (simpler, proven, ≤O(n) solutions)
  - Why not bidirectional: more complex, overkill for our use case
  - Consequences: 
    - Pro: elegant, extensible to generics
    - Con: cannot handle some advanced type features
    ↓
Implement Apkc/sem_type_inference.h matching HM algorithm
    ↓
Reference ADR in code comments & SPEC_TYPE_SYSTEM.md
    ↓
Update CONCEPT_LATTICE.md to link ADR to code
```

### Key Insight
**ADRs are immutable history:** Decisions are final once recorded. If requirements change, write a new ADR (e.g., "ADR_0004: Extend HM with row polymorphism").

### Linking
```markdown
- **Related SPEC:** SPEC_TYPE_SYSTEM.md (formal spec of what we decided)
- **Related Code:** Apkc/sem_type_system.h (implementation)
- **Related Tests:** tests/test_type_inference.c (verification)
- **Supersedes:** (none, initial decision)
- **Superseded by:** (none yet, but link here if decision changes)
- **Lattice Position:** Documentation/Decisions/TypeSystem
```

---

## Template 5: CLOSURE_* (Gap Closure)

### Purpose
**Track** how a gap (VOID/PENDING/AUDIT) is closed: what solution, implementation, evidence, verification.

### When to Write
- After identifying a gap (from GAP_INDEX_COMPLETE_AUDIT.md)
- When beginning gap resolution work
- When collecting evidence that gap is closed
- For audit trail / compliance

### Key Sections
1. **Gap Definition** — Copy gap from index, why it matters
2. **Root Cause Analysis** — Why the gap exists, how to prevent similar gaps
3. **Closure Solution** — What closes the gap, implementation plan, acceptance criteria
4. **Implementation Timeline** — Phases, owners, milestones
5. **Evidence & Verification** — Checklist of proof that gap is closed
6. **Sign-Off** — Approvals from implementer, reviewer, tech lead
7. **Post-Closure Monitoring** — How to prevent regression

### Template File
```
docs/closures/CLOSURE_[GapID]_[Title].md
```

### Example Flow
```
Identify gap: VOID-C1 (Compiler Provenance Chain missing)
    ↓
Create: CLOSURE_VOID_C1_COMPILER_PROVENANCE.md
    ↓
Design solution:
  - Implement tools/witness_apkc_build.sh (captures witness JSON)
  - Integrate into CI (.github/workflows/ci.yml)
  - Create witness evidence (docs/proofs/WITNESS_*.md)
    ↓
Implement (2-week sprint):
  - Day 1-2: Write script
  - Day 3-4: CI integration
  - Day 5: Testing + documentation
    ↓
Verify acceptance criteria:
  - [ ] Script runs: bash tools/witness_apkc_build.sh ✅
  - [ ] Output correct: JSON has all fields ✅
  - [ ] Determinism: 3 runs match ✅
  - [ ] CI integrated: workflow step exists ✅
    ↓
Collect evidence:
  - WITNESS_APKC_BUILD_20260824.md (proof script works)
  - GitHub Actions run logs (CI integration works)
  - RUNBOOK_VERIFY_BUILD_REPRODUCIBILITY.md (operational use)
    ↓
Get sign-offs:
  - Implementer: "I verified acceptance criteria"
  - Reviewer: "I reviewed implementation"
  - Tech lead: "I approve gap closure"
    ↓
Commit CLOSURE document → Mark gap ✅ CLOSED in index
```

### Key Insight
**CLOSURE documents chain all evidence together:** A single document links implementation + tests + witness + runbook + ADR + spec. External auditors can follow chain start-to-finish.

### Linking
```markdown
- **From Gap:** GAP_INDEX_COMPLETE_AUDIT.md → VOID-C1
- **Closes:** VOID-C1 (Compiler Provenance)
- **Implemented by:** Phase 46-48 (Production Integration)
- **Related Witness:** WITNESS_APKC_BUILD_*.md (proof)
- **Related Runbook:** RUNBOOK_VERIFY_BUILD_REPRODUCIBILITY.md (operations)
- **Related Spec:** SPEC_COMPILER_PROVENANCE.md (what reproducibility means)
- **Related ADR:** ADR_0001_FREESTANDING_NO_MALLOC.md (why determinism critical)
- **Lattice Position:** Documentation/Closures/CompilerProvenance
```

---

## How to Write High-Quality Documentation Using All 5 Templates

### 1. Discovery Phase: What Gap Exists?

**Input:** Requirement or user need  
**Process:**
1. Check GAP_INDEX_COMPLETE_AUDIT.md — does this gap already exist?
2. If YES → create CLOSURE_* for that gap
3. If NO → add new gap to index (document + assign impact)

**Output:** Gap ID (e.g., VOID-C1)

---

### 2. Design Phase: Why This Solution?

**Input:** Gap ID + requirements  
**Process:**
1. **Write ADR_*.md** explaining design choice
   - What problem are we solving?
   - Why this approach (vs. alternatives)?
   - What are trade-offs?
   
2. **Write SPEC_*.md** defining formal requirements
   - Formal definition of what we're building
   - Algorithms, invariants, proofs
   - Examples and test cases

**Output:** Design document + specification

---

### 3. Implementation Phase: Build Solution

**Input:** Design + spec  
**Process:**
1. Implement in Apkc/ (or relevant codebase)
2. Create unit tests (tests/test_*.c)
3. Create integration tests
4. Verify code matches spec

**Output:** Working code + tests passing

---

### 4. Verification Phase: Prove It Works

**Input:** Working code  
**Process:**
1. **Write WITNESS_*.md** proving the code works
   - Capture evidence (hashes, outputs, timestamps)
   - Prove determinism/reproducibility
   - Get sign-offs

2. **Write RUNBOOK_*.md** for operational use
   - How to deploy/debug this component
   - Step-by-step procedures
   - Troubleshooting & recovery

**Output:** Immutable evidence + operational procedures

---

### 5. Closure Phase: Connect All Evidence

**Input:** All of above (ADR + SPEC + code + tests + WITNESS + RUNBOOK)  
**Process:**
1. **Write CLOSURE_*.md** connecting all evidence
   - Copy gap definition from index
   - Describe solution
   - Link all related docs (ADR, SPEC, WITNESS, RUNBOOK)
   - Get final sign-offs

2. **Update lattice:** CONCEPT_LATTICE.md
   - Add new doc to lattice position
   - Link cross-references

3. **Update gap index:** GAP_INDEX_COMPLETE_AUDIT.md
   - Change gap status from ⊘ OPEN → ✅ CLOSED
   - Link to CLOSURE document

**Output:** Gap marked closed + all evidence linked + auditable chain

---

## Quality Checklist: Before Committing Any Doc

### Before Committing SPEC_*.md
- [ ] Formal definition is unambiguous (no hand-waving)
- [ ] Algorithm pseudocode is complete (can trace execution)
- [ ] Examples cover happy path + error cases (≥3 test cases)
- [ ] Proof sketch is sound (or explicitly marked TODO)
- [ ] Invariants are testable (code matches spec)
- [ ] Freestanding constraints documented (buffer sizes, no malloc, etc.)
- [ ] Performance characteristics stated (time/space complexity, benchmark results)

### Before Committing WITNESS_*.md
- [ ] Claim is specific and testable (one sentence, no ambiguity)
- [ ] Setup is reproducible (verbatim command provided)
- [ ] Actual output captured (not hypothetical, but real terminal output)
- [ ] Evidence chain is complete (source → compilation → binary → verification)
- [ ] Determinism verified (run 3×, outputs match exactly)
- [ ] Sign-offs present (implementer, reviewer, tech lead)
- [ ] No secrets in output (credentials, API keys, etc. removed)

### Before Committing RUNBOOK_*.md
- [ ] Commands are copy-paste-ready (no abbreviations, no assumptions)
- [ ] Steps have been tested ≥2× in real environment
- [ ] Verification checks actually work (not hypothetical)
- [ ] Troubleshooting covers common failure modes
- [ ] Rollback procedure has been tested (at least in staging)
- [ ] Estimated times are measured (not guesses)
- [ ] Escalation contacts are current (phone, Slack, email, pagerduty)

### Before Committing ADR_*.md
- [ ] Context explains problem clearly (why decision needed)
- [ ] Constraints explicitly stated (what limits our choices)
- [ ] Decision is clear and concrete (not vague or wishy-washy)
- [ ] Rationale explains trade-offs (not just pros)
- [ ] Alternatives considered (at least 2-3 other approaches evaluated)
- [ ] Consequences are realistic (both positive and negative)
- [ ] Related decisions linked (other ADRs, dependencies)

### Before Committing CLOSURE_*.md
- [ ] Gap ID matches GAP_INDEX_COMPLETE_AUDIT.md
- [ ] All acceptance criteria are checkable (not vague)
- [ ] Implementation is complete (not PENDING, but done)
- [ ] Evidence collected (related WITNESS, RUNBOOK, SPEC, ADR docs)
- [ ] Sign-offs complete (implementer ✅, reviewer ✅, tech lead ✅)
- [ ] Post-closure monitoring plan in place
- [ ] Related gap dependencies identified

---

## Integration with CI & Automation

### CI Gates for Documentation Quality

```yaml
# .github/workflows/ci.yml

- name: Validate Lattice Consistency
  run: bash tools/validate_lattice_consistency.sh
  # Checks: every doc has LATTICE_POSITION tag, no orphaned files

- name: Validate Gap Index
  run: bash tools/validate_gap_index.sh
  # Checks: every CLOSURE_*.md links to a gap in index
  # Checks: gap status matches closure status

- name: Lint Documentation
  run: bash tools/lint_docs.sh
  # Checks: SPEC_* files have required sections
  # Checks: WITNESS_*.md is immutable (no edits after merge)
  # Checks: ADR_*.md decision is not reversed
  # Checks: RUNBOOK_*.md commands pass syntax check

- name: Update Documentation Index
  run: bash tools/generate_doc_index.sh
  # Auto-generates docs/INDEX_AUTO.md from lattice
```

---

## Summary: 5 Templates = 360° Professional Documentation

| Template | Answers | Output | Audience |
|----------|---------|--------|----------|
| **SPEC_*** | What (formally)? How does it work? | Formal spec, algorithms, proofs | Implementers, mathematicians |
| **WITNESS_*** | Does it actually work? (proof) | Evidence, audit trail, reproducibility | Auditors, compliance, stakeholders |
| **RUNBOOK_*** | How to use/deploy/fix it? | Step-by-step procedures | Operations, on-call, incident response |
| **ADR_*** | Why this design? What alternatives? | Design rationale, trade-offs | Architects, future maintainers |
| **CLOSURE_*** | What gaps close? How? | Gap resolution tracking | Project managers, auditors |

**Together:** All 5 templates create a complete, auditable, professional documentation system where every claim has evidence, every design has rationale, and every gap has a closure path.

---

## Getting Started

1. **Read templates:** `docs/TEMPLATE_*.md` (this directory)
2. **Study examples:** Find similar docs in `docs/` that follow patterns
3. **Use for your gap:** Pick a gap from `GAP_INDEX_COMPLETE_AUDIT.md`
4. **Follow 5-phase flow:** Design → Implement → Verify → Closure
5. **Link to lattice:** Update `CONCEPT_LATTICE.md` as you go
6. **Commit immutably:** Once merged, doc is permanent record

**Questions?** See `docs/AGENTES.md` for AI collaboration protocol.
