/* Apkc/hardening_fail_closed.h — Fail-closed semantics for ApkC gates
 *
 * No malloc, no libc, no abstractions. Direct control flow gates.
 * Ensures safe failure modes: never promote unknown state to PASS.
 */

#ifndef HARDENING_FAIL_CLOSED_H
#define HARDENING_FAIL_CLOSED_H 1

#include <stdint.h>
#include <stddef.h>

/* Fail-closed result: exactly one state is set */
enum failclosed_result {
	FAILCLOSED_PASS = 0,       /* Explicit verification succeeded */
	FAILCLOSED_FAIL = 1,       /* Gate rejected input */
	FAILCLOSED_TOKEN_VAZIO = 2, /* Evidence missing, no promotion */
	FAILCLOSED_INVALID = 255,  /* Invalid state - must never use */
};

/* Read operation with fail-closed semantics */
struct failclosed_read {
	enum failclosed_result state;
	uint64_t bytes_read;
	uint64_t bytes_expected;
	uint8_t saw_eof;
	uint8_t saw_error;
};

static inline void failclosed_read_init(struct failclosed_read *rd) {
	rd->state = FAILCLOSED_TOKEN_VAZIO;
	rd->bytes_read = 0;
	rd->bytes_expected = 0;
	rd->saw_eof = 0;
	rd->saw_error = 0;
}

static inline void failclosed_read_mark_eof(struct failclosed_read *rd) {
	if (rd->state != FAILCLOSED_TOKEN_VAZIO) return;
	rd->saw_eof = 1;

	/* Only PASS if we read exactly what was expected */
	if (rd->bytes_read == rd->bytes_expected) {
		rd->state = FAILCLOSED_PASS;
	} else {
		rd->state = FAILCLOSED_FAIL;
	}
}

static inline void failclosed_read_mark_error(struct failclosed_read *rd) {
	rd->saw_error = 1;
	rd->state = FAILCLOSED_FAIL;
}

static inline void failclosed_read_advance(struct failclosed_read *rd, uint64_t n) {
	if (rd->state == FAILCLOSED_PASS) return;  /* Already resolved */
	rd->bytes_read += n;
}

/* === Compile operation gate === */

struct failclosed_compile {
	enum failclosed_result state;
	uint32_t exit_code;
	uint8_t has_syntax_error;
	uint8_t has_warning;
	uint8_t object_size_valid;
	uint32_t object_size;
};

static inline void failclosed_compile_init(struct failclosed_compile *cmp) {
	cmp->state = FAILCLOSED_TOKEN_VAZIO;
	cmp->exit_code = 255;  /* unknown */
	cmp->has_syntax_error = 0;
	cmp->has_warning = 0;
	cmp->object_size_valid = 0;
	cmp->object_size = 0;
}

static inline void failclosed_compile_check(struct failclosed_compile *cmp) {
	if (cmp->state != FAILCLOSED_TOKEN_VAZIO) return;

	/* Fail if syntax errors */
	if (cmp->has_syntax_error) {
		cmp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if exit code non-zero */
	if (cmp->exit_code != 0) {
		cmp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if object size invalid */
	if (!cmp->object_size_valid || cmp->object_size == 0) {
		cmp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Only PASS if all checks pass */
	cmp->state = FAILCLOSED_PASS;
}

/* === Link operation gate === */

struct failclosed_link {
	enum failclosed_result state;
	uint32_t exit_code;
	uint8_t has_undefined_symbols;
	uint8_t has_relocation_error;
	uint32_t executable_size;
	uint8_t is_static;
	uint8_t is_relocatable;
};

static inline void failclosed_link_init(struct failclosed_link *lnk) {
	lnk->state = FAILCLOSED_TOKEN_VAZIO;
	lnk->exit_code = 255;
	lnk->has_undefined_symbols = 0;
	lnk->has_relocation_error = 0;
	lnk->executable_size = 0;
	lnk->is_static = 0;
	lnk->is_relocatable = 0;
}

static inline void failclosed_link_check(struct failclosed_link *lnk) {
	if (lnk->state != FAILCLOSED_TOKEN_VAZIO) return;

	/* Fail if undefined symbols (unless relocatable) */
	if (lnk->has_undefined_symbols && !lnk->is_relocatable) {
		lnk->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if relocation errors */
	if (lnk->has_relocation_error) {
		lnk->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if executable is empty */
	if (lnk->executable_size == 0) {
		lnk->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if exit code non-zero */
	if (lnk->exit_code != 0) {
		lnk->state = FAILCLOSED_FAIL;
		return;
	}

	lnk->state = FAILCLOSED_PASS;
}

/* === ZIP/APK creation gate === */

struct failclosed_zip {
	enum failclosed_result state;
	uint32_t exit_code;
	uint8_t has_compression_error;
	uint8_t has_signature_error;
	uint32_t file_count;
	uint64_t zip_size;
	uint8_t central_dir_present;
};

static inline void failclosed_zip_init(struct failclosed_zip *zp) {
	zp->state = FAILCLOSED_TOKEN_VAZIO;
	zp->exit_code = 255;
	zp->has_compression_error = 0;
	zp->has_signature_error = 0;
	zp->file_count = 0;
	zp->zip_size = 0;
	zp->central_dir_present = 0;
}

static inline void failclosed_zip_check(struct failclosed_zip *zp) {
	if (zp->state != FAILCLOSED_TOKEN_VAZIO) return;

	/* Fail if compression error */
	if (zp->has_compression_error) {
		zp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if signature error */
	if (zp->has_signature_error) {
		zp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if no files or central directory */
	if (zp->file_count == 0 || !zp->central_dir_present) {
		zp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if ZIP is too small to be valid */
	if (zp->zip_size < 22) {  /* Minimum ZIP64 central dir size */
		zp->state = FAILCLOSED_FAIL;
		return;
	}

	/* Fail if exit code non-zero */
	if (zp->exit_code != 0) {
		zp->state = FAILCLOSED_FAIL;
		return;
	}

	zp->state = FAILCLOSED_PASS;
}

/* === Manifest barrier: blocks promotion of TOKEN_VAZIO to PASS === */

struct failclosed_manifest_barrier {
	enum failclosed_result read_gate;
	enum failclosed_result compile_gate;
	enum failclosed_result link_gate;
	enum failclosed_result zip_gate;
	uint8_t allow_token_vazio_passthrough;
	enum failclosed_result final_result;
};

static inline void failclosed_manifest_init(struct failclosed_manifest_barrier *bar) {
	bar->read_gate = FAILCLOSED_TOKEN_VAZIO;
	bar->compile_gate = FAILCLOSED_TOKEN_VAZIO;
	bar->link_gate = FAILCLOSED_TOKEN_VAZIO;
	bar->zip_gate = FAILCLOSED_TOKEN_VAZIO;
	bar->allow_token_vazio_passthrough = 0;
	bar->final_result = FAILCLOSED_TOKEN_VAZIO;
}

static inline void failclosed_manifest_compute_result(struct failclosed_manifest_barrier *bar) {
	/* Any FAIL means final result is FAIL */
	if (bar->read_gate == FAILCLOSED_FAIL ||
	    bar->compile_gate == FAILCLOSED_FAIL ||
	    bar->link_gate == FAILCLOSED_FAIL ||
	    bar->zip_gate == FAILCLOSED_FAIL) {
		bar->final_result = FAILCLOSED_FAIL;
		return;
	}

	/* Any TOKEN_VAZIO without passthrough permission means final is TOKEN_VAZIO */
	if (!bar->allow_token_vazio_passthrough) {
		if (bar->read_gate == FAILCLOSED_TOKEN_VAZIO ||
		    bar->compile_gate == FAILCLOSED_TOKEN_VAZIO ||
		    bar->link_gate == FAILCLOSED_TOKEN_VAZIO ||
		    bar->zip_gate == FAILCLOSED_TOKEN_VAZIO) {
			bar->final_result = FAILCLOSED_TOKEN_VAZIO;
			return;
		}
	}

	/* All PASS */
	if (bar->read_gate == FAILCLOSED_PASS &&
	    bar->compile_gate == FAILCLOSED_PASS &&
	    bar->link_gate == FAILCLOSED_PASS &&
	    bar->zip_gate == FAILCLOSED_PASS) {
		bar->final_result = FAILCLOSED_PASS;
		return;
	}

	/* Default: TOKEN_VAZIO (never unknown) */
	bar->final_result = FAILCLOSED_TOKEN_VAZIO;
}

/* === Guard helper: query without modifying === */

static inline uint8_t failclosed_gate_is_pass(enum failclosed_result result) {
	return result == FAILCLOSED_PASS;
}

static inline uint8_t failclosed_gate_is_fail(enum failclosed_result result) {
	return result == FAILCLOSED_FAIL;
}

static inline uint8_t failclosed_gate_is_unknown(enum failclosed_result result) {
	return result == FAILCLOSED_TOKEN_VAZIO;
}

#endif /* HARDENING_FAIL_CLOSED_H */
