<!-- TEMPLATE: Operational Runbook (RUNBOOK_*.md) -->
<!-- LATTICE_POSITION: Documentation/Runbooks -->
<!-- USE THIS TEMPLATE FOR: Step-by-step procedures, emergency response, deployment -->

# RUNBOOK_[Operation]: [What This Procedure Does]

**Date:** YYYY-MM-DD  
**Author(s):** Name(s)  
**Lattice Position:** Operations/[Category]  
**Severity:** 🟢 Low | 🟡 Medium | 🔴 High | 🔥 Critical  
**On-Call:** TBD (who to page if something goes wrong)  
**Escalation:** [escalation contact info]  

---

## 1. Executive Summary

**What:** One-sentence description of this procedure.

**When to use:** Describe the condition/alert that triggers this runbook.

**Duration:** Estimated time to complete (e.g., "5 minutes for manual deploy", "30 seconds for automated rollback").

**Success criteria:** How do you know when this procedure succeeded?

**Example:**
```
What: Deploy RafPolimata compiler v1.0 to production Android device
When: New release tag pushed to main branch
Duration: 10 minutes (CI automated, 2 minutes manual verification)
Success: APK installs, logcat shows "libhello loaded" with no FATAL messages
```

---

## 2. Prerequisites & Safety Checks

### 2.1 Mandatory Prerequisites

Before starting this procedure, verify:

- [ ] System has [tool/permission/resource]
  - Check: `which [tool]` or `ls -la [file]`
  - If missing: [installation command]

- [ ] Network connectivity
  - Check: `ping github.com`
  - If failing: Use offline mode (see section on offline mode)

- [ ] Device/environment ready
  - Check: `adb devices` (if using Android)
  - Expected output: Device listed as "device" (not "unauthorized")

- [ ] Backup created (if modifying state)
  - Command: `cp -r [state-dir] [backup-dir]-$(date -u +%s)`
  - Verification: `ls [backup-dir]*` should show recent backup

### 2.2 Safety Checks (do NOT skip)

Run these checks to prevent data loss:

```bash
# Check 1: Verify we're on the right branch
git rev-parse --abbrev-ref HEAD
# Expected: main (or deployment branch)

# Check 2: Verify no uncommitted changes
git status
# Expected: "working tree clean"

# Check 3: Verify we have the latest code
git fetch origin main
git log -1 --oneline
# Expected: should match remote

# Check 4: Backup existing state
cp -r out/ out.backup.$(date -u +%s)
```

### 2.3 Abort Conditions

**STOP immediately if any of these are true:**
- Network connectivity lost mid-deployment
- Device becomes "unauthorized" or offline
- Backup creation failed
- Insufficient disk space (need 500MB minimum)
- Previous deployment still in progress (check lock file)

**If abort is necessary:** Go to section 5 (Rollback).

---

## 3. Step-by-Step Procedure

### Step 1: [First Major Action]

**Purpose:** [Why we do this step]

**Exact command:**
```bash
[copy-paste-ready command, no abbreviations]
```

**Expected output:**
```
[what success looks like]
```

**Verification:**
- [ ] Command exited with code 0 (success)
- [ ] Output contains expected strings: [key markers]
- [ ] No FATAL or ERROR in output (scan for these strings)

**If failed:** Check section 4 (Troubleshooting) for this step.

**Time estimate:** X minutes

---

### Step 2: [Second Major Action]

**Purpose:** [Why]

**Exact command:**
```bash
[command]
```

**Expected output:**
```
[output]
```

**Verification:**
- [ ] Verification check 1
- [ ] Verification check 2

**If failed:** See section 4, "Step 2 failed".

**Time estimate:** X minutes

---

### Step N: Final Validation

**Purpose:** Ensure the entire procedure succeeded end-to-end.

**Exact command:**
```bash
[validation command]
```

**Expected output:**
```
[success output]
```

**Verification:**
- [ ] All checks pass
- [ ] No errors in logs
- [ ] System in expected final state

---

## 4. Troubleshooting

### 4.1 Step 1 Failed: [Specific Error]

**Symptom:** Error message contains "[key error text]"

**Root causes:**
1. [Cause A] → Check with: `[diagnostic command]`
   - Solution: [fix steps]
   
2. [Cause B] → Check with: `[diagnostic command]`
   - Solution: [fix steps]

**Recovery:** After fix, re-run Step 1

---

### 4.2 Step 2 Failed: [Specific Error]

**Symptom:** [error description]

**Root causes:**
1. [Cause] → Check: [diagnostic]
   - Solution: [fix]

**Recovery:** [re-run steps]

---

### 4.3 Common Issues

| Issue | Symptom | Diagnosis | Fix |
|---|---|---|---|
| Device offline | `adb: device offline` | `adb devices` | Reconnect USB or restart adb: `adb kill-server && adb start-server` |
| Permission denied | `Permission denied: /dev/[file]` | Check: `ls -la /dev/[file]` | Run with: `sudo [command]` (or add user to group) |
| Out of disk space | `No space left on device` | Check: `df -h` | Delete old backups: `rm -rf out.backup.*` |
| Network timeout | `Connection timed out` | Check: `ping github.com` | Retry with: `--retry 3` flag or use offline resources |

---

## 5. Rollback Procedure (If Something Goes Wrong)

**Use this if:**
- Procedure failed midway and cannot be recovered
- You need to revert to previous state immediately
- Time is critical (opt for rollback over troubleshooting)

### 5.1 Quick Rollback (< 1 minute)

```bash
# Restore from backup created in prerequisites
rm -rf out/
cp -r out.backup.[timestamp]/ out/

# Verify rollback succeeded
[validation command from step N]

# Check logs for errors
grep -i error out/*.log || echo "No errors"
```

### 5.2 Full Rollback (if backup unavailable)

```bash
# Reset to previous git commit
git reset --hard origin/main~1

# Rebuild from scratch
make clean
make build

# Verify
[validation command]
```

### 5.3 Emergency Contact

If rollback fails:
1. **Stop the procedure**
2. **Note down:** exact error message, last successful step
3. **Contact:** [on-call engineer or escalation contact]
4. **Do NOT retry** without guidance (risk of cascading failures)

---

## 6. Verification & Monitoring

### 6.1 Post-Procedure Checks (within 5 minutes)

Run these checks to confirm the procedure succeeded:

```bash
# Check 1: System is responsive
curl http://[service] || echo "Service not responding"

# Check 2: No error messages in logs
tail -100 /var/log/[component].log | grep -i error || echo "Clean"

# Check 3: Performance is normal
[perf check command]

# Check 4: Monitoring dashboards
# Go to: [monitoring URL]
# Expected: green/healthy status
```

### 6.2 Ongoing Monitoring (next 24 hours)

Monitor these metrics for degradation:

| Metric | Normal | Alert Threshold | Check Command |
|---|---|---|---|
| CPU usage | <50% | >80% | `top -bn1 \| head -3` |
| Memory usage | <60% | >85% | `free -h` |
| Error rate | <0.1% | >1% | `tail -1000 logs \| grep ERROR \| wc -l` |
| Latency (p99) | <100ms | >500ms | `[perf metric tool]` |

### 6.3 Alerting Rules

If any metric crosses threshold:
1. Check logs for root cause
2. If recoverable, follow troubleshooting section 4
3. If not, initiate rollback (section 5)
4. Notify on-call + tech lead

---

## 7. Communication & Documentation

### 7.1 During the Procedure

**Update status channel (Slack, etc.):**
```
[13:45] Starting deployment of v1.0 to prod
[13:50] Step 1 complete, moving to Step 2
[13:55] All steps complete, running validation
[14:00] Deployment successful ✅
```

### 7.2 After the Procedure

**Log the execution:**
```bash
# Create procedure log
cat > /var/log/deployment-$(date +%Y%m%d-%H%M%S).log << EOF
Date: $(date -u)
Operator: $(whoami)
Procedure: RUNBOOK_[Operation]
Status: SUCCESS | ROLLBACK | FAILURE
Duration: X minutes
Notes: [any issues encountered]
EOF
```

**Update change log:**
```markdown
## [YYYY-MM-DD]
- Deployed v1.0 to production
- Duration: 10 minutes
- No issues
- Validated: all checks passing
```

### 7.3 Post-Execution Report

If procedure took >1 hour or had issues:

```markdown
## Deployment Report: [Operation] [Date]

**Status:** ✅ SUCCESS | ⚠️ PARTIAL | ❌ FAILED

**Timeline:**
- 13:45 - Started deployment
- 14:15 - Network timeout (recovered)
- 14:30 - Deployment complete

**Issues Encountered:**
1. Network timeout on step 2 → resolved by retrying

**Validation:**
- All checks pass
- Error rate normal
- Performance baseline met

**Lessons Learned:**
- [Issue 1]: Next time, increase timeout to X seconds
- [Issue 2]: [Action to prevent recurrence]

**Sign-off:**
- Operator: Alice
- Tech lead: Bob
- Date: 2026-08-15
```

---

## 8. Special Cases

### 8.1 Offline Deployment

If network is unavailable:

```bash
# Use pre-downloaded artifacts
ls /offline-cache/artifacts/
# If available, deploy from cache:
cp /offline-cache/artifacts/app-v1.0.apk out.apk
# Skip steps involving network
```

### 8.2 Partial Failure Recovery

If Step N succeeds but Step N+1 fails:

```bash
# Don't rollback immediately
# Instead, check if Step N state is valid
[diagnostic for step N state]
# If valid, can continue from Step N+1
# If invalid, proceed to section 5 (Rollback)
```

### 8.3 Concurrent Operations

If another deployment is in progress:

```bash
# Check for lock file
ls -la /var/lock/deployment.lock
# If exists, get details:
cat /var/lock/deployment.lock
# Wait for other operator to finish, OR
# If stale (>30 minutes old), remove it:
rm /var/lock/deployment.lock
```

---

## 9. Maintenance & Updates

### 9.1 Review Frequency

- **After each execution:** Review actual steps taken vs. documented steps
- **Monthly:** Verify all verification commands still work
- **Quarterly:** Update for new tools/changes in infrastructure

### 9.2 Update This Runbook When

- [ ] A new tool or service is introduced
- [ ] Procedure takes significantly longer than estimated
- [ ] A failure mode is discovered that wasn't documented
- [ ] New verification steps become available
- [ ] Escalation contact changes

### 9.3 Version Control

```
Version: 1.0 (2026-08-15)
- Initial runbook
- Tested 3× successfully
- Ready for production use

Version 1.1 (2026-08-20)
- Added offline mode (section 8.1)
- Updated device timeout to 30s (was 10s)
- Added monitoring section 6.2
```

---

## 10. Checklist for Runbook Completion

Before considering this runbook "ready for production":

- [ ] Procedure has been executed at least 2 times successfully
- [ ] All verification steps actually work (re-run them)
- [ ] Troubleshooting section covers common failure modes
- [ ] Rollback procedure has been tested (at least in staging)
- [ ] Communication templates are provided
- [ ] Escalation contacts are current and verified
- [ ] On-call handoff has reviewed and approved
- [ ] No credentials or secrets in the text
- [ ] Estimated times are realistic (measured from actual runs)
- [ ] Links to other docs and dashboards are correct

---

**End of Runbook Template**

**To use this template:**
1. Copy to `docs/runbooks/RUNBOOK_[OperationName].md`
2. Fill in all steps (section 3)
3. Execute procedure on test environment 2-3 times
4. Document actual failures and fixes (section 4)
5. Test rollback procedure (section 5)
6. Get approval from tech lead
7. Commit as immutable record
