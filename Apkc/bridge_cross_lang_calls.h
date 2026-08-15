/* bridge_cross_lang_calls.h — Cross-Language Function Calls (Stage 8.2)
 *
 * Calling convention bridging: language ABI → branchless ABI → language ABI.
 * Argument marshalling and routing (up to 8 arguments).
 * Return value collection and translation.
 * Error propagation and exception mapping across language boundaries.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_BRIDGE_CROSS_LANG_CALLS_H
#define APKC_BRIDGE_CROSS_LANG_CALLS_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Forward declarations (from bridge_language_bindings.h) */
struct MarshalledValue;
struct TypeDescriptor;
struct LanguageBinding;

/* Error codes for cross-language calls */
enum CrossLangError {
	XLERR_OK = 0,
	XLERR_ARG_COUNT = 1,      /* Argument count mismatch */
	XLERR_ARG_TYPE = 2,        /* Argument type incompatibility */
	XLERR_NO_FUNCTION = 3,     /* Function not found */
	XLERR_MARSHALLING = 4,     /* Marshalling failed */
	XLERR_EXCEPTION = 5,       /* Exception in callee */
	XLERR_TIMEOUT = 6,         /* Call timeout */
	XLERR_STACK_OVERFLOW = 7   /* Stack overflow in call */
};

/* Function signature for cross-language resolution */
struct CrossLangFuncSig {
	const u8 *name;                    /* Function name */
	u32 name_len;
	u32 arg_count;                     /* Number of arguments */
	struct TypeDescriptor args[8];     /* Argument types (max 8) */
	struct TypeDescriptor return_type; /* Return type */
	u8 caller_lang;                    /* Caller language */
	u8 callee_lang;                    /* Callee language */
};

/* Call setup state for cross-language invocation */
struct CrossLangCallSetup {
	struct CrossLangFuncSig sig;
	u64 func_ptr;                      /* Address of function to call */
	struct MarshalledValue args[8];    /* Marshalled arguments */
	u32 arg_count;
	u8 args_ready;                     /* 1 if args prepared */
	u8 r0, r1, r2;                     /* Registers for first 3 args */
	u64 stack_args[5];                 /* Stack args for 4-8th params */
	u32 stack_arg_count;
};

/* Call result (return value + error status) */
struct CrossLangResult {
	struct MarshalledValue value;      /* Return value */
	enum CrossLangError error;         /* Error code */
	const u8 *error_msg;               /* Error message (if error) */
	u32 error_msg_len;
	u64 exception_code;                /* Exception code from callee */
};

/* Exception context from callee */
struct CrossLangException {
	enum CrossLangError type;
	const u8 *message;
	u32 message_len;
	u64 code;
	u8 lang_origin;                    /* Language where exception occurred */
};

/* ============================================================ */
/* CALL SETUP */
/* ============================================================ */

/* Initialize call setup for cross-language invocation */
static inline void cross_lang_call_setup_init(
	struct CrossLangCallSetup *setup) {

	if (!setup) return;

	setup->sig.arg_count = 0;
	setup->sig.caller_lang = 0;
	setup->sig.callee_lang = 0;
	setup->arg_count = 0;
	setup->args_ready = 0;
	setup->r0 = 0;
	setup->r1 = 0;
	setup->r2 = 0;
	setup->stack_arg_count = 0;
}

/* Add argument to call setup */
static inline u8 cross_lang_call_add_arg(
	struct CrossLangCallSetup *setup,
	struct MarshalledValue arg,
	struct TypeDescriptor type) {

	if (!setup) return 1;
	if (setup->arg_count >= 8) return 1;  /* Max 8 arguments */

	setup->args[setup->arg_count] = arg;
	setup->sig.args[setup->arg_count] = type;
	setup->arg_count++;
	return 0;
}

/* Prepare arguments for ARM64 ABI (x0-x7, then stack) */
static inline u8 cross_lang_call_prepare_args(
	struct CrossLangCallSetup *setup) {

	if (!setup || setup->arg_count > 8) return 1;

	/* First 3 args go in x0-x2 (simplified: always use these) */
	if (setup->arg_count >= 1) setup->r0 = (u8)setup->args[0].value;
	if (setup->arg_count >= 2) setup->r1 = (u8)setup->args[1].value;
	if (setup->arg_count >= 3) setup->r2 = (u8)setup->args[2].value;

	/* Args 4-8 go on stack */
	u32 i;
	for (i = 3; i < setup->arg_count; i++) {
		setup->stack_args[i - 3] = setup->args[i].value;
		setup->stack_arg_count++;
	}

	setup->args_ready = 1;
	return 0;
}

/* Validate arguments against expected signature */
static inline u8 cross_lang_call_validate_args(
	struct LanguageBinding *binding,
	struct CrossLangCallSetup *setup) {

	if (!setup || !binding) return 1;

	u32 i;
	for (i = 0; i < setup->arg_count; i++) {
		/* Check type compatibility (simplified) */
		if (binding->type_compat) {
			if (!binding->type_compat(setup->args[i], setup->sig.args[i])) {
				return 1;  /* Type mismatch */
			}
		}
	}

	return 0;
}

/* ============================================================ */
/* CALL INVOCATION */
/* ============================================================ */

/* Type for bare function call (up to 8 u64 arguments) */
typedef u64 (*CrossLangFunc0)(void);
typedef u64 (*CrossLangFunc1)(u64);
typedef u64 (*CrossLangFunc2)(u64, u64);
typedef u64 (*CrossLangFunc3)(u64, u64, u64);
typedef u64 (*CrossLangFunc4)(u64, u64, u64, u64);
typedef u64 (*CrossLangFunc5)(u64, u64, u64, u64, u64);
typedef u64 (*CrossLangFunc6)(u64, u64, u64, u64, u64, u64);
typedef u64 (*CrossLangFunc7)(u64, u64, u64, u64, u64, u64, u64);
typedef u64 (*CrossLangFunc8)(u64, u64, u64, u64, u64, u64, u64, u64);

/* Invoke function with prepared arguments */
static inline u64 cross_lang_call_invoke(
	struct CrossLangCallSetup *setup) {

	if (!setup || !setup->func_ptr || !setup->args_ready) return 0;

	/* Dispatch by argument count */
	switch (setup->arg_count) {
		case 0: {
			CrossLangFunc0 f = (CrossLangFunc0)setup->func_ptr;
			return f();
		}
		case 1: {
			CrossLangFunc1 f = (CrossLangFunc1)setup->func_ptr;
			return f(setup->args[0].value);
		}
		case 2: {
			CrossLangFunc2 f = (CrossLangFunc2)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value);
		}
		case 3: {
			CrossLangFunc3 f = (CrossLangFunc3)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value);
		}
		case 4: {
			CrossLangFunc4 f = (CrossLangFunc4)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value, setup->args[3].value);
		}
		case 5: {
			CrossLangFunc5 f = (CrossLangFunc5)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value, setup->args[3].value,
					 setup->args[4].value);
		}
		case 6: {
			CrossLangFunc6 f = (CrossLangFunc6)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value, setup->args[3].value,
					 setup->args[4].value, setup->args[5].value);
		}
		case 7: {
			CrossLangFunc7 f = (CrossLangFunc7)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value, setup->args[3].value,
					 setup->args[4].value, setup->args[5].value,
					 setup->args[6].value);
		}
		case 8: {
			CrossLangFunc8 f = (CrossLangFunc8)setup->func_ptr;
			return f(setup->args[0].value, setup->args[1].value,
					 setup->args[2].value, setup->args[3].value,
					 setup->args[4].value, setup->args[5].value,
					 setup->args[6].value, setup->args[7].value);
		}
		default:
			return 0;
	}
}

/* ============================================================ */
/* RETURN VALUE HANDLING */
/* ============================================================ */

/* Collect return value from machine register */
static inline struct MarshalledValue cross_lang_call_return_value(
	u64 raw_return,
	struct TypeDescriptor return_type) {

	struct MarshalledValue result;
	result.value = raw_return;
	result.aux = 0;
	result.type = return_type;
	return result;
}

/* Unmarshal return value for caller's language */
static inline u8 cross_lang_call_unmarshal_return(
	struct LanguageBinding *binding,
	struct MarshalledValue m,
	u8 *lang_out, u32 *len_out) {

	if (!binding || !binding->from_machine) return 1;
	return binding->from_machine(m, lang_out, len_out);
}

/* ============================================================ */
/* ERROR MAPPING */
/* ============================================================ */

/* Map language-specific exception to cross-language error code */
static inline enum CrossLangError cross_lang_error_map_python(
	u64 py_exc_code) {

	/* Python exception codes → CrossLangError */
	switch (py_exc_code) {
		case 1: return XLERR_ARG_TYPE;      /* TypeError */
		case 2: return XLERR_ARG_COUNT;     /* ArgumentError */
		case 3: return XLERR_EXCEPTION;     /* RuntimeError */
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_go(
	u64 go_exc_code) {

	/* Go error codes → CrossLangError */
	switch (go_exc_code) {
		case 1: return XLERR_ARG_TYPE;
		case 2: return XLERR_ARG_COUNT;
		case 3: return XLERR_NO_FUNCTION;
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_rust(
	u64 rust_exc_code) {

	/* Rust error codes → CrossLangError */
	switch (rust_exc_code) {
		case 1: return XLERR_ARG_TYPE;
		case 2: return XLERR_EXCEPTION;
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_c(
	u64 c_exc_code) {

	/* C error codes (errno-like) → CrossLangError */
	switch (c_exc_code) {
		case 22: return XLERR_ARG_TYPE;     /* EINVAL */
		case 12: return XLERR_STACK_OVERFLOW;  /* ENOMEM */
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_js(
	u64 js_exc_code) {

	/* JavaScript exception codes → CrossLangError */
	switch (js_exc_code) {
		case 1: return XLERR_ARG_TYPE;      /* TypeError */
		case 2: return XLERR_EXCEPTION;     /* ReferenceError */
		case 3: return XLERR_EXCEPTION;     /* RangeError */
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_java(
	u64 java_exc_code) {

	/* Java exception codes → CrossLangError */
	switch (java_exc_code) {
		case 1: return XLERR_ARG_TYPE;      /* IllegalArgumentException */
		case 2: return XLERR_ARG_COUNT;     /* MethodNotFoundException */
		default: return XLERR_EXCEPTION;
	}
}

static inline enum CrossLangError cross_lang_error_map_swift(
	u64 swift_exc_code) {

	/* Swift error codes → CrossLangError */
	switch (swift_exc_code) {
		case 1: return XLERR_ARG_TYPE;
		case 2: return XLERR_EXCEPTION;
		default: return XLERR_EXCEPTION;
	}
}

/* Map error code by language origin */
static inline enum CrossLangError cross_lang_error_map(
	u64 exc_code, u8 lang_origin) {

	switch (lang_origin) {
		case 0: return cross_lang_error_map_python(exc_code);
		case 1: return cross_lang_error_map_go(exc_code);
		case 2: return cross_lang_error_map_rust(exc_code);
		case 3: return cross_lang_error_map_c(exc_code);
		case 4: return cross_lang_error_map_js(exc_code);
		case 5: return cross_lang_error_map_java(exc_code);
		case 6: return cross_lang_error_map_swift(exc_code);
		default: return XLERR_EXCEPTION;
	}
}

/* Create exception from error code */
static inline struct CrossLangException cross_lang_exception_from_error(
	enum CrossLangError err, u8 lang_origin) {

	struct CrossLangException exc = {0};
	exc.type = err;
	exc.lang_origin = lang_origin;
	exc.code = (u64)err;

	/* Assign error message by type */
	switch (err) {
		case XLERR_ARG_COUNT:
			exc.message = (const u8*)"argument count mismatch";
			exc.message_len = 23;
			break;
		case XLERR_ARG_TYPE:
			exc.message = (const u8*)"argument type incompatible";
			exc.message_len = 26;
			break;
		case XLERR_NO_FUNCTION:
			exc.message = (const u8*)"function not found";
			exc.message_len = 18;
			break;
		case XLERR_MARSHALLING:
			exc.message = (const u8*)"marshalling failed";
			exc.message_len = 18;
			break;
		case XLERR_EXCEPTION:
			exc.message = (const u8*)"exception in callee";
			exc.message_len = 19;
			break;
		case XLERR_TIMEOUT:
			exc.message = (const u8*)"call timeout";
			exc.message_len = 12;
			break;
		case XLERR_STACK_OVERFLOW:
			exc.message = (const u8*)"stack overflow";
			exc.message_len = 14;
			break;
		default:
			exc.message = (const u8*)"unknown error";
			exc.message_len = 13;
			break;
	}

	return exc;
}

/* ============================================================ */
/* COMPLETE CALL FLOW */
/* ============================================================ */

/* Execute cross-language call: marshal → invoke → unmarshal */
static inline struct CrossLangResult cross_lang_call_execute(
	struct CrossLangCallSetup *setup,
	struct LanguageBinding *caller_binding,
	struct LanguageBinding *callee_binding) {

	struct CrossLangResult result = {0};

	if (!setup || !setup->func_ptr) {
		result.error = XLERR_NO_FUNCTION;
		return result;
	}

	/* Validate arguments */
	if (callee_binding && callee_binding->type_compat) {
		u32 i;
		for (i = 0; i < setup->arg_count; i++) {
			if (!callee_binding->type_compat(setup->args[i], setup->sig.args[i])) {
				result.error = XLERR_ARG_TYPE;
				return result;
			}
		}
	}

	/* Prepare arguments for ARM64 ABI */
	if (cross_lang_call_prepare_args(setup)) {
		result.error = XLERR_MARSHALLING;
		return result;
	}

	/* Invoke function */
	u64 raw_return = cross_lang_call_invoke(setup);

	/* Collect return value */
	result.value = cross_lang_call_return_value(raw_return, setup->sig.return_type);
	result.error = XLERR_OK;

	return result;
}

/* ============================================================ */
/* CALL DESCRIPTION & LOGGING */
/* ============================================================ */

/* Format call description for debugging */
static inline u32 cross_lang_call_format_desc(
	struct CrossLangCallSetup *setup,
	u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 50) return 0;

	const u8 *fmt = (const u8*)"call[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	/* Append function name (first 8 bytes) */
	u32 j;
	if (setup->sig.name) {
		for (j = 0; j < setup->sig.name_len && j < 8 && i < buf_size - 1; j++) {
			buf[i++] = setup->sig.name[j];
		}
	}

	if (i < buf_size - 4) {
		buf[i++] = ']';
		buf[i++] = '(';
		buf[i] = ')';
		buf[i + 1] = '\0';
	}

	return i + 2;
}

#endif /* APKC_BRIDGE_CROSS_LANG_CALLS_H */
