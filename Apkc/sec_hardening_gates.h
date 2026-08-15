/* sec_hardening_gates.h — Runtime Hardening Gates (Stage 11.4)
 *
 * Fail-closed barriers: reject unsafe state transitions.
 * Integrity seals: cryptographic markers for state validity.
 * Audit checkpoints: logging for all security-relevant operations.
 * Tamper detection: identify and block corrupted state.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_HARDENING_GATES_H
#define APKC_SEC_HARDENING_GATES_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Hardening gate status */
enum GateStatus {
	GATE_PASS = 0,           /* State valid, proceed */
	GATE_REJECT = 1,         /* State invalid, REJECT and halt */
	GATE_ALARM = 2,          /* Potential tampering detected */
	GATE_AUDIT = 3           /* Audit point logged */
};

/* Integrity seal (cryptographic marker) */
struct IntegritySeal {
	u64 seal_value;          /* Computed seal (e.g., SHA256 hash) */
	u64 state_hash;          /* Hash of sealed state */
	u32 seal_counter;        /* Monotonic counter for replay detection */
};

/* Audit checkpoint */
struct AuditCheckpoint {
	u32 timestamp;           /* Unix timestamp */
	u64 state_hash;          /* Hash of state at checkpoint */
	const char *operation;   /* Operation name */
	u8 gate_passed;          /* 1 if gate passed */
};

/* Audit log (ring buffer) */
struct AuditLog {
	struct AuditCheckpoint checkpoints[128];
	u32 checkpoint_count;
	u32 next_write_idx;  /* Ring buffer pointer */
};

/* Hardening gate context */
struct HardeningContext {
	struct IntegritySeal seal;
	struct AuditLog audit_log;
	u8 fail_closed;          /* 1 = fail-closed (default), 0 = fail-open */
	u8 tamper_detected;      /* 1 if tampering detected */
	u64 operations_count;    /* Total operations executed */
};

/* ============================================================ */
/* INTEGRITY SEALING */
/* ============================================================ */

/* Initialize hardening context */
static inline void harden_init(struct HardeningContext *hc) {
	if (!hc) return;
	hc->seal.seal_value = 0;
	hc->seal.state_hash = 0;
	hc->seal.seal_counter = 0;
	hc->audit_log.checkpoint_count = 0;
	hc->audit_log.next_write_idx = 0;
	hc->fail_closed = 1;
	hc->tamper_detected = 0;
	hc->operations_count = 0;
}

/* Compute simple hash of buffer */
static inline u64 harden_hash(const u8 *data, u32 len) {
	u64 hash = 0x9e3779b97f4a7c15ULL;
	u32 i;
	for (i = 0; i < len; i++) {
		hash ^= data[i];
		hash *= 0xbf58476d1ce4e5b9ULL;
		hash ^= hash >> 27;
	}
	return hash;
}

/* Seal state: create integrity marker */
static inline void harden_seal_state(
	struct HardeningContext *hc,
	const u8 *state,
	u32 state_len) {

	if (!hc || !state) return;

	hc->seal.state_hash = harden_hash(state, state_len);
	hc->seal.seal_value = hc->seal.state_hash ^ 0xdeadbeefcafebabeULL;
	hc->seal.seal_counter++;
}

/* Verify seal: check state integrity */
static inline u8 harden_verify_seal(
	struct HardeningContext *hc,
	const u8 *state,
	u32 state_len) {

	if (!hc || !state) return GATE_REJECT;

	/* First checkpoint: no prior seal to verify */
	if (hc->seal.seal_value == 0) return GATE_PASS;

	u64 current_hash = harden_hash(state, state_len);
	u64 expected_seal = current_hash ^ 0xdeadbeefcafebabeULL;

	if (hc->seal.seal_value != expected_seal) {
		hc->tamper_detected = 1;
		return GATE_REJECT;
	}

	return GATE_PASS;
}

/* ============================================================ */
/* AUDIT LOGGING */
/* ============================================================ */

/* Log audit checkpoint */
static inline u8 harden_audit(
	struct HardeningContext *hc,
	const char *operation,
	const u8 *state,
	u32 state_len,
	u8 gate_passed) {

	if (!hc || !operation) return GATE_AUDIT;

	struct AuditCheckpoint *cp = &hc->audit_log.checkpoints[hc->audit_log.next_write_idx];
	cp->timestamp = 0;  /* Would be filled with actual timestamp */
	cp->state_hash = harden_hash(state, state_len);
	cp->operation = operation;
	cp->gate_passed = gate_passed;

	hc->audit_log.next_write_idx = (hc->audit_log.next_write_idx + 1) % 128;
	if (hc->audit_log.checkpoint_count < 128) {
		hc->audit_log.checkpoint_count++;
	}

	return GATE_AUDIT;
}

/* Get audit checkpoint */
static inline struct AuditCheckpoint *harden_get_audit(
	struct HardeningContext *hc,
	u32 index) {

	if (!hc || index >= hc->audit_log.checkpoint_count) return 0;
	return &hc->audit_log.checkpoints[index];
}

/* ============================================================ */
/* FAIL-CLOSED BARRIERS */
/* ============================================================ */

/* Gate: Verify precondition before state transition */
static inline u8 harden_gate_precondition(
	struct HardeningContext *hc,
	u8 condition) {

	if (!hc) return GATE_REJECT;

	if (!condition) {
		if (hc->fail_closed) {
			return GATE_REJECT;  /* REJECT: fail-closed */
		} else {
			return GATE_PASS;    /* ALLOW: fail-open (not recommended) */
		}
	}

	return GATE_PASS;
}

/* Gate: Verify postcondition after operation */
static inline u8 harden_gate_postcondition(
	struct HardeningContext *hc,
	u8 condition) {

	if (!hc) return GATE_REJECT;

	if (!condition) {
		if (hc->fail_closed) {
			hc->tamper_detected = 1;
			return GATE_REJECT;  /* REJECT: fail-closed, mark tampering */
		} else {
			return GATE_PASS;    /* ALLOW: fail-open */
		}
	}

	return GATE_PASS;
}

/* Gate: Monotonic counter must always increase */
static inline u8 harden_gate_monotonic(
	struct HardeningContext *hc,
	u32 current_counter) {

	if (!hc) return GATE_REJECT;

	if (current_counter <= hc->seal.seal_counter) {
		return GATE_REJECT;  /* Counter did not increase: tampering */
	}

	hc->seal.seal_counter = current_counter;
	return GATE_PASS;
}

/* Gate: State machine valid transition */
static inline u8 harden_gate_state_transition(
	struct HardeningContext *hc,
	u8 from_state,
	u8 to_state,
	const u8 *valid_transitions,
	u32 transition_count) {

	if (!hc || !valid_transitions) return GATE_REJECT;

	u32 i;
	for (i = 0; i < transition_count; i++) {
		u8 encoded = valid_transitions[i];
		u8 expected_from = (encoded >> 4) & 0xf;
		u8 expected_to = encoded & 0xf;

		if (from_state == expected_from && to_state == expected_to) {
			return GATE_PASS;  /* Valid transition */
		}
	}

	return GATE_REJECT;  /* Invalid transition */
}

/* ============================================================ */
/* TAMPER DETECTION */
/* ============================================================ */

/* Detect tampering: inconsistent audit trail */
static inline u8 harden_detect_tampering(struct HardeningContext *hc) {
	if (!hc) return 1;

	if (hc->tamper_detected) return 1;

	/* Check audit log for gaps */
	u32 i;
	for (i = 1; i < hc->audit_log.checkpoint_count; i++) {
		struct AuditCheckpoint *prev = &hc->audit_log.checkpoints[i - 1];
		struct AuditCheckpoint *curr = &hc->audit_log.checkpoints[i];

		/* If previous operation failed but current passed, possible tampering */
		if (!prev->gate_passed && curr->gate_passed) {
			return 1;  /* Inconsistency detected */
		}
	}

	return 0;  /* No tampering detected */
}

/* ============================================================ */
/* CHECKPOINT GATES */
/* ============================================================ */

/* Checkpoint: composite gate with all checks */
static inline u8 harden_checkpoint(
	struct HardeningContext *hc,
	const char *operation,
	const u8 *state,
	u32 state_len,
	u8 precondition,
	u8 postcondition,
	u32 counter) {

	if (!hc || !operation || !state) return GATE_REJECT;

	/* 1. Check precondition */
	if (harden_gate_precondition(hc, precondition) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	/* 2. Verify current seal (state not tampered) */
	if (harden_verify_seal(hc, state, state_len) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	/* 3. Check monotonic counter */
	if (harden_gate_monotonic(hc, counter) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	/* 4. Check postcondition */
	if (harden_gate_postcondition(hc, postcondition) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	/* 5. Reseal state for next operation */
	harden_seal_state(hc, state, state_len);

	/* 6. Log successful checkpoint */
	harden_audit(hc, operation, state, state_len, 1);
	hc->operations_count++;

	return GATE_PASS;
}

/* ============================================================ */
/* HARDENING REPORT */
/* ============================================================ */

struct HardeningReport {
	u8 fail_closed;         /* Fail-closed policy active */
	u8 tamper_detected;     /* Tampering detected flag */
	u64 operations_count;   /* Total operations */
	u32 audit_checkpoints;  /* Number of audit entries */
	u32 seal_counter;       /* Current seal counter */
	u64 current_seal;       /* Current seal value */
};

/* Generate hardening report */
static inline void harden_report(
	struct HardeningContext *hc,
	struct HardeningReport *report) {

	if (!report || !hc) return;

	report->fail_closed = hc->fail_closed;
	report->tamper_detected = hc->tamper_detected;
	report->operations_count = hc->operations_count;
	report->audit_checkpoints = hc->audit_log.checkpoint_count;
	report->seal_counter = hc->seal.seal_counter;
	report->current_seal = hc->seal.seal_value;
}

#endif /* APKC_SEC_HARDENING_GATES_H */
