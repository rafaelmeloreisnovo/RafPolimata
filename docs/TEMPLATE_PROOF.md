<!-- TEMPLATE: Hardcoding Witness (WITNESS_*.md) -->
<!-- LATTICE_POSITION: Documentation/Proofs -->
<!-- USE THIS TEMPLATE FOR: Evidence capture, audit trails, reproducible verification -->

# WITNESS_[ComponentName]_[Date]: [What This Proves]

**Date:** YYYY-MM-DD  
**Witness ID:** WIT-YYYY-MMDD-NNNN  
**Lattice Position:** Documentation/Proofs/[Category]  
**Closes Gap:** [Gap ID from GAP_INDEX_COMPLETE_AUDIT.md, e.g., VOID-C1]  
**Status:** ✅ PASS | ◐ PARTIAL | ⊘ FAILED  

---

## 1. Executive Summary

**Claim:** State what this witness proves in one sentence.

**Example:**
- "Proves that apkc compiler produces bitwise-identical ARM64 ELF binaries across 3 independent compilations."
- "Proves that generated APK installs on ARM64 Android device and executes without crash."
- "Proves that C↔Rust symbol table is consistent across language boundary."

**Evidence Class:** Hardware, Software, Cryptographic, Formal Proof, Execution Trace

---

## 2. Setup & Prerequisites

### 2.1 Environment

| Attribute | Value |
|---|---|
| **Host OS** | Linux 6.18.5-fc-v20 |
| **Architecture** | x86_64 |
| **Compiler** | gcc 13.2.0 (or clang 17.0) |
| **Toolchain** | glibc 2.39 |
| **Device** | (if applicable) Android 14, ARM64 Snapdragon 8 Gen 3 |
| **Network** | (if applicable) stable, <100ms latency |

### 2.2 Preconditions

- Git repository at commit `[SHA-256 hash]`
- All prerequisites compiled (list any build dependencies)
- Device connected and adb accessible (if hardware test)
- SSH keys configured for remote execution (if distributed)

### 2.3 Reproducibility

**Command to reproduce:** (full, copy-paste-ready)

```bash
git clone https://github.com/rafaelmeloreisnovo/RafPolimata.git
cd RafPolimata
git checkout [commit-hash]
bash tools/witness_[name].sh
```

---

## 3. Test Command & Execution

### 3.1 Exact Command Executed

Provide the **verbatim** command (no abbreviations):

```bash
gcc -std=c11 -Wall -Wextra -nostdlib \
    -Wl,-e,_start Apkc/apkc.c -o /tmp/apkc_test_build1

sha256sum /tmp/apkc_test_build1

echo "Build 1 timestamp: $(date -u)"
echo "Build 1 git commit: $(git rev-parse HEAD)"
```

### 3.2 Expected Output (Before Execution)

What the test should produce if it passes:

```
[hash1 for build 1]
Build 1 timestamp: 2026-08-15T14:30:22Z
Build 1 git commit: a1b2c3d4e5f6...
```

### 3.3 Actual Output (After Execution)

Paste the **exact** output from the command:

```
1a2b3c4d5e6f7890abcdef1234567890abcdef1234567890abcdef1234567890  /tmp/apkc_test_build1
Build 1 timestamp: 2026-08-15T14:30:22Z
Build 1 git commit: a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2
```

### 3.4 Comparison & Verdict

| Build | Hash | Status |
|---|---|---|
| Build 1 | `1a2b3c4d5e6f...` | ✅ |
| Build 2 | `1a2b3c4d5e6f...` | ✅ |
| Build 3 | `1a2b3c4d5e6f...` | ✅ |
| **Result** | All match | **✅ DETERMINISM VERIFIED** |

---

## 4. Evidence Chain

### 4.1 Provenance

Cryptographic chain of evidence:

```
Source Code (git commit)
  ↓ [commit hash: abc123def456...]
Compilation (gcc invocation)
  ↓ [compiler version: gcc 13.2.0, flags: -nostdlib -Wall]
Binary Output (apkc executable)
  ↓ [SHA256: 1a2b3c4d5e6f...]
Attestation (timestamped signature)
  ↓ [timestamp: 2026-08-15T14:30:22Z, signer: Claude Code witness]
```

### 4.2 Hash Attestation

**SHA256(apkc):** `1a2b3c4d5e6f7890abcdef1234567890abcdef1234567890abcdef1234567890`

**Signed by:** OpenPGP key [fingerprint], or simple timestamped log

**Verification:**
```bash
echo "1a2b3c4d5e6f7890... /tmp/apkc" | sha256sum -c -
# Output: /tmp/apkc: OK
```

### 4.3 Reproducibility Chain

1. ✅ Source code matches git commit
2. ✅ Compiler toolchain documented (gcc 13.2.0)
3. ✅ Exact flags recorded (see 3.1)
4. ✅ Output hash deterministic (all 3 builds match)
5. ✅ Timestamp & environment captured

**Conclusion:** Evidence chain is unbroken; binary is reproducible from source.

---

## 5. Detailed Results

### 5.1 Hardware Test Example

If testing on device:

```
$ adb devices
List of attached devices:
  FA7AX1A9999  device

$ adb shell monkey -p com.rafael.teste -c android.intent.category.LAUNCHER 1
Events injected: 1

$ adb logcat -d | grep -i "NativeActivity\|fatal\|crash"
[Output captured in next section]
```

### 5.2 Output Logs (Full Capture)

Paste complete output from all stages (may be long, but do not truncate):

```
== Build Stage ==
Compiling Apkc/apkc.c...
gcc: no warning
Linking to /tmp/apkc...
gcc: no error
Output: apkc 145632 bytes

== Run Stage ==
Running apkc on test input...
Input: Apkc/hello.s.txt (245 bytes)
Output: /tmp/hello-signed.apk (12544 bytes)
Status: OK

== Verification Stage ==
Checking binary structure...
unzip -t /tmp/hello-signed.apk
    testing: AndroidManifest.xml   OK
    testing: classes.dex   OK
    testing: lib/arm32-v7a/libhello.so   OK
All files OK

Checking signatures...
jarsigner -verify -verbose /tmp/hello-signed.apk
    145 bytes: classes.dex
    12544 bytes: AndroidManifest.xml
    Signature verified OK
```

---

## 6. Interpretation & Significance

### 6.1 What This Proves

**Positive:** If test passes, this shows that:
- [Specific claim 1]
- [Specific claim 2]
- [Specific claim 3]

**Example:** Determinism test proves:
- Same source code + same compiler + same flags → identical binary
- No timestamp embedding in binary (bitwise identical)
- Reproducible from git commit

### 6.2 What This Does NOT Prove

**Limitations:** This test does **not** show that:
- [What this test cannot verify 1]
- [What this test cannot verify 2]

**Example:** Determinism test does NOT prove:
- That the binary is correct (only that it's reproducible)
- That it works on ARM64 hardware (x86_64 test only)
- That it handles all edge cases

### 6.3 Next Steps

If this test passes, the next verification step is:

"Once this witness completes, run `[next witness name]` to verify [next claim]."

---

## 7. Failure Scenarios & Recovery

### 7.1 If Test Failed

If actual output **differs** from expected:

1. **Diagnosis:** Why might it differ?
   - Compiler version mismatch? (check: `gcc --version`)
   - Different build flags? (check: `echo $CFLAGS`)
   - Nondeterministic code? (check: timestamps, random seeds)
   - Source code changed? (check: `git status`, `git diff`)

2. **Recovery:**
   ```bash
   # Clean rebuild
   make clean
   make test-determinism
   
   # If still fails, capture diff
   hexdump -C /tmp/build1 > /tmp/build1.hex
   hexdump -C /tmp/build2 > /tmp/build2.hex
   diff /tmp/build1.hex /tmp/build2.hex | head -20
   ```

3. **Escalation:** If unresolved, file issue with:
   - Exact command that failed
   - All output logs (this witness)
   - Environment details (compiler version, OS, etc.)
   - Reproducible steps

### 7.2 Partial Success

If some checks pass but others fail:

| Check | Status | Action |
|---|---|---|
| Build 1 hash | ✅ | OK |
| Build 2 hash | ✅ | OK |
| Build 3 hash | ⊘ Different | Investigate (see 7.1) |
| Signature verify | ✅ | OK |
| **Overall** | ◐ **PARTIAL** | Need to fix build 3 |

---

## 8. Cross-Link to Gap Closure

**This witness closes gap:** [Gap ID]

**From GAP_INDEX_COMPLETE_AUDIT.md:**
- Gap: [Description]
- Impact: CRITICAL | HIGH | MEDIUM
- Status Before: ⊘ VOID | PENDING | AUDIT
- Status After: ✅ PASS (this witness confirms it)

**Related Gap Closure Document:** `docs/closures/CLOSURE_[GapID].md`

---

## 9. CI Integration

### 9.1 Automation

This witness can be run automatically:

```bash
# In .github/workflows/ci.yml:
- name: Witness [ComponentName]
  run: bash tools/witness_[name].sh > /tmp/witness.log 2>&1
  
- name: Verify Witness
  run: grep "PASS" /tmp/witness.log && echo "Witness OK" || exit 1
```

### 9.2 Artifact Storage

Upload witness output to CI artifacts:

```yaml
- uses: actions/upload-artifact@v3
  if: always()
  with:
    name: witness-[name]-${{ github.run_number }}
    path: /tmp/witness.log
```

### 9.3 Monitoring

Track witness results over time:

| CI Run | Date | Result | Diff from Previous |
|---|---|---|---|
| #150 | 2026-08-15 | ✅ PASS | None (stable) |
| #149 | 2026-08-14 | ✅ PASS | None |
| #148 | 2026-08-13 | ⊘ FAIL | Regression detected |

---

## 10. Attestation & Sign-Off

### 10.1 Who Generated This Witness

| Role | Name/System | Date | Signature |
|---|---|---|---|
| **Executor** | Claude Code (agent) | 2026-08-15T14:30:22Z | automated |
| **Verifier** | (human) | TBD | ⊘ PENDING |
| **Approver** | (tech lead) | TBD | ⊘ PENDING |

### 10.2 Witness Checklist

- [ ] All prerequisites met (environment, setup)
- [ ] Command is reproducible (copy-paste works)
- [ ] Output is complete (no truncation)
- [ ] Hash values are correct (re-verified)
- [ ] Interpretation is clear (what it proves)
- [ ] Next witness is identified (gap closure chain)
- [ ] CI integration ready (or manual trigger OK)
- [ ] No secrets in logs (check before committing)

### 10.3 Permanence

This witness is **permanent** once committed:
- Stored in: `docs/proofs/WITNESS_[name]_[date].md`
- Git history: immutable record (cannot be edited after merge)
- Replay: can be re-run on any commit by checking out that commit

---

## 11. Appendix: Full Logs (Optional)

If output was very long, put complete unedited logs here:

```
[Full build log, compiler output, device logcat, etc.]
[This section is for reference only, not critical to understanding the proof]
```

---

**End of Witness Document**

**To use this template:**
1. Copy this file to `docs/proofs/WITNESS_[YourComponentName]_[YYYYMMDD].md`
2. Fill in all sections (especially 1, 3.1-3.3, 4)
3. Run the command (section 3.1) and capture actual output
4. Update section 3.3 with actual results
5. Commit to git (immutable record)
