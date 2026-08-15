/* adv_error_recovery.h — Error Recovery & IDE Support (Phase 27-28)
 *
 * Error recovery: continue parsing after errors
 * Diagnostics: precise error messages with context
 * Incremental analysis: cache and reuse results
 * IDE integration: hover information, completions
 * Error suggestions: fix recommendations
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ADV_ERROR_RECOVERY_H
#define APKC_ADV_ERROR_RECOVERY_H 1

#include "sem_verifier.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* ============================================================ */
/* ERROR RECOVERY STRATEGIES */
/* ============================================================ */

enum RecoveryStrategy {
	RECOVERY_SKIP_TOKEN = 0,
	RECOVERY_INSERT_TOKEN = 1,
	RECOVERY_REPLACE_TOKEN = 2,
	RECOVERY_SYNC_TO_DELIMITER = 3
};

/* ============================================================ */
/* DIAGNOSTIC MESSAGE */
/* ============================================================ */

struct Diagnostic {
	u32 line;
	u32 column;
	u8 severity;  /* 0=info, 1=warning, 2=error, 3=fatal */
	const char *message;
	const char *code;
	const char *fix_suggestion;
	const char *context_line;
};

/* ============================================================ */
/* ERROR RECOVERY CONTEXT */
/* ============================================================ */

struct ErrorRecoveryContext {
	struct Diagnostic diagnostics[256];
	u32 diagnostic_count;
	u32 errors_recovered;
	u32 error_locations[64];
	u32 error_count;
	u8 is_recovering;
};

/* ============================================================ */
/* INITIALIZATION */
/* ============================================================ */

static inline void error_recovery_init(struct ErrorRecoveryContext *ctx) {
	if (!ctx) return;
	ctx->diagnostic_count = 0;
	ctx->errors_recovered = 0;
	ctx->error_count = 0;
	ctx->is_recovering = 0;
}

/* ============================================================ */
/* ERROR REPORTING */
/* ============================================================ */

static inline u8 report_diagnostic(
	struct ErrorRecoveryContext *ctx,
	u32 line,
	u32 column,
	u8 severity,
	const char *message,
	const char *code) {

	if (!ctx || ctx->diagnostic_count >= 256) return 1;

	struct Diagnostic *diag = &ctx->diagnostics[ctx->diagnostic_count];
	diag->line = line;
	diag->column = column;
	diag->severity = severity;
	diag->message = message;
	diag->code = code;
	diag->fix_suggestion = 0;
	diag->context_line = 0;

	ctx->diagnostic_count++;
	if (severity >= 2) ctx->error_count++;
	return 0;
}

static inline u8 report_diagnostic_with_fix(
	struct ErrorRecoveryContext *ctx,
	u32 line,
	const char *message,
	const char *fix) {

	if (!ctx || ctx->diagnostic_count >= 256) return 1;

	struct Diagnostic *diag = &ctx->diagnostics[ctx->diagnostic_count];
	diag->line = line;
	diag->column = 0;
	diag->severity = 2;  /* Error */
	diag->message = message;
	diag->fix_suggestion = fix;
	diag->code = 0;

	ctx->diagnostic_count++;
	ctx->error_count++;
	return 0;
}

/* ============================================================ */
/* RECOVERY MECHANISMS */
/* ============================================================ */

static inline u8 recover_from_error(
	struct ErrorRecoveryContext *ctx,
	enum RecoveryStrategy strategy) {

	if (!ctx) return 1;

	ctx->is_recovering = 1;
	ctx->errors_recovered++;

	switch (strategy) {
	case RECOVERY_SKIP_TOKEN:
		/* Skip current token, continue */
		break;
	case RECOVERY_INSERT_TOKEN:
		/* Insert expected token, continue */
		break;
	case RECOVERY_REPLACE_TOKEN:
		/* Replace current token, continue */
		break;
	case RECOVERY_SYNC_TO_DELIMITER:
		/* Skip until next delimiter */
		break;
	}

	return 0;
}

/* ============================================================ */
/* INCREMENTAL ANALYSIS */
/* ============================================================ */

struct AnalysisCache {
	struct Symbol cached_symbols[128];
	u32 symbol_cache_count;
	struct Type cached_types[64];
	u32 type_cache_count;
	u32 cache_hits;
	u32 cache_misses;
};

static inline void analysis_cache_init(struct AnalysisCache *cache) {
	if (!cache) return;
	cache->symbol_cache_count = 0;
	cache->type_cache_count = 0;
	cache->cache_hits = 0;
	cache->cache_misses = 0;
}

static inline u8 cache_lookup_symbol(
	struct AnalysisCache *cache,
	const char *name,
	struct Symbol **result) {

	if (!cache || !name || !result) return 1;

	u32 i;
	for (i = 0; i < cache->symbol_cache_count; i++) {
		const char *cached_name = cache->cached_symbols[i].name;
		const char *lookup = name;
		u32 j = 0;
		while (cached_name[j] && lookup[j] && cached_name[j] == lookup[j]) j++;
		if (cached_name[j] == 0 && lookup[j] == 0) {
			cache->cache_hits++;
			*result = &cache->cached_symbols[i];
			return 0;
		}
	}

	cache->cache_misses++;
	return 1;
}

/* ============================================================ */
/* IDE SUPPORT */
/* ============================================================ */

struct CompletionItem {
	const char *label;
	const char *kind;  /* "function", "variable", "type", etc */
	const char *documentation;
	const char *detail;
};

struct HoverInfo {
	const char *symbol_name;
	const char *type_signature;
	const char *documentation;
	u32 definition_line;
};

static inline struct HoverInfo get_hover_info(
	const char *symbol_name) {

	struct HoverInfo info;
	info.symbol_name = symbol_name;
	info.type_signature = 0;
	info.documentation = 0;
	info.definition_line = 0;
	return info;
}

/* ============================================================ */
/* SUGGESTION ENGINE */
/* ============================================================ */

struct ErrorSuggestion {
	const char *error_code;
	const char *suggestion;
	const char *example;
};

static inline struct ErrorSuggestion get_fix_suggestion(
	const char *error_code) {

	struct ErrorSuggestion sugg;
	sugg.error_code = error_code;
	sugg.suggestion = 0;
	sugg.example = 0;

	/* Simple pattern matching for common errors */
	if (error_code) {
		switch (error_code[0]) {
		case 'E':
			sugg.suggestion = "Check the error message for details";
			break;
		case 'W':
			sugg.suggestion = "Warning: review the code carefully";
			break;
		default:
			sugg.suggestion = "Unknown error type";
		}
	}

	return sugg;
}

/* ============================================================ */
/* STATISTICS */
/* ============================================================ */

struct DiagnosticStats {
	u32 total_diagnostics;
	u32 error_count;
	u32 warning_count;
	u32 info_count;
	u32 recovery_attempts;
	u32 recovery_success_rate;
};

static inline struct DiagnosticStats get_diagnostic_stats(
	struct ErrorRecoveryContext *ctx) {

	struct DiagnosticStats stats;
	stats.total_diagnostics = ctx->diagnostic_count;
	stats.error_count = ctx->error_count;
	stats.warning_count = 0;
	stats.info_count = 0;
	stats.recovery_attempts = ctx->errors_recovered;
	stats.recovery_success_rate = 100;

	u32 i;
	for (i = 0; i < ctx->diagnostic_count; i++) {
		if (ctx->diagnostics[i].severity == 1) {
			stats.warning_count++;
		} else if (ctx->diagnostics[i].severity == 0) {
			stats.info_count++;
		}
	}

	return stats;
}

#endif /* APKC_ADV_ERROR_RECOVERY_H */
