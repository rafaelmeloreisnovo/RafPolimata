/* deploy_debugger_integration.h — Debugger Integration & Stack Traces (Stage 9.3)
 *
 * Debug symbol embedding (source location mappings).
 * Stack trace generation with source file/line resolution.
 * Breakpoint management (set/clear/query).
 * Variable inspection at breakpoint.
 * Stepping control (step-in, step-over, step-out).
 * Max 1024 debug symbols, max 32 breakpoints per module.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DEPLOY_DEBUGGER_INTEGRATION_H
#define APKC_DEPLOY_DEBUGGER_INTEGRATION_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Debug symbol: maps instruction address to source location */
struct DebugSymbol {
	u64 insn_addr;             /* Instruction address */
	u32 source_file;           /* Source file index (into symbol table's file array) */
	u32 source_line;           /* Source line number */
	u32 source_column;         /* Source column number */
	const u8 *func_name;       /* Function name */
	u32 func_name_len;
};

/* Stack frame at specific point in execution */
struct StackFrame {
	u64 return_addr;           /* Return address */
	u64 frame_ptr;             /* Frame pointer / stack base */
	const u8 *func_name;       /* Function name */
	u32 func_name_len;
	const u8 *source_file;     /* Source file name */
	u32 source_file_len;
	u32 source_line;           /* Source line number */
	u64 *local_vars;           /* Pointer to local variables (on stack) */
	u32 local_var_count;
};

/* Breakpoint */
struct Breakpoint {
	u64 address;               /* Breakpoint address (code location) */
	u32 hit_count;             /* Number of times hit */
	u8 enabled;                /* 1 if breakpoint active */
	u8 temporary;              /* 1 if one-shot breakpoint */
	const u8 *condition;       /* Condition string (simplified) */
	u32 condition_len;
};

/* Debug context for module */
struct DebugContext {
	struct DebugSymbol symbols[1024];  /* Up to 1024 debug symbols */
	u32 symbol_count;
	struct Breakpoint breakpoints[32];  /* Up to 32 breakpoints */
	u32 breakpoint_count;
	struct StackFrame call_stack[32];  /* Up to 32 nested calls */
	u32 stack_depth;
	u64 current_pc;            /* Current program counter */
	u8 stepping;               /* 1 if single-stepping */
	u8 step_type;              /* 0=step-in, 1=step-over, 2=step-out */
	const u8 **source_files;   /* Array of source file names */
	u32 file_count;
};

/* ============================================================ */
/* DEBUG SYMBOL MANAGEMENT */
/* ============================================================ */

/* Initialize debug context */
static inline void debug_context_init(struct DebugContext *dbg) {
	if (!dbg) return;

	dbg->symbol_count = 0;
	dbg->breakpoint_count = 0;
	dbg->stack_depth = 0;
	dbg->current_pc = 0;
	dbg->stepping = 0;
	dbg->step_type = 0;
	dbg->file_count = 0;
}

/* Add debug symbol (instruction → source mapping) */
static inline u8 debug_add_symbol(
	struct DebugContext *dbg,
	u64 insn_addr, u32 file_idx, u32 line, u32 column,
	const u8 *func_name, u32 func_len) {

	if (!dbg || dbg->symbol_count >= 1024) return 1;

	struct DebugSymbol *sym = &dbg->symbols[dbg->symbol_count];
	sym->insn_addr = insn_addr;
	sym->source_file = file_idx;
	sym->source_line = line;
	sym->source_column = column;
	sym->func_name = func_name;
	sym->func_name_len = func_len;

	dbg->symbol_count++;
	return 0;
}

/* Register source file */
static inline u32 debug_register_source_file(
	struct DebugContext *dbg,
	const u8 *filename, u32 filename_len) {

	if (!dbg) return 0xFFFFFFFF;

	/* Simplified: just store first occurrence */
	if (dbg->file_count < 256) {
		dbg->source_files[dbg->file_count] = filename;
		return dbg->file_count++;
	}

	return 0xFFFFFFFF;
}

/* Lookup debug symbol by address */
static inline struct DebugSymbol* debug_symbol_at_address(
	struct DebugContext *dbg,
	u64 addr) {

	if (!dbg) return NULL;

	/* Find closest symbol not exceeding addr */
	struct DebugSymbol *best = NULL;
	u32 i;

	for (i = 0; i < dbg->symbol_count; i++) {
		struct DebugSymbol *sym = &dbg->symbols[i];
		if (sym->insn_addr <= addr) {
			if (!best || sym->insn_addr > best->insn_addr) {
				best = sym;
			}
		}
	}

	return best;
}

/* ============================================================ */
/* BREAKPOINT MANAGEMENT */
/* ============================================================ */

/* Set breakpoint at address */
static inline u8 debug_set_breakpoint(
	struct DebugContext *dbg,
	u64 address) {

	if (!dbg || dbg->breakpoint_count >= 32) return 1;

	struct Breakpoint *bp = &dbg->breakpoints[dbg->breakpoint_count];
	bp->address = address;
	bp->hit_count = 0;
	bp->enabled = 1;
	bp->temporary = 0;
	bp->condition = NULL;
	bp->condition_len = 0;

	dbg->breakpoint_count++;
	return 0;
}

/* Clear breakpoint at address */
static inline u8 debug_clear_breakpoint(
	struct DebugContext *dbg,
	u64 address) {

	if (!dbg) return 1;

	u32 i;
	for (i = 0; i < dbg->breakpoint_count; i++) {
		if (dbg->breakpoints[i].address == address) {
			/* Swap with last and decrement count */
			if (i < dbg->breakpoint_count - 1) {
				dbg->breakpoints[i] = dbg->breakpoints[dbg->breakpoint_count - 1];
			}
			dbg->breakpoint_count--;
			return 0;
		}
	}

	return 1;  /* Breakpoint not found */
}

/* Check if breakpoint hit at current address */
static inline struct Breakpoint* debug_check_breakpoint(
	struct DebugContext *dbg,
	u64 current_pc) {

	if (!dbg) return NULL;

	u32 i;
	for (i = 0; i < dbg->breakpoint_count; i++) {
		struct Breakpoint *bp = &dbg->breakpoints[i];
		if (bp->enabled && bp->address == current_pc) {
			bp->hit_count++;
			return bp;
		}
	}

	return NULL;  /* No breakpoint at this address */
}

/* ============================================================ */
/* STACK UNWINDING */
/* ============================================================ */

/* Push stack frame on function entry */
static inline u8 debug_push_frame(
	struct DebugContext *dbg,
	u64 return_addr, u64 frame_ptr,
	const u8 *func_name, u32 func_len,
	const u8 *source_file, u32 file_len,
	u32 source_line) {

	if (!dbg || dbg->stack_depth >= 32) return 1;

	struct StackFrame *frame = &dbg->call_stack[dbg->stack_depth];
	frame->return_addr = return_addr;
	frame->frame_ptr = frame_ptr;
	frame->func_name = func_name;
	frame->func_name_len = func_len;
	frame->source_file = source_file;
	frame->source_file_len = file_len;
	frame->source_line = source_line;
	frame->local_vars = (u64 *)frame_ptr;
	frame->local_var_count = 0;

	dbg->stack_depth++;
	return 0;
}

/* Pop stack frame on function exit */
static inline u8 debug_pop_frame(struct DebugContext *dbg) {
	if (!dbg || dbg->stack_depth == 0) return 1;

	dbg->stack_depth--;
	return 0;
}

/* Get current stack frame */
static inline struct StackFrame* debug_current_frame(
	struct DebugContext *dbg) {

	if (!dbg || dbg->stack_depth == 0) return NULL;
	return &dbg->call_stack[dbg->stack_depth - 1];
}

/* ============================================================ */
/* STACK TRACE GENERATION */
/* ============================================================ */

/* Generate stack trace from current state */
static inline u32 debug_generate_stacktrace(
	struct DebugContext *dbg,
	u8 *buf, u32 buf_size) {

	if (!dbg || !buf || buf_size < 100) return 0;

	const u8 *hdr = (const u8*)"Stacktrace:\n";
	u32 i = 0;
	while (hdr[i] && i < buf_size - 1) {
		buf[i] = hdr[i];
		i++;
	}

	/* Print each frame in call stack (most recent first) */
	u32 frame_idx;
	for (frame_idx = dbg->stack_depth; frame_idx > 0 && i < buf_size - 100; frame_idx--) {
		struct StackFrame *frame = &dbg->call_stack[frame_idx - 1];

		/* Frame number */
		u32 idx = dbg->stack_depth - frame_idx;
		if (i < buf_size - 1) buf[i++] = '#';
		if (i < buf_size - 1) buf[i++] = '0' + (idx % 10);
		if (i < buf_size - 1) buf[i++] = ' ';

		/* Function name */
		if (frame->func_name) {
			u32 j;
			for (j = 0; j < frame->func_name_len && j < 32 && i < buf_size - 1; j++) {
				buf[i++] = frame->func_name[j];
			}
		}

		/* Source location */
		if (i < buf_size - 3) {
			buf[i++] = ' ';
			buf[i++] = 'a';
			buf[i++] = 't';
			buf[i++] = ' ';
		}

		if (frame->source_file) {
			u32 j;
			for (j = 0; j < frame->source_file_len && j < 32 && i < buf_size - 1; j++) {
				buf[i++] = frame->source_file[j];
			}
		}

		if (i < buf_size - 1) buf[i++] = ':';

		/* Line number */
		u32 line = frame->source_line;
		u32 div = 1000;
		while (div >= 1 && i < buf_size - 1) {
			u8 digit = (line / div) % 10;
			if (digit || div <= line) {
				buf[i++] = '0' + digit;
			}
			div /= 10;
		}

		if (i < buf_size - 1) buf[i++] = '\n';
	}

	buf[i] = '\0';
	return i;
}

/* Format single stack frame for output */
static inline u32 debug_format_frame(
	struct StackFrame *frame,
	u8 *buf, u32 buf_size) {

	if (!frame || !buf || buf_size < 50) return 0;

	u32 i = 0;

	/* Function name */
	if (frame->func_name) {
		u32 j;
		for (j = 0; j < frame->func_name_len && j < 32 && i < buf_size - 1; j++) {
			buf[i++] = frame->func_name[j];
		}
	}

	if (i < buf_size - 3) {
		buf[i++] = ' ';
		buf[i++] = '(';
	}

	/* Source file and line */
	if (frame->source_file && i < buf_size - 1) {
		u32 j;
		for (j = 0; j < frame->source_file_len && j < 16 && i < buf_size - 1; j++) {
			buf[i++] = frame->source_file[j];
		}
	}

	if (i < buf_size - 1) buf[i++] = ':';

	u32 line = frame->source_line;
	u32 div = 100;
	while (div >= 1 && i < buf_size - 1) {
		u8 digit = (line / div) % 10;
		if (digit || div <= line) {
			buf[i++] = '0' + digit;
		}
		div /= 10;
	}

	if (i < buf_size - 1) buf[i++] = ')';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* STEPPING CONTROL */
/* ============================================================ */

/* Start step-in (step into function calls) */
static inline void debug_step_in(struct DebugContext *dbg) {
	if (!dbg) return;

	dbg->stepping = 1;
	dbg->step_type = 0;
}

/* Start step-over (skip function calls) */
static inline void debug_step_over(struct DebugContext *dbg) {
	if (!dbg) return;

	dbg->stepping = 1;
	dbg->step_type = 1;
}

/* Start step-out (execute until function return) */
static inline void debug_step_out(struct DebugContext *dbg) {
	if (!dbg) return;

	dbg->stepping = 1;
	dbg->step_type = 2;
}

/* Stop stepping */
static inline void debug_step_stop(struct DebugContext *dbg) {
	if (!dbg) return;

	dbg->stepping = 0;
}

/* ============================================================ */
/* VARIABLE INSPECTION */
/* ============================================================ */

/* Read variable value from current frame */
static inline u64 debug_read_variable(
	struct DebugContext *dbg,
	u32 var_index) {

	if (!dbg || dbg->stack_depth == 0) return 0;

	struct StackFrame *frame = &dbg->call_stack[dbg->stack_depth - 1];

	if (var_index >= frame->local_var_count) return 0;

	if (frame->local_vars) {
		return frame->local_vars[var_index];
	}

	return 0;
}

/* Set variable value in current frame */
static inline u8 debug_write_variable(
	struct DebugContext *dbg,
	u32 var_index,
	u64 value) {

	if (!dbg || dbg->stack_depth == 0) return 1;

	struct StackFrame *frame = &dbg->call_stack[dbg->stack_depth - 1];

	if (var_index >= frame->local_var_count) return 1;

	if (frame->local_vars) {
		frame->local_vars[var_index] = value;
		return 0;
	}

	return 1;
}

#endif /* APKC_DEPLOY_DEBUGGER_INTEGRATION_H */
