/* Apkc/hardening_boundary_gates.h — Freestanding boundary validation gates
 *
 * No malloc, no libc, no abstractions. Pure loops, flags, symbols.
 * Validates source and APK capacity limits with exact fail-closed semantics.
 */

#ifndef HARDENING_BOUNDARY_GATES_H
#define HARDENING_BOUNDARY_GATES_H 1

#include <stdint.h>
#include <stddef.h>

#define HARDENING_SOURCE_CAP 0x100000UL  /* 1 MiB source limit */
#define HARDENING_APK_CAP    0x1000000UL /* 16 MiB APK limit */
#define HARDENING_PROOF_MAX  0x10000UL   /* 64 KiB proof buffer */

/* Boundary gate state: one flag set, never combined */
enum boundary_gate_state {
	BOUNDARY_PASS = 0,
	BOUNDARY_SOURCE_EXCEEDS_CAP = 1,
	BOUNDARY_SOURCE_READ_ERROR = 2,
	BOUNDARY_APK_EXCEEDS_CAP = 3,
	BOUNDARY_PROOF_EXCEEDS_CAP = 4,
	BOUNDARY_INVALID_OFFSET = 5,
};

/* Single source boundary check: no malloc, no callbacks, just flags */
struct boundary_gate_source {
	uint64_t bytes_read;
	uint64_t cap_limit;
	enum boundary_gate_state state;
	uint8_t saw_overflow_probe;
};

/* Single APK boundary check */
struct boundary_gate_apk {
	uint64_t total_size;
	uint64_t cap_limit;
	enum boundary_gate_state state;
	uint8_t section_count;
};

/* Proof record boundary */
struct boundary_gate_proof {
	uint64_t record_count;
	uint64_t bytes_used;
	uint64_t cap_limit;
	enum boundary_gate_state state;
};

/* === Source boundary gate === */

static inline void boundary_source_init(struct boundary_gate_source *g, uint64_t cap) {
	g->bytes_read = 0;
	g->cap_limit = cap;
	g->state = BOUNDARY_PASS;
	g->saw_overflow_probe = 0;
}

static inline void boundary_source_advance(struct boundary_gate_source *g, uint64_t n_bytes) {
	if (g->state != BOUNDARY_PASS) return;

	/* Reject if adding n_bytes would exceed cap */
	if (g->bytes_read >= g->cap_limit) {
		g->state = BOUNDARY_SOURCE_EXCEEDS_CAP;
		return;
	}

	/* Reserve 1 byte for NUL terminator */
	uint64_t avail = (g->cap_limit - 1) - g->bytes_read;
	if (n_bytes > avail) {
		g->state = BOUNDARY_SOURCE_EXCEEDS_CAP;
		return;
	}

	g->bytes_read += n_bytes;
}

static inline void boundary_source_probe_overflow(struct boundary_gate_source *g, uint8_t found_more) {
	if (g->state != BOUNDARY_PASS) return;

	/* At capacity: probe for exactly one more byte */
	if (g->bytes_read >= (g->cap_limit - 1)) {
		g->saw_overflow_probe = 1;
		if (found_more) {
			g->state = BOUNDARY_SOURCE_EXCEEDS_CAP;
		}
	}
}

static inline void boundary_source_read_error(struct boundary_gate_source *g) {
	if (g->state != BOUNDARY_PASS) return;
	g->state = BOUNDARY_SOURCE_READ_ERROR;
}

/* === APK boundary gate === */

static inline void boundary_apk_init(struct boundary_gate_apk *g, uint64_t cap) {
	g->total_size = 0;
	g->cap_limit = cap;
	g->state = BOUNDARY_PASS;
	g->section_count = 0;
}

static inline void boundary_apk_add_section(struct boundary_gate_apk *g, uint64_t section_size) {
	if (g->state != BOUNDARY_PASS) return;

	/* Overflow check: if adding section_size would exceed cap */
	if (section_size > g->cap_limit - g->total_size) {
		g->state = BOUNDARY_APK_EXCEEDS_CAP;
		return;
	}

	g->total_size += section_size;
	g->section_count++;
}

static inline uint8_t boundary_apk_fits(struct boundary_gate_apk *g) {
	return g->state == BOUNDARY_PASS && g->total_size <= g->cap_limit;
}

/* === Proof record boundary gate === */

static inline void boundary_proof_init(struct boundary_gate_proof *g, uint64_t cap) {
	g->record_count = 0;
	g->bytes_used = 0;
	g->cap_limit = cap;
	g->state = BOUNDARY_PASS;
}

static inline void boundary_proof_add_record(struct boundary_gate_proof *g, uint64_t record_bytes) {
	if (g->state != BOUNDARY_PASS) return;

	/* Each record must fit within remaining capacity */
	if (record_bytes > g->cap_limit - g->bytes_used) {
		g->state = BOUNDARY_PROOF_EXCEEDS_CAP;
		return;
	}

	g->bytes_used += record_bytes;
	g->record_count++;
}

/* === Manifest gate: validate offset ranges === */

struct boundary_offset_range {
	uint64_t offset_min;
	uint64_t offset_max;
	uint64_t declared_size;
};

static inline uint8_t boundary_offset_valid(const struct boundary_offset_range *range, uint64_t buffer_size) {
	/* Range must not exceed buffer */
	if (range->offset_min > range->offset_max) return 0;
	if (range->offset_max > buffer_size) return 0;

	/* Declared size must fit in range */
	uint64_t range_span = range->offset_max - range->offset_min;
	if (range->declared_size > range_span) return 0;

	return 1;
}

/* === Combined state snapshot for proof === */

struct boundary_gates_snapshot {
	struct boundary_gate_source source;
	struct boundary_gate_apk apk;
	struct boundary_gate_proof proof;
	uint32_t flags;  /* bit 0: source probed, bit 1: apk finalized, bit 2: proof sealed */
	uint8_t all_pass;
};

static inline void boundary_snapshot_init(struct boundary_gates_snapshot *snap,
	uint64_t src_cap, uint64_t apk_cap, uint64_t proof_cap)
{
	boundary_source_init(&snap->source, src_cap);
	boundary_apk_init(&snap->apk, apk_cap);
	boundary_proof_init(&snap->proof, proof_cap);
	snap->flags = 0;
	snap->all_pass = 1;
}

static inline uint8_t boundary_snapshot_all_pass(const struct boundary_gates_snapshot *snap) {
	if (snap->source.state != BOUNDARY_PASS) return 0;
	if (snap->apk.state != BOUNDARY_PASS) return 0;
	if (snap->proof.state != BOUNDARY_PASS) return 0;
	return 1;
}

#endif /* HARDENING_BOUNDARY_GATES_H */
