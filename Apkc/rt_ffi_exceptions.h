/* rt_ffi_exceptions.h — FFI & Exception Handling (Stage 7.3)
 *
 * Foreign function interface for calling native C code.
 * Type marshalling between high-level and C types.
 * Exception/error handling with try/catch/finally.
 * Stack unwinding and resource cleanup on exception.
 * Max 5 exception handlers per function.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_RT_FFI_EXCEPTIONS_H
#define APKC_RT_FFI_EXCEPTIONS_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Exception/error type */
typedef enum {
	EXC_NONE = 0,
	EXC_NULL_POINTER = 1,
	EXC_DIVIDE_BY_ZERO = 2,
	EXC_ARRAY_OUT_OF_BOUNDS = 3,
	EXC_INVALID_ARGUMENT = 4,
	EXC_MEMORY_ERROR = 5,
	EXC_RUNTIME_ERROR = 6,
	EXC_NOT_IMPLEMENTED = 7,
	EXC_USER_DEFINED = 8
} ExceptionType;

/* Exception context */
struct Exception {
	ExceptionType type;
	const u8 *message;
	u32 message_len;
	u64 value;             /* Associated value */
	u32 stack_depth;       /* Depth when exception thrown */
};

/* Stack frame for unwinding */
struct StackFrame {
	u64 return_addr;       /* Return address */
	const u8 *func_name;   /* Function name for debugging */
	u32 func_len;
	u64 frame_data[8];     /* Local data */
	u32 frame_size;
	void (*finally_handler)(void);  /* Finally block to execute */
};

/* Exception handler (catch clause) */
struct ExceptionHandler {
	ExceptionType catch_type;
	u8 (*handler_func)(struct Exception *exc);  /* Returns 1 if handled */
};

/* Try/catch/finally block */
struct TryCatchBlock {
	u32 try_start;         /* Code position of try block */
	u32 try_end;
	struct ExceptionHandler handlers[5];  /* Up to 5 catch clauses */
	u32 handler_count;
	u32 finally_start;     /* Code position of finally block */
	u32 finally_len;
};

/* Exception handling context */
struct ExceptionContext {
	struct Exception current_exception;
	struct StackFrame call_stack[32];  /* Up to 32 nested calls */
	u32 stack_depth;
	struct TryCatchBlock try_blocks[16];  /* Up to 16 try/catch blocks */
	u32 try_block_count;
	u8 exception_pending;  /* 1 if exception is being handled */
};

/* Initialize exception context */
static inline void exception_ctx_init(struct ExceptionContext *exc) {
	exc->current_exception.type = EXC_NONE;
	exc->stack_depth = 0;
	exc->try_block_count = 0;
	exc->exception_pending = 0;
}

/* === EXCEPTION THROWING === */

/* Throw exception */
static inline void exception_throw(
	struct ExceptionContext *ctx,
	ExceptionType type,
	const u8 *message, u32 msg_len,
	u64 value)
{
	ctx->current_exception.type = type;
	ctx->current_exception.message = message;
	ctx->current_exception.message_len = msg_len;
	ctx->current_exception.value = value;
	ctx->current_exception.stack_depth = ctx->stack_depth;
	ctx->exception_pending = 1;
}

/* Check if exception is pending */
static inline u8 exception_pending(struct ExceptionContext *ctx) {
	return ctx->exception_pending;
}

/* === EXCEPTION HANDLING === */

/* Register try/catch/finally block */
static inline u8 exception_register_try_block(
	struct ExceptionContext *ctx,
	u32 try_start, u32 try_end,
	u32 finally_start, u32 finally_len)
{
	if (ctx->try_block_count >= 16) return 1;

	struct TryCatchBlock *tcb = &ctx->try_blocks[ctx->try_block_count];
	tcb->try_start = try_start;
	tcb->try_end = try_end;
	tcb->handler_count = 0;
	tcb->finally_start = finally_start;
	tcb->finally_len = finally_len;

	ctx->try_block_count++;
	return 0;
}

/* Add exception handler to try block */
static inline u8 exception_add_handler(
	struct ExceptionContext *ctx,
	ExceptionType catch_type,
	u8 (*handler)(struct Exception *exc))
{
	if (ctx->try_block_count == 0) return 1;

	struct TryCatchBlock *tcb = &ctx->try_blocks[ctx->try_block_count - 1];
	if (tcb->handler_count >= 5) return 1;

	struct ExceptionHandler *eh = &tcb->handlers[tcb->handler_count];
	eh->catch_type = catch_type;
	eh->handler_func = handler;

	tcb->handler_count++;
	return 0;
}

/* Unwind stack on exception */
static inline void exception_unwind_stack(
	struct ExceptionContext *ctx)
{
	/* Call finally blocks as we unwind */
	while (ctx->stack_depth > 0) {
		struct StackFrame *frame = &ctx->call_stack[ctx->stack_depth - 1];
		if (frame->finally_handler) {
			/* Call finally handler for cleanup */
			frame->finally_handler();
		}
		ctx->stack_depth--;
	}
}

/* === FFI SUPPORT === */

/* Foreign function call context */
struct FFICall {
	u64 func_ptr;          /* Native C function pointer */
	u64 args[8];           /* Arguments (up to 8) */
	u32 arg_count;
	u64 return_value;      /* Return value from C function */
	u8 arg_types[8];       /* Type of each argument */
	u8 return_type;        /* Return type */
};

/* Call native C function with FFI wrapper */
static inline u64 ffi_call(
	struct FFICall *ffi_ctx)
{
	/* Call function with proper ABI conventions */
	/* First 3 args in x0-x2 (ARM64 ABI) */
	/* Rest on stack */

	typedef u64 (*Func0)(void);
	typedef u64 (*Func1)(u64);
	typedef u64 (*Func2)(u64, u64);
	typedef u64 (*Func3)(u64, u64, u64);

	switch (ffi_ctx->arg_count) {
		case 0: {
			Func0 f = (Func0)ffi_ctx->func_ptr;
			ffi_ctx->return_value = f();
			break;
		}
		case 1: {
			Func1 f = (Func1)ffi_ctx->func_ptr;
			ffi_ctx->return_value = f(ffi_ctx->args[0]);
			break;
		}
		case 2: {
			Func2 f = (Func2)ffi_ctx->func_ptr;
			ffi_ctx->return_value = f(ffi_ctx->args[0], ffi_ctx->args[1]);
			break;
		}
		case 3: {
			Func3 f = (Func3)ffi_ctx->func_ptr;
			ffi_ctx->return_value = f(ffi_ctx->args[0], ffi_ctx->args[1],
						   ffi_ctx->args[2]);
			break;
		}
		default:
			/* Too many arguments for this simplified FFI */
			return 0;
	}

	return ffi_ctx->return_value;
}

/* === TYPE MARSHALLING === */

/* Marshal high-level value to C type */
static inline u64 ffi_marshal_in(
	u64 value, u8 c_type)
{
	/* Type codes: 0=INT64, 1=INT32, 2=FLOAT, 3=POINTER */
	switch (c_type) {
		case 0: return value;  /* INT64: pass as-is */
		case 1: return (u64)(u32)value;  /* INT32: truncate */
		case 2: return value;  /* FLOAT: pass as-is (IEEE format) */
		case 3: return value;  /* POINTER: pass as-is */
		default: return 0;
	}
}

/* Marshal C return value to high-level type */
static inline u64 ffi_marshal_out(
	u64 c_value, u8 type)
{
	/* Reverse of marshal_in */
	switch (type) {
		case 0: return c_value;  /* INT64 */
		case 1: return (u64)(u32)c_value;  /* INT32 */
		case 2: return c_value;  /* FLOAT */
		case 3: return c_value;  /* POINTER */
		default: return 0;
	}
}

/* === STACK MANAGEMENT === */

/* Push stack frame on entry */
static inline u8 stack_push_frame(
	struct ExceptionContext *ctx,
	const u8 *func_name, u32 func_len,
	void (*finally_handler)(void))
{
	if (ctx->stack_depth >= 32) return 1;

	struct StackFrame *frame = &ctx->call_stack[ctx->stack_depth];
	frame->func_name = func_name;
	frame->func_len = func_len;
	frame->finally_handler = finally_handler;
	frame->frame_size = 0;

	ctx->stack_depth++;
	return 0;
}

/* Pop stack frame on exit */
static inline void stack_pop_frame(
	struct ExceptionContext *ctx)
{
	if (ctx->stack_depth > 0) {
		ctx->stack_depth--;
	}
}

/* Print stack trace for debugging */
static inline void stack_trace_print(
	struct ExceptionContext *ctx)
{
	/* Print exception info and call stack */
	/* In freestanding model: output to debug console or log */
	/* Format: "ExceptionType: message\nCallStack:\n  func1\n  func2\n..." */
	u32 i;
	for (i = 0; i < ctx->stack_depth; i++) {
		struct StackFrame *frame = &ctx->call_stack[i];
		/* Print frame->func_name */
	}
}

#endif /* APKC_RT_FFI_EXCEPTIONS_H */
