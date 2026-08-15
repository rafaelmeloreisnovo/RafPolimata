/* sec_hardening_gates.h — Runtime Hardening Gates (Stage 11.4)
 *
 * Fail-closed barriers: reject unsafe state transitions.
 * Integrity seals: deterministic non-cryptographic corruption markers.
 * Audit checkpoints: logging for all security-relevant operations.
 * Tamper detection: identify and block corrupted state.
 *
 * IMPORTANT: the built-in 64-bit seal is NOT cryptographic authentication,
 * SHA-256, MAC, or signature. It is an integrity substitute only.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_HARDENING_GATES_H
#define APKC_SEC_HARDENING_GATES_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#define HARDEN_INTEGRITY_CRYPTOGRAPHIC 0
#define HARDEN_INTEGRITY_CAPABILITY "INTEGRITY_SUBSTITUTE_ONLY"

/* Hardening gate status */
enum GateStatus {
	GATE_PASS = 0,
	GATE_REJECT = 1,
	GATE_ALARM = 2,
	GATE_AUDIT = 3
};

/* Integrity seal: deterministic corruption marker, not authentication. */
struct IntegritySeal {
	u64 seal_value;
	u64 state_hash;
	u32 seal_counter;
};

struct AuditCheckpoint {
	u32 timestamp;
	u64 state_hash;
	const char *operation;
	u8 gate_passed;
};

struct AuditLog {
	struct AuditCheckpoint checkpoints[128];
	u32 checkpoint_count;
	u32 next_write_idx;
};

struct HardeningContext {
	struct IntegritySeal seal;
	struct AuditLog audit_log;
	u8 fail_closed;
	u8 tamper_detected;
	u64 operations_count;
};

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

/* Deterministic 64-bit integrity substitute; not collision-resistant crypto. */
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

/* Seal state. Counter ownership stays with harden_gate_monotonic(). */
static inline void harden_seal_state(
	struct HardeningContext *hc,
	const u8 *state,
	u32 state_len) {
	if (!hc || !state) return;
	hc->seal.state_hash = harden_hash(state, state_len);
	hc->seal.seal_value = hc->seal.state_hash ^ 0xdeadbeefcafebabeULL;
}

/* Verify an existing seal. An unsealed context is not a successful verify. */
static inline u8 harden_verify_seal(
	struct HardeningContext *hc,
	const u8 *state,
	u32 state_len) {
	if (!hc || !state) return GATE_REJECT;
	if (hc->seal.seal_value == 0) return GATE_REJECT;

	u64 current_hash = harden_hash(state, state_len);
	u64 expected_seal = current_hash ^ 0xdeadbeefcafebabeULL;
	if (hc->seal.seal_value != expected_seal ||
		hc->seal.state_hash != current_hash) {
		hc->tamper_detected = 1;
		return GATE_REJECT;
	}
	return GATE_PASS;
}

static inline u8 harden_audit(
	struct HardeningContext *hc,
	const char *operation,
	const u8 *state,
	u32 state_len,
	u8 gate_passed) {
	if (!hc || !operation || (!state && state_len != 0)) return GATE_REJECT;
	struct AuditCheckpoint *cp = &hc->audit_log.checkpoints[hc->audit_log.next_write_idx];
	cp->timestamp = 0;
	cp->state_hash = state ? harden_hash(state, state_len) : 0;
	cp->operation = operation;
	cp->gate_passed = gate_passed;
	hc->audit_log.next_write_idx = (hc->audit_log.next_write_idx + 1) % 128;
	if (hc->audit_log.checkpoint_count < 128) hc->audit_log.checkpoint_count++;
	return GATE_AUDIT;
}

static inline struct AuditCheckpoint *harden_get_audit(
	struct HardeningContext *hc,
	u32 index) {
	if (!hc || index >= hc->audit_log.checkpoint_count) return 0;
	return &hc->audit_log.checkpoints[index];
}

static inline u8 harden_gate_precondition(
	struct HardeningContext *hc,
	u8 condition) {
	if (!hc) return GATE_REJECT;
	if (!condition) return hc->fail_closed ? GATE_REJECT : GATE_PASS;
	return GATE_PASS;
}

static inline u8 harden_gate_postcondition(
	struct HardeningContext *hc,
	u8 condition) {
	if (!hc) return GATE_REJECT;
	if (!condition) {
		if (hc->fail_closed) {
			hc->tamper_detected = 1;
			return GATE_REJECT;
		}
		return GATE_PASS;
	}
	return GATE_PASS;
}

static inline u8 harden_gate_monotonic(
	struct HardeningContext *hc,
	u32 current_counter) {
	if (!hc) return GATE_REJECT;
	if (current_counter <= hc->seal.seal_counter) return GATE_REJECT;
	hc->seal.seal_counter = current_counter;
	return GATE_PASS;
}

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
		if (from_state == expected_from && to_state == expected_to) return GATE_PASS;
	}
	return GATE_REJECT;
}

static inline u8 harden_detect_tampering(struct HardeningContext *hc) {
	if (!hc) return 1;
	if (hc->tamper_detected) return 1;
	u32 i;
	for (i = 1; i < hc->audit_log.checkpoint_count; i++) {
		struct AuditCheckpoint *prev = &hc->audit_log.checkpoints[i - 1];
		struct AuditCheckpoint *curr = &hc->audit_log.checkpoints[i];
		if (!prev->gate_passed && curr->gate_passed) return 1;
	}
	return 0;
}

/*
 * Composite gate.
 * Bootstrap is explicit: the first checkpoint may establish a baseline seal,
 * but a direct verification call on an unsealed context always rejects.
 */
static inline u8 harden_checkpoint(
	struct HardeningContext *hc,
	const char *operation,
	const u8 *state,
	u32 state_len,
	u8 precondition,
	u8 postcondition,
	u32 counter) {
	if (!hc || !operation || !state) return GATE_REJECT;

	if (harden_gate_precondition(hc, precondition) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	/* Only a pristine context may bootstrap without a prior seal. */
	if (hc->seal.seal_value != 0) {
		if (harden_verify_seal(hc, state, state_len) == GATE_REJECT) {
			harden_audit(hc, operation, state, state_len, 0);
			return GATE_REJECT;
		}
	} else if (hc->seal.state_hash != 0 || hc->seal.seal_counter != 0) {
		hc->tamper_detected = 1;
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	if (harden_gate_monotonic(hc, counter) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	if (harden_gate_postcondition(hc, postcondition) == GATE_REJECT) {
		harden_audit(hc, operation, state, state_len, 0);
		return GATE_REJECT;
	}

	harden_seal_state(hc, state, state_len);
	harden_audit(hc, operation, state, state_len, 1);
	hc->operations_count++;
	return GATE_PASS;
}

struct HardeningReport {
	u8 fail_closed;
	u8 tamper_detected;
	u64 operations_count;
	u32 audit_checkpoints;
	u32 seal_counter;
	u64 current_seal;
};

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
