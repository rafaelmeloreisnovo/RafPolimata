/* diag_error_reporting.h — Error & Warning Diagnostics (Phase 47)
 *
 * Phase 47: Comprehensive error and warning system for semantic analysis
 * - Error and warning definitions with severity levels
 * - Source location tracking with line/column info
 * - Error message formatting with context
 * - Diagnostic aggregation and reporting
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DIAG_ERROR_REPORTING_H
#define APKC_DIAG_ERROR_REPORTING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* DIAGNOSTIC SEVERITY */
/* ============================================================ */

enum DiagSeverity {
	DIAG_NOTE = 0,
	DIAG_WARNING = 1,
	DIAG_ERROR = 2,
	DIAG_FATAL = 3
};

enum DiagnosticCode {
	DIAG_TYPE_MISMATCH = 100,
	DIAG_UNDEFINED_SYMBOL = 101,
	DIAG_AMBIGUOUS_SYMBOL = 102,
	DIAG_INVALID_TYPE = 103,
	DIAG_CONSTRAINT_FAILURE = 104,
	DIAG_UNIFICATION_FAILED = 105,
	DIAG_OCCURS_CHECK_FAILED = 106,
	DIAG_SYMBOL_REDEFINED = 107,
	DIAG_UNREACHABLE_CODE = 200,
	DIAG_UNUSED_VARIABLE = 201,
	DIAG_DEAD_CODE = 202,
	DIAG_USE_AFTER_FREE = 203,
	DIAG_NULL_POINTER = 204,
	DIAG_OUT_OF_BOUNDS = 205,
	DIAG_UNSAFE_OPERATION = 206
};

/* ============================================================ */
/* SOURCE LOCATION */
/* ============================================================ */

struct SourceLocation {
	const char *filename;
	u32 line;
	u32 column;
	u32 byte_offset;
	u32 length;
};

static inline void source_location_init(struct SourceLocation *loc) {
	if (!loc) return;
	loc->filename = 0;
	loc->line = 0;
	loc->column = 0;
	loc->byte_offset = 0;
	loc->length = 0;
}

static inline u8 source_location_set(
	struct SourceLocation *loc,
	const char *filename,
	u32 line,
	u32 column,
	u32 byte_offset) {

	if (!loc || !filename) return 1;
	loc->filename = filename;
	loc->line = line;
	loc->column = column;
	loc->byte_offset = byte_offset;
	loc->length = 1;
	return 0;
}

/* ============================================================ */
/* DIAGNOSTIC MESSAGE */
/* ============================================================ */

struct Diagnostic {
	enum DiagSeverity severity;
	enum DiagnosticCode code;
	const char *message;
	struct SourceLocation location;
	const char *context_line;
	u32 context_length;
	u32 highlight_start;
	u32 highlight_length;
};

static inline u8 diagnostic_init(
	struct Diagnostic *diag,
	enum DiagSeverity severity,
	enum DiagnosticCode code,
	const char *message) {

	if (!diag || !message) return 1;
	diag->severity = severity;
	diag->code = code;
	diag->message = message;
	source_location_init(&diag->location);
	diag->context_line = 0;
	diag->context_length = 0;
	diag->highlight_start = 0;
	diag->highlight_length = 0;
	return 0;
}

static inline u8 diagnostic_set_location(
	struct Diagnostic *diag,
	const char *filename,
	u32 line,
	u32 column) {

	if (!diag) return 1;
	return source_location_set(&diag->location, filename, line, column, 0);
}

static inline u8 diagnostic_set_context(
	struct Diagnostic *diag,
	const char *context_line,
	u32 start,
	u32 length) {

	if (!diag || !context_line) return 1;
	diag->context_line = context_line;
	diag->highlight_start = start;
	diag->highlight_length = length;
	return 0;
}

/* ============================================================ */
/* DIAGNOSTIC BUFFER */
/* ============================================================ */

struct DiagnosticsBuffer {
	struct Diagnostic diags[256];
	u32 diag_count;
	u32 error_count;
	u32 warning_count;
	u32 note_count;
	u8 is_fatal;
};

static inline void diagnostics_init(struct DiagnosticsBuffer *buf) {
	if (!buf) return;
	buf->diag_count = 0;
	buf->error_count = 0;
	buf->warning_count = 0;
	buf->note_count = 0;
	buf->is_fatal = 0;
}

static inline u8 add_diagnostic(
	struct DiagnosticsBuffer *buf,
	struct Diagnostic *diag) {

	if (!buf || !diag || buf->diag_count >= 256) return 1;

	buf->diags[buf->diag_count] = *diag;
	buf->diag_count++;

	switch (diag->severity) {
	case DIAG_NOTE:
		buf->note_count++;
		break;
	case DIAG_WARNING:
		buf->warning_count++;
		break;
	case DIAG_ERROR:
		buf->error_count++;
		break;
	case DIAG_FATAL:
		buf->error_count++;
		buf->is_fatal = 1;
		break;
	}

	return 0;
}

static inline u8 should_halt_compilation(struct DiagnosticsBuffer *buf) {
	if (!buf) return 1;
	return buf->is_fatal || buf->error_count > 0;
}

static inline u32 get_error_count(struct DiagnosticsBuffer *buf) {
	if (!buf) return 0;
	return buf->error_count;
}

static inline u32 get_warning_count(struct DiagnosticsBuffer *buf) {
	if (!buf) return 0;
	return buf->warning_count;
}

/* ============================================================ */
/* ERROR MESSAGE CATEGORIES */
/* ============================================================ */

struct ErrorCategory {
	enum DiagnosticCode code;
	const char *name;
	enum DiagSeverity default_severity;
	const char *description;
};

static inline const char *diagnostic_code_name(enum DiagnosticCode code) {
	switch (code) {
	case DIAG_TYPE_MISMATCH: return "type_mismatch";
	case DIAG_UNDEFINED_SYMBOL: return "undefined_symbol";
	case DIAG_AMBIGUOUS_SYMBOL: return "ambiguous_symbol";
	case DIAG_INVALID_TYPE: return "invalid_type";
	case DIAG_CONSTRAINT_FAILURE: return "constraint_failure";
	case DIAG_UNIFICATION_FAILED: return "unification_failed";
	case DIAG_OCCURS_CHECK_FAILED: return "occurs_check_failed";
	case DIAG_SYMBOL_REDEFINED: return "symbol_redefined";
	case DIAG_UNREACHABLE_CODE: return "unreachable_code";
	case DIAG_UNUSED_VARIABLE: return "unused_variable";
	case DIAG_DEAD_CODE: return "dead_code";
	case DIAG_USE_AFTER_FREE: return "use_after_free";
	case DIAG_NULL_POINTER: return "null_pointer";
	case DIAG_OUT_OF_BOUNDS: return "out_of_bounds";
	case DIAG_UNSAFE_OPERATION: return "unsafe_operation";
	default: return "unknown";
	}
}

static inline const char *severity_name(enum DiagSeverity sev) {
	switch (sev) {
	case DIAG_NOTE: return "note";
	case DIAG_WARNING: return "warning";
	case DIAG_ERROR: return "error";
	case DIAG_FATAL: return "fatal error";
	default: return "unknown";
	}
}

/* ============================================================ */
/* STATISTICS */
/* ============================================================ */

struct DiagnosticsStats {
	u32 total_diags;
	u32 errors;
	u32 warnings;
	u32 notes;
	u32 fatal_errors;
	u32 errors_by_phase[45];
};

static inline void compute_diagnostics_stats(
	struct DiagnosticsBuffer *buf,
	struct DiagnosticsStats *stats) {

	if (!buf || !stats) return;

	stats->total_diags = buf->diag_count;
	stats->errors = buf->error_count;
	stats->warnings = buf->warning_count;
	stats->notes = buf->note_count;
	stats->fatal_errors = buf->is_fatal ? 1 : 0;

	for (u32 i = 0; i < 45; i++) {
		stats->errors_by_phase[i] = 0;
	}
}

#endif /* APKC_DIAG_ERROR_REPORTING_H */
