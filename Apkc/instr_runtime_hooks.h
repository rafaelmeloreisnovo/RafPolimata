/* instr_runtime_hooks.h — Runtime Instrumentation & Hooks (Stage 16.4)
 *
 * Instrumentation points: embed hooks in compiled code for monitoring.
 * Hook callbacks: register handlers for entry/exit/exception/memory events.
 * Runtime event capture: log events to circular buffer for analysis.
 * Hook filtering: enable/disable hooks by type or location.
 * Minimal overhead: hooks only active when explicitly enabled.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_INSTR_RUNTIME_HOOKS_H
#define APKC_INSTR_RUNTIME_HOOKS_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Hook event types */
enum HookEventType {
	HOOK_FUNCTION_ENTRY = 0,    /* Function entry point */
	HOOK_FUNCTION_EXIT = 1,     /* Function return */
	HOOK_MEMORY_ALLOC = 2,      /* Memory allocation */
	HOOK_MEMORY_FREE = 3,       /* Memory deallocation */
	HOOK_EXCEPTION = 4,         /* Exception/error thrown */
	HOOK_BREAKPOINT = 5,        /* Breakpoint hit */
	HOOK_ASSERTION = 6,         /* Assertion failure */
	HOOK_CUSTOM = 7             /* User-defined hook */
};

/* Hook event record */
struct HookEvent {
	u8 event_type;              /* HookEventType */
	u32 location_id;            /* Function/instruction ID */
	u64 timestamp;              /* Event timestamp (ns since start) */
	u64 context_data;           /* Event-specific data (return value, address, etc.) */
	u32 call_depth;             /* Current call stack depth */
	u8 is_enabled;              /* 1 if hook is active */
};

/* Hook callback type */
typedef void (*HookCallback)(struct HookEvent *event);

/* Hook registration entry */
struct HookRegistration {
	u32 hook_id;                /* Unique hook identifier */
	u8 event_type;              /* HookEventType to trigger on */
	u32 location_id;            /* Location filter (0 = all locations) */
	HookCallback callback;      /* Callback function pointer */
	u8 is_enabled;              /* 1 if hook is active */
	u32 hit_count;              /* Times this hook has fired */
};

/* Hook event ring buffer */
struct HookEventBuffer {
	struct HookEvent events[256];   /* Circular buffer of events */
	u32 write_pos;                 /* Current write position */
	u32 total_events_logged;       /* Total events ever logged */
	u32 buffer_overflow_count;     /* Times buffer wrapped around */
	u64 start_time;                /* Timestamp of first event */
};

/* Runtime instrumentation manager */
struct InstrumentationContext {
	struct HookRegistration hooks[32];    /* Up to 32 registered hooks */
	u32 hook_count;
	struct HookEventBuffer event_buffer;
	u8 instrumentation_enabled;
	u32 next_hook_id;
	u32 total_hooks_fired;
	u32 max_call_depth;              /* Maximum call depth observed */
	u32 current_call_depth;          /* Current call stack depth */
};

/* ============================================================ */
/* INSTRUMENTATION INITIALIZATION */
/* ============================================================ */

/* Initialize instrumentation context */
static inline void instr_init(struct InstrumentationContext *ctx) {
	if (!ctx) return;
	ctx->hook_count = 0;
	ctx->instrumentation_enabled = 0;
	ctx->next_hook_id = 1;
	ctx->total_hooks_fired = 0;
	ctx->max_call_depth = 0;
	ctx->current_call_depth = 0;
	ctx->event_buffer.write_pos = 0;
	ctx->event_buffer.total_events_logged = 0;
	ctx->event_buffer.buffer_overflow_count = 0;
	ctx->event_buffer.start_time = 0;
}

/* ============================================================ */
/* HOOK REGISTRATION & MANAGEMENT */
/* ============================================================ */

/* Register a hook callback */
static inline u32 instr_register_hook(
	struct InstrumentationContext *ctx,
	u8 event_type,
	u32 location_id,
	HookCallback callback) {

	if (!ctx || !callback) return 0;
	if (ctx->hook_count >= 32) return 0;  /* Hook table full */

	struct HookRegistration *hook = &ctx->hooks[ctx->hook_count];
	hook->hook_id = ctx->next_hook_id++;
	hook->event_type = event_type;
	hook->location_id = location_id;
	hook->callback = callback;
	hook->is_enabled = 1;
	hook->hit_count = 0;

	ctx->hook_count++;
	return hook->hook_id;
}

/* Unregister a hook by ID */
static inline u8 instr_unregister_hook(
	struct InstrumentationContext *ctx,
	u32 hook_id) {

	if (!ctx) return 0;

	u32 i;
	for (i = 0; i < ctx->hook_count; i++) {
		if (ctx->hooks[i].hook_id == hook_id) {
			/* Mark as disabled instead of removing (preserve indices) */
			ctx->hooks[i].is_enabled = 0;
			return 1;
		}
	}

	return 0;  /* Hook not found */
}

/* Enable/disable a hook */
static inline u8 instr_set_hook_enabled(
	struct InstrumentationContext *ctx,
	u32 hook_id,
	u8 enabled) {

	if (!ctx) return 0;

	u32 i;
	for (i = 0; i < ctx->hook_count; i++) {
		if (ctx->hooks[i].hook_id == hook_id) {
			ctx->hooks[i].is_enabled = enabled;
			return 1;
		}
	}

	return 0;
}

/* ============================================================ */
/* INSTRUMENTATION POINTS */
/* ============================================================ */

/* Fire hook event (called from instrumented code) */
static inline void instr_fire_event(
	struct InstrumentationContext *ctx,
	u8 event_type,
	u32 location_id,
	u64 context_data) {

	if (!ctx || !ctx->instrumentation_enabled) return;

	/* Log event to ring buffer */
	struct HookEvent *event = &ctx->event_buffer.events[ctx->event_buffer.write_pos];
	event->event_type = event_type;
	event->location_id = location_id;
	event->timestamp = 0;  /* Would be current time in ns */
	event->context_data = context_data;
	event->call_depth = ctx->current_call_depth;
	event->is_enabled = 1;

	ctx->event_buffer.write_pos = (ctx->event_buffer.write_pos + 1) % 256;
	ctx->event_buffer.total_events_logged++;

	if (ctx->event_buffer.write_pos == 0) {
		ctx->event_buffer.buffer_overflow_count++;
	}

	/* Call registered hook callbacks */
	u32 i;
	for (i = 0; i < ctx->hook_count; i++) {
		struct HookRegistration *hook = &ctx->hooks[i];
		if (!hook->is_enabled) continue;
		if (hook->event_type != event_type) continue;
		if (hook->location_id != 0 && hook->location_id != location_id) continue;

		/* Invoke callback */
		if (hook->callback) {
			hook->callback(event);
		}
		hook->hit_count++;
	}

	ctx->total_hooks_fired++;
}

/* Function entry hook */
static inline void instr_function_entry(
	struct InstrumentationContext *ctx,
	u32 function_id) {

	if (!ctx) return;
	ctx->current_call_depth++;
	if (ctx->current_call_depth > ctx->max_call_depth) {
		ctx->max_call_depth = ctx->current_call_depth;
	}
	instr_fire_event(ctx, HOOK_FUNCTION_ENTRY, function_id, function_id);
}

/* Function exit hook */
static inline void instr_function_exit(
	struct InstrumentationContext *ctx,
	u32 function_id,
	u64 return_value) {

	if (!ctx) return;
	instr_fire_event(ctx, HOOK_FUNCTION_EXIT, function_id, return_value);
	if (ctx->current_call_depth > 0) {
		ctx->current_call_depth--;
	}
}

/* Memory allocation hook */
static inline void instr_memory_alloc(
	struct InstrumentationContext *ctx,
	u64 address,
	u32 size) {

	if (!ctx) return;
	instr_fire_event(ctx, HOOK_MEMORY_ALLOC, 0, (address << 32) | size);
}

/* Memory deallocation hook */
static inline void instr_memory_free(
	struct InstrumentationContext *ctx,
	u64 address) {

	if (!ctx) return;
	instr_fire_event(ctx, HOOK_MEMORY_FREE, 0, address);
}

/* Exception hook */
static inline void instr_exception(
	struct InstrumentationContext *ctx,
	u32 exception_code) {

	if (!ctx) return;
	instr_fire_event(ctx, HOOK_EXCEPTION, 0, exception_code);
}

/* Breakpoint hook */
static inline void instr_breakpoint(
	struct InstrumentationContext *ctx,
	u32 location_id) {

	if (!ctx) return;
	instr_fire_event(ctx, HOOK_BREAKPOINT, location_id, location_id);
}

/* ============================================================ */
/* EVENT BUFFER QUERIES */
/* ============================================================ */

/* Get event from buffer by index */
static inline struct HookEvent *instr_get_event(
	struct InstrumentationContext *ctx,
	u32 index) {

	if (!ctx || index >= 256) return 0;
	return &ctx->event_buffer.events[index];
}

/* Get most recent N events */
static inline u32 instr_get_recent_events(
	struct InstrumentationContext *ctx,
	struct HookEvent *out_events,
	u32 max_count) {

	if (!ctx || !out_events) return 0;

	u32 count = 0;
	u32 read_pos = ctx->event_buffer.write_pos;

	u32 i;
	for (i = 0; i < max_count && i < 256; i++) {
		if (read_pos == 0) {
			read_pos = 255;
		} else {
			read_pos--;
		}

		out_events[count++] = ctx->event_buffer.events[read_pos];
	}

	return count;
}

/* ============================================================ */
/* HOOK STATISTICS & MONITORING */
/* ============================================================ */

/* Get hook hit count */
static inline u32 instr_get_hook_hit_count(
	struct InstrumentationContext *ctx,
	u32 hook_id) {

	if (!ctx) return 0;

	u32 i;
	for (i = 0; i < ctx->hook_count; i++) {
		if (ctx->hooks[i].hook_id == hook_id) {
			return ctx->hooks[i].hit_count;
		}
	}

	return 0;
}

/* Get total events logged */
static inline u32 instr_get_total_events(struct InstrumentationContext *ctx) {
	if (!ctx) return 0;
	return ctx->event_buffer.total_events_logged;
}

/* Get buffer overflow count */
static inline u32 instr_get_overflow_count(struct InstrumentationContext *ctx) {
	if (!ctx) return 0;
	return ctx->event_buffer.buffer_overflow_count;
}

/* Get maximum call depth observed */
static inline u32 instr_get_max_call_depth(struct InstrumentationContext *ctx) {
	if (!ctx) return 0;
	return ctx->max_call_depth;
}

/* Count active hooks by event type */
static inline u32 instr_count_hooks_by_type(
	struct InstrumentationContext *ctx,
	u8 event_type) {

	if (!ctx) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < ctx->hook_count; i++) {
		if (ctx->hooks[i].is_enabled && ctx->hooks[i].event_type == event_type) {
			count++;
		}
	}

	return count;
}

#endif /* APKC_INSTR_RUNTIME_HOOKS_H */
