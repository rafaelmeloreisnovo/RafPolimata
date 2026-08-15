<!-- TEMPLATE: Gap Closure Plan (CLOSURE_*.md) -->
<!-- LATTICE_POSITION: Documentation/Closures -->
<!-- USE THIS TEMPLATE FOR: Tracking gap resolution, documenting gap→PASS transition -->

# CLOSURE_[GapID]: [Gap Title]

**Date Created:** YYYY-MM-DD  
**Gap ID:** [e.g., VOID-C1, AUDIT-H1, PENDING-M3]  
**From:** docs/GAP_INDEX_COMPLETE_AUDIT.md  
**Status:** 🟢 CLOSED | 🟡 IN_PROGRESS | 🔴 OPEN | ⊘ BLOCKED  
**Lattice Position:** Documentation/Closures/[Category]  

---

## 1. Gap Definition

### 1.1 Original Gap Statement

Verbatim from GAP_INDEX_COMPLETE_AUDIT.md:

**Gap:** [Copy gap title]  
**Type:** VOID | PENDING | AUDIT | RUNTIME | REFERENCE  
**Impact:** CRITICAL | HIGH | MEDIUM | LOW  

**Original Description:**
```
[Copy full gap description from GAP_INDEX_COMPLETE_AUDIT.md]
```

### 1.2 Why This Gap Matters

**Business Impact:**
- Without closing this gap, [consequence for product/stakeholders]

**Technical Impact:**
- Blocks: [what cannot proceed without this]
- Risk: [what breaks if this remains unfixed]
- Opportunity: [what becomes possible if closed]

**Example:**
```
Gap: Compiler Provenance Chain (source→binary proof missing)
Impact: 
  - Cannot prove binaries are built from committed source
  - Commercial product cannot claim reproducibility
  - Customers cannot audit our build process
  - Regulatory compliance risk (supply chain attacks)
```

### 1.3 Current State Before Closure

**Evidence present:**
- ✅ [What exists]
- ⊘ [What is missing]
- ⚠️ [What is incomplete]

**Verification status:**
- [ ] Code review: ⊘ NOT DONE | ◐ PARTIAL | ✅ DONE
- [ ] Tests passing: ⊘ NOT DONE | ◐ PARTIAL | ✅ DONE
- [ ] Documentation: ⊘ NOT DONE | ◐ PARTIAL | ✅ DONE
- [ ] Audit trail: ⊘ NOT DONE | ◐ PARTIAL | ✅ DONE

---

## 2. Root Cause Analysis

Why does this gap exist? What led to it?

### 2.1 Contributing Factors

| Factor | Why Present? | How to Prevent? |
|--------|---|---|
| [Factor 1] | [cause] | [prevention] |
| [Factor 2] | [cause] | [prevention] |

**Example:**
```
Factor: No automated witness generation
Why: Originally a manual research project; automation not prioritized until production
How to prevent: CI gate that enforces witness capture on every release tag
```

### 2.2 Classification

**Type:** 
- [ ] **Design gap** (architecture not defined)
- [ ] **Implementation gap** (design exists, code missing)
- [ ] **Testing gap** (code exists, verification missing)
- [ ] **Documentation gap** (code works, spec not written)
- [ ] **Process gap** (capability exists, not automated)

**Severity of root cause:**
- [ ] **Critical:** Blocks all progress until fixed
- [ ] **High:** Prevents most progress
- [ ] **Medium:** Creates workarounds but inefficient
- [ ] **Low:** Nice-to-have improvement

---

## 3. Closure Solution

### 3.1 What Closes This Gap

**Solution statement:** [One sentence describing the fix]

**Example:**
```
Create automated witness generation script (tools/witness_apkc_build.sh) that captures
compilation environment, compiler invocation, and binary SHA256 hash on every build.
Integrate into CI so witness is auto-generated with each release tag.
```

### 3.2 High-Level Approach

**Steps:**
1. [Major step 1]
2. [Major step 2]
3. [Major step 3]
4. [Verification step]

**Example:**
```
1. Design witness capture mechanism (what to capture, format)
2. Implement tools/witness_apkc_build.sh script (captures: commit, toolchain, hash, date)
3. Integrate into CI .github/workflows/ci.yml (auto-run on version tags)
4. Verify with 2-3 manual runs (ensure outputs are consistent, hashes match across runs)
5. Document in docs/proofs/WITNESS_APKC_BUILD_<date>.md (evidence format)
```

### 3.3 Detailed Implementation Plan

**Task 1: Witness Capture Script**
- **File:** `tools/witness_apkc_build.sh`
- **Responsible:** [Phase/team]
- **Deliverable:** Executable script that:
  - Captures git commit SHA
  - Captures compiler version (`gcc --version`)
  - Builds apkc with exact flags
  - Computes SHA256(binary)
  - Outputs JSON: `{commit, date_utc, compiler_version, binary_hash}`
- **Effort:** 2-3 hours
- **Testing:** Run 3× with same commit, verify output is identical JSON

**Task 2: CI Integration**
- **File:** `.github/workflows/ci.yml`
- **Responsible:** CI/ops team
- **Deliverable:** New step in CI workflow:
  - Triggered: on every push to `main` or tag creation
  - Runs: `bash tools/witness_apkc_build.sh > /tmp/witness.json`
  - Uploads artifact: witness.json to GitHub Actions artifacts
  - Fail condition: witness generation fails → CI fails
- **Effort:** 1-2 hours
- **Testing:** Create test tag, verify step runs and artifact uploaded

**Task 3: Documentation**
- **File:** `docs/proofs/WITNESS_APKC_BUILD_<date>.md`
- **Responsible:** Documentation team
- **Deliverable:** Template for witness evidence (per TEMPLATE_PROOF.md)
- **Effort:** 1 hour

### 3.4 Acceptance Criteria

The gap is closed when ALL of these are true:

- [ ] Script implemented: `tools/witness_apkc_build.sh` exists and runs without error
- [ ] Script produces correct output: JSON with all required fields (commit, compiler, hash, date)
- [ ] Determinism verified: running script 3× from same commit produces identical JSON
- [ ] CI integrated: witness generation step exists in `.github/workflows/ci.yml`
- [ ] CI triggered: at least 1 CI run has successfully generated witness artifact
- [ ] Evidence captured: witness evidence saved in `docs/proofs/WITNESS_*.md`
- [ ] Code review approved: PR with script + CI changes reviewed by tech lead
- [ ] Tests passing: all related tests pass (unit + integration)
- [ ] Documentation complete: SPEC_* + ADR_* docs link to this closure

---

## 4. Implementation Timeline & Ownership

### 4.1 Phase Assignment

**This gap is part of:** [Phase, e.g., "Phase 46-48 (Production Integration)"]

**Owner:** [Person/team responsible]

**Blockers:**
- [ ] Depends on: [gap that must close first]
- [ ] Blocked by: [external dependency]
- [ ] No blockers, can start immediately

### 4.2 Gantt-Style Timeline

```
Week 1:
  [Mon-Tue] Design witness format (0.5d) + implement script (1.5d)
  [Wed]     Test script locally (0.5d)
  [Thu-Fri] Integrate into CI (1.5d)

Week 2:
  [Mon]     Verify CI runs successfully (0.5d)
  [Tue-Wed] Write documentation (1.5d)
  [Thu]     Code review (0.5d)
  [Fri]     Merge to main (0.25d)

Timeline: 2 weeks
Effort: ~8 person-days
```

### 4.3 Milestones

| Milestone | Date | Status |
|---|---|---|
| Design complete | 2026-08-17 | ⊘ PENDING |
| Script working locally | 2026-08-19 | ⊘ PENDING |
| CI integration complete | 2026-08-22 | ⊘ PENDING |
| PR reviewed & merged | 2026-08-23 | ⊘ PENDING |
| Gap marked CLOSED | 2026-08-24 | ⊘ PENDING |

---

## 5. Evidence & Verification

### 5.1 How We Know Gap Is Closed

**Verification checklist:**

- [ ] **Artifact exists:** `tools/witness_apkc_build.sh` can be executed
  - Check: `bash tools/witness_apkc_build.sh --help`
  - Expected: script runs, no errors

- [ ] **Output is correct:** JSON witness contains required fields
  - Check: `bash tools/witness_apkc_build.sh | jq .`
  - Expected: `{commit, date_utc, compiler_version, binary_hash, status: "OK"}`

- [ ] **Determinism verified:** 3 runs from same commit produce identical output
  - Check: 
    ```bash
    bash tools/witness_apkc_build.sh > /tmp/w1.json
    bash tools/witness_apkc_build.sh > /tmp/w2.json
    bash tools/witness_apkc_build.sh > /tmp/w3.json
    diff /tmp/w1.json /tmp/w2.json && diff /tmp/w2.json /tmp/w3.json && echo "PASS"
    ```
  - Expected: All diffs empty, script prints "PASS"

- [ ] **CI integration verified:** GitHub Actions workflow runs script
  - Check: View CI run logs for workflow step
  - Expected: Step completes successfully, artifact uploaded

- [ ] **Evidence documented:** Witness saved in docs/proofs/
  - Check: `ls docs/proofs/WITNESS_APKC_BUILD_*.md`
  - Expected: At least 1 file exists with proper content (per TEMPLATE_PROOF.md)

- [ ] **Code review approved:** PR merged with tech lead sign-off
  - Check: GitHub PR history
  - Expected: PR merged, approvals recorded, no outstanding comments

### 5.2 Related Witness Documents

This closure produces evidence in:

| Witness | Purpose | Location |
|---|---|---|
| WITNESS_APKC_BUILD_[date] | Proof that script works | docs/proofs/WITNESS_APKC_BUILD_20260824.md |
| WITNESS_DETERMINISM_[date] | Proof that output is reproducible | docs/proofs/WITNESS_DETERMINISM_20260824.md |
| CI workflow run | Proof that CI integration works | GitHub Actions run #[N] |

### 5.3 Test Cases

**Unit test:** `tests/test_witness_generation.c`
```c
// Test 1: Script runs without error
EXPECT_EQ(run_witness_script(), 0);

// Test 2: Output JSON is valid
EXPECT_TRUE(json_valid(witness_output));

// Test 3: Required fields present
EXPECT_NE(json_get(witness_output, "commit"), NULL);
EXPECT_NE(json_get(witness_output, "binary_hash"), NULL);
EXPECT_NE(json_get(witness_output, "compiler_version"), NULL);

// Test 4: Determinism (run 3×, compare)
EXPECT_EQ(witness_output_1, witness_output_2);
EXPECT_EQ(witness_output_2, witness_output_3);
```

**Integration test:** `scripts/test_witness_ci_integration.sh`
```bash
# Simulate CI trigger
git tag -a v1.0.test-witness -m "test"
git push origin v1.0.test-witness
# Wait for CI run...
# Verify artifact uploaded
gh run view [run-id] --json artifacts
# Verify artifact contains witness
```

---

## 6. Trade-offs & Alternatives Considered

### 6.1 Alternatives Rejected

| Alternative | Why Considered? | Why Rejected? |
|---|---|---|
| Manual witness capture | Simple to start | Not scalable, error-prone, manual overhead |
| External service (SignalDB, etc.) | Professional, audited | Introduces external dependency, not freestanding |
| Blockchain attestation | Immutable record | Overkill complexity, no production use case |

### 6.2 Trade-offs Accepted

- **Complexity vs. Automation:** Script is simple (~50 lines) but requires CI setup
  - Trade-off: Initial setup overhead (2-3 days) but then automatic forever
  
- **Determinism vs. Real-Time:** Cannot change witness format mid-release (would break reproducibility)
  - Trade-off: Witness schema is fixed; evolving format requires versioning

---

## 7. Known Risks & Mitigation

### 7.1 Implementation Risks

| Risk | Probability | Impact | Mitigation |
|---|---|---|---|
| Script fails on some platforms | Medium | CI broken | Test on x86_64, ARM64, both gcc + clang |
| Witness JSON becomes outdated | Low | Historical records unreadable | Version witness schema, maintain compat |
| Performance regression | Low | CI slower | Benchmark: script should run <2 seconds |

### 7.2 Deployment Risks

| Risk | Probability | Impact | Mitigation |
|---|---|---|---|
| CI workflow breaks | Low | No witness generated | Test in staging before rolling to production |
| Artifact storage full | Very low | CI fails silently | Monitor artifact size, enforce retention policy |

---

## 8. Lessons Learned & Prevention

### 8.1 What Allowed This Gap to Exist

```
1. Initial project was research-focused (proof-of-concept)
   → Automation not prioritized until production phase

2. Manual witness capture worked initially
   → Inertia: "if it works, why automate?"

3. No CI gate requiring witness
   → Gap remained invisible (not a build blocker)
```

### 8.2 How to Prevent Similar Gaps

**Process change 1: Shift-Left Automation**
- On Day 1 of any new feature: ask "how is this verified?"
- Require CI gate + automated test before merge

**Process change 2: Gap Review Cadence**
- Quarterly: scan for VOID/PENDING/AUDIT states
- Assign owners to close highest-impact gaps

**Process change 3: Acceptance Criteria Template**
- Every task must include: witness artifact, CI gate, documentation
- Merge blocked if acceptance criteria not met

---

## 9. Sign-Off & Approval

### 9.1 Implementation Sign-Off

| Role | Name | Date | Status |
|---|---|---|---|
| **Implementer** | [Name] | 2026-08-24 | ⊘ PENDING |
| **Code Reviewer** | [Name] | TBD | ⊘ PENDING |
| **Tech Lead** | [Name] | TBD | ⊘ PENDING |
| **Product Owner** | [Name] | TBD | ⊘ PENDING |

### 9.2 Verification Sign-Off

- [ ] **Implementer:** "I have verified that all acceptance criteria (section 3.4) are met"
  - Signed: [name] on [date]
  
- [ ] **Code Reviewer:** "I have reviewed the implementation and approve merging"
  - Signed: [name] on [date]
  
- [ ] **Tech Lead:** "I approve this gap closure as satisfying the original requirement"
  - Signed: [name] on [date]

### 9.3 Final Status

**Gap Status:** 🟢 CLOSED  
**Closed Date:** 2026-08-24  
**Evidence Link:** [Link to this CLOSURE document]  
**Related PR:** [GitHub PR #NNN]  
**Related Witness:** [Link to docs/proofs/WITNESS_*.md]  

---

## 10. Transition & Monitoring

### 10.1 Post-Closure Maintenance

After gap is closed, monitor for regression:

**Weekly checks:**
- Verify witness script still runs cleanly
- Spot-check JSON output format
- Confirm CI step still passing

**Monthly review:**
- Verify determinism still holds (run test witness 3×)
- Check artifact storage growth
- Review any witness failures

### 10.2 Escalation

If gap re-opens (witness stops working):

1. **Immediate:** File bug, investigate root cause
2. **Escalate:** Notify tech lead if CI broken
3. **Rollback:** If fix is >1 hour, revert to previous witness version
4. **Document:** Update LESSONS LEARNED section

### 10.3 Related Gap Dependencies

This gap closure enables closing:

- [Gap Y] (depends on witness infrastructure)
- [Gap Z] (uses witness format)

---

**End of CLOSURE Template**

**To use this template:**
1. Copy to `docs/closures/CLOSURE_[GapID]_[Title].md`
2. Fill in all sections (especially 1-3, 4, 9)
3. Complete implementation (section 4)
4. Run verification checks (section 5)
5. Get all sign-offs (section 9)
6. Link from GAP_INDEX_COMPLETE_AUDIT.md (change status from OPEN to CLOSED)
7. Commit as immutable record
