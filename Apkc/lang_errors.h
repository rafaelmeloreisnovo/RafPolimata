/* lang_errors.h — Error handling and validation (Stage 4.8)
 *
 * Comprehensive error reporting, validation, and recovery
 * Tracks line numbers, error types, and graceful recovery
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_ERRORS_H
#define APKC_LANG_ERRORS_H 1

typedef unsigned char u8;
typedef unsigned int u32;

/* Error types */
enum ErrorType {
	ERR_NONE = 0,
	ERR_SYNTAX,           /* Unexpected token or invalid syntax */
	ERR_UNDEFINED_VAR,    /* Variable not declared */
	ERR_REDEFINED_VAR,    /* Variable declared twice in same scope */
	ERR_UNEXPECTED_TOKEN, /* Token not expected here */
	ERR_UNCLOSED_PAREN,   /* Missing closing parenthesis */
	ERR_UNCLOSED_BRACE,   /* Missing closing brace */
	ERR_TYPE_MISMATCH,    /* Type incompatibility */
	ERR_INVALID_RETURN,   /* Return outside function */
	ERR_INVALID_BREAK,    /* Break/continue outside loop */
	ERR_STACK_OVERFLOW,   /* Too many nested scopes/expressions */
	ERR_BUFFER_OVERFLOW,  /* Code buffer full */
};

/* Error record */
struct Error {
	enum ErrorType type;
	u32 line;
	u32 column;
	const u8 *msg;
	u32 msg_len;
};

/* Error context: tracks multiple errors during parsing */
struct ErrorCtx {
	struct Error errors[16];  /* Up to 16 errors per compilation */
	u32 error_count;
	u32 current_line;
	u32 current_column;
	u8 has_fatal;           /* 1 if any fatal error encountered */
};

/* Initialize error context */
static inline void error_init(struct ErrorCtx *ec) {
	ec->error_count = 0;
	ec->current_line = 1;
	ec->current_column = 0;
	ec->has_fatal = 0;
}

/* Record an error */
static inline void error_record(
	struct ErrorCtx *ec,
	enum ErrorType type,
	const u8 *msg, u32 msg_len)
{
	if (ec->error_count >= 16) return;  /* Ignore errors after limit */

	ec->errors[ec->error_count].type = type;
	ec->errors[ec->error_count].line = ec->current_line;
	ec->errors[ec->error_count].column = ec->current_column;
	ec->errors[ec->error_count].msg = msg;
	ec->errors[ec->error_count].msg_len = msg_len;
	ec->error_count++;

	/* Mark as fatal if critical error */
	if (type == ERR_BUFFER_OVERFLOW || type == ERR_STACK_OVERFLOW) {
		ec->has_fatal = 1;
	}
}

/* Check if any errors recorded */
static inline u8 error_has_errors(struct ErrorCtx *ec) {
	return ec->error_count > 0;
}

/* Check if any fatal errors */
static inline u8 error_has_fatal(struct ErrorCtx *ec) {
	return ec->has_fatal;
}

/* Advance line counter (called on newline) */
static inline void error_advance_line(struct ErrorCtx *ec) {
	ec->current_line++;
	ec->current_column = 0;
}

/* Advance column counter */
static inline void error_advance_column(struct ErrorCtx *ec) {
	ec->current_column++;
}

/* Get error count */
static inline u32 error_count(struct ErrorCtx *ec) {
	return ec->error_count;
}

/* Get error at index */
static inline struct Error error_get(struct ErrorCtx *ec, u32 idx) {
	if (idx < ec->error_count) return ec->errors[idx];
	struct Error e = {0};
	return e;
}

/* === VALIDATION FUNCTIONS === */

/* Validate parenthesis balance in token stream */
static inline u8 error_validate_parens(const u8 *src, u32 len) {
	u32 paren_depth = 0;
	u32 brace_depth = 0;
	u32 i;

	for (i = 0; i < len; i++) {
		if (src[i] == '(') paren_depth++;
		if (src[i] == ')') {
			if (paren_depth == 0) return 0;  /* Unmatched ) */
			paren_depth--;
		}
		if (src[i] == '{') brace_depth++;
		if (src[i] == '}') {
			if (brace_depth == 0) return 0;  /* Unmatched } */
			brace_depth--;
		}
	}

	return paren_depth == 0 && brace_depth == 0;
}

/* Validate maximum nesting depth */
static inline u8 error_validate_depth(const u8 *src, u32 len, u32 max_depth) {
	u32 depth = 0;
	u32 max_seen = 0;
	u32 i;

	for (i = 0; i < len; i++) {
		if (src[i] == '{' || src[i] == '[' || src[i] == '(') {
			depth++;
			if (depth > max_seen) max_seen = depth;
		}
		if (src[i] == '}' || src[i] == ']' || src[i] == ')') {
			if (depth > 0) depth--;
		}
	}

	return max_seen <= max_depth;
}

/* Validate buffer capacity */
static inline u8 error_validate_buffer(u32 available, u32 needed) {
	return available >= needed;
}

#endif /* APKC_LANG_ERRORS_H */
