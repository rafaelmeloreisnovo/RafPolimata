/* deploy_profiling.h — Performance Profiling & Error Reporting (Stage 9.4)
 *
 * Instruction-level profiling (execution time per instruction).
 * Function profiling (call count, total time, self time).
 * Memory profiling (allocation rate, peak usage, lifetime).
 * Error reporting (exceptions, crashes, assertion failures).
 * Telemetry payload with bounded memory (ring buffer).
 * Stateless functions; no global state except data structures.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DEPLOY_PROFILING_H
#define APKC_DEPLOY_PROFILING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Function profile data */
struct FunctionProfile {
	const u8 *func_name;       /* Function name */
	u32 func_name_len;
	u64 func_addr;             /* Function address */
	u32 call_count;            /* Number of calls */
	u64 total_time_us;         /* Total time in microseconds */
	u64 self_time_us;          /* Time excluding subcalls */
	u64 min_time_us;           /* Minimum call duration */
	u64 max_time_us;           /* Maximum call duration */
};

/* Per-instruction timing */
struct InstructionProfile {
	u64 insn_addr;             /* Instruction address */
	u32 execute_count;         /* Number of executions */
	u64 total_time_us;         /* Total execution time */
};

/* Memory allocation profile */
struct MemoryAlloc {
	u64 alloc_addr;            /* Allocated address */
	u32 alloc_size;            /* Allocated size */
	u32 alloc_id;              /* Allocation ID */
	u64 timestamp;             /* Allocation timestamp */
	u8 freed;                  /* 1 if allocation freed */
};

/* Memory profile summary */
struct MemoryProfile {
	u32 total_allocations;     /* Total allocation count */
	u32 total_deallocations;   /* Total deallocation count */
	u64 total_bytes_alloc;     /* Total bytes allocated */
	u64 peak_bytes;            /* Peak memory usage */
	u64 current_bytes;         /* Current memory usage */
	u32 active_allocations;    /* Current active allocations */
	struct MemoryAlloc allocations[256];  /* Up to 256 tracked allocations */
	u32 alloc_count;
};

/* Error report */
struct ErrorReport {
	u32 error_code;            /* Error code */
	const u8 *error_type;      /* Error type name */
	u32 error_type_len;
	const u8 *error_message;   /* Error message */
	u32 error_msg_len;
	u64 timestamp;             /* When error occurred */
	u64 faulting_address;      /* Address that faulted */
	u32 faulting_instruction;  /* Instruction that faulted */
	const u8 *source_file;     /* Source file */
	u32 source_file_len;
	u32 source_line;           /* Source line */
};

/* Profiling context */
struct ProfilingContext {
	u8 enabled;                /* 1 if profiling enabled */
	u64 start_time;            /* Profile start timestamp */
	u64 end_time;              /* Profile end timestamp */
	struct FunctionProfile functions[64];  /* Up to 64 functions */
	u32 function_count;
	struct InstructionProfile instructions[256];  /* Up to 256 instructions */
	u32 instruction_count;
	struct MemoryProfile memory;
	struct ErrorReport errors[16];  /* Up to 16 errors */
	u32 error_count;
};

/* ============================================================ */
/* PROFILING CONTROL */
/* ============================================================ */

/* Initialize profiling context */
static inline void profiling_init(struct ProfilingContext *prof) {
	if (!prof) return;

	prof->enabled = 0;
	prof->start_time = 0;
	prof->end_time = 0;
	prof->function_count = 0;
	prof->instruction_count = 0;
	prof->error_count = 0;

	/* Initialize memory profile */
	prof->memory.total_allocations = 0;
	prof->memory.total_deallocations = 0;
	prof->memory.total_bytes_alloc = 0;
	prof->memory.peak_bytes = 0;
	prof->memory.current_bytes = 0;
	prof->memory.active_allocations = 0;
	prof->memory.alloc_count = 0;
}

/* Start profiling */
static inline void profiling_start(struct ProfilingContext *prof) {
	if (!prof) return;

	prof->enabled = 1;
	prof->start_time = 0;  /* Would be current time */
	prof->end_time = 0;
	prof->function_count = 0;
	prof->instruction_count = 0;
	prof->error_count = 0;
}

/* Stop profiling */
static inline void profiling_stop(struct ProfilingContext *prof) {
	if (!prof) return;

	prof->enabled = 0;
	prof->end_time = 0;  /* Would be current time */
}

/* Reset profiling data */
static inline void profiling_reset(struct ProfilingContext *prof) {
	if (!prof) return;

	profiling_init(prof);
}

/* ============================================================ */
/* FUNCTION PROFILING */
/* ============================================================ */

/* Record function entry (start call timing) */
static inline u8 profiling_record_call(
	struct ProfilingContext *prof,
	const u8 *func_name, u32 func_len,
	u64 func_addr) {

	if (!prof || !prof->enabled || prof->function_count >= 64) return 1;

	/* Find or create function profile entry */
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		if (prof->functions[i].func_addr == func_addr) {
			/* Found existing entry; increment call count */
			prof->functions[i].call_count++;
			return 0;
		}
	}

	/* Create new entry */
	struct FunctionProfile *fp = &prof->functions[prof->function_count];
	fp->func_name = func_name;
	fp->func_name_len = func_len;
	fp->func_addr = func_addr;
	fp->call_count = 1;
	fp->total_time_us = 0;
	fp->self_time_us = 0;
	fp->min_time_us = 0xFFFFFFFFFFFFFFFFULL;
	fp->max_time_us = 0;

	prof->function_count++;
	return 0;
}

/* Record function return (end call timing) */
static inline u8 profiling_record_return(
	struct ProfilingContext *prof,
	u64 func_addr,
	u64 call_duration_us) {

	if (!prof || !prof->enabled) return 1;

	/* Find function profile and update timing */
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		if (prof->functions[i].func_addr == func_addr) {
			struct FunctionProfile *fp = &prof->functions[i];
			fp->total_time_us += call_duration_us;

			if (call_duration_us < fp->min_time_us) {
				fp->min_time_us = call_duration_us;
			}
			if (call_duration_us > fp->max_time_us) {
				fp->max_time_us = call_duration_us;
			}

			return 0;
		}
	}

	return 1;  /* Function not found */
}

/* ============================================================ */
/* INSTRUCTION PROFILING */
/* ============================================================ */

/* Record instruction execution */
static inline u8 profiling_record_insn(
	struct ProfilingContext *prof,
	u64 insn_addr,
	u64 execution_time_us) {

	if (!prof || !prof->enabled || prof->instruction_count >= 256) return 1;

	/* Find or create instruction profile */
	u32 i;
	for (i = 0; i < prof->instruction_count; i++) {
		if (prof->instructions[i].insn_addr == insn_addr) {
			prof->instructions[i].execute_count++;
			prof->instructions[i].total_time_us += execution_time_us;
			return 0;
		}
	}

	/* Create new entry */
	struct InstructionProfile *ip = &prof->instructions[prof->instruction_count];
	ip->insn_addr = insn_addr;
	ip->execute_count = 1;
	ip->total_time_us = execution_time_us;

	prof->instruction_count++;
	return 0;
}

/* ============================================================ */
/* MEMORY PROFILING */
/* ============================================================ */

/* Record memory allocation */
static inline u8 profiling_record_alloc(
	struct ProfilingContext *prof,
	u64 alloc_addr, u32 alloc_size) {

	if (!prof || !prof->enabled) return 1;

	struct MemoryProfile *mem = &prof->memory;

	if (mem->alloc_count >= 256) {
		/* Ring buffer: overwrite oldest */
		mem->alloc_count = 256;
	}

	u32 idx = mem->total_allocations % 256;
	struct MemoryAlloc *ma = &mem->allocations[idx];
	ma->alloc_addr = alloc_addr;
	ma->alloc_size = alloc_size;
	ma->alloc_id = mem->total_allocations;
	ma->timestamp = 0;  /* Would be current time */
	ma->freed = 0;

	mem->total_allocations++;
	mem->total_bytes_alloc += alloc_size;
	mem->current_bytes += alloc_size;
	mem->active_allocations++;

	if (mem->current_bytes > mem->peak_bytes) {
		mem->peak_bytes = mem->current_bytes;
	}

	return 0;
}

/* Record memory deallocation */
static inline u8 profiling_record_free(
	struct ProfilingContext *prof,
	u64 free_addr, u32 free_size) {

	if (!prof || !prof->enabled) return 1;

	struct MemoryProfile *mem = &prof->memory;

	/* Find allocation entry */
	u32 i;
	for (i = 0; i < mem->alloc_count; i++) {
		if (mem->allocations[i].alloc_addr == free_addr && !mem->allocations[i].freed) {
			mem->allocations[i].freed = 1;
			mem->total_deallocations++;
			mem->current_bytes -= free_size;
			if (mem->active_allocations > 0) {
				mem->active_allocations--;
			}
			return 0;
		}
	}

	return 1;  /* Allocation not found */
}

/* ============================================================ */
/* ERROR REPORTING */
/* ============================================================ */

/* Report an error occurrence */
static inline u8 profiling_report_error(
	struct ProfilingContext *prof,
	u32 error_code,
	const u8 *error_type, u32 type_len,
	const u8 *error_msg, u32 msg_len,
	u64 faulting_addr) {

	if (!prof || prof->error_count >= 16) return 1;

	struct ErrorReport *err = &prof->errors[prof->error_count];
	err->error_code = error_code;
	err->error_type = error_type;
	err->error_type_len = type_len;
	err->error_message = error_msg;
	err->error_msg_len = msg_len;
	err->timestamp = 0;  /* Would be current time */
	err->faulting_address = faulting_addr;
	err->faulting_instruction = 0;
	err->source_file = NULL;
	err->source_file_len = 0;
	err->source_line = 0;

	prof->error_count++;
	return 0;
}

/* ============================================================ */
/* REPORT FORMATTING */
/* ============================================================ */

/* Format profiling report as human-readable string */
static inline u32 profiling_format_report(
	struct ProfilingContext *prof,
	u8 *buf, u32 buf_size) {

	if (!prof || !buf || buf_size < 200) return 0;

	const u8 *hdr = (const u8*)"Profile Report:\n";
	u32 i = 0;
	while (hdr[i] && i < buf_size - 1) {
		buf[i] = hdr[i];
		i++;
	}

	/* Function summary */
	if (i < buf_size - 50 && prof->function_count > 0) {
		const u8 *fsec = (const u8*)"Functions: ";
		u32 j = 0;
		while (fsec[j] && i < buf_size - 1) {
			buf[i++] = fsec[j++];
		}

		u32 cnt = prof->function_count;
		u32 div = 10;
		while (div >= 1 && i < buf_size - 1) {
			u8 digit = (cnt / div) % 10;
			if (digit || div <= cnt) {
				buf[i++] = '0' + digit;
			}
			div /= 10;
		}

		if (i < buf_size - 1) buf[i++] = '\n';
	}

	/* Memory summary */
	if (i < buf_size - 50) {
		const u8 *msec = (const u8*)"Memory Peak: ";
		u32 j = 0;
		while (msec[j] && i < buf_size - 1) {
			buf[i++] = msec[j++];
		}

		u64 peak = prof->memory.peak_bytes / 1024;  /* Convert to KB */
		u32 div = 100000;
		while (div >= 1 && i < buf_size - 1) {
			u8 digit = (peak / div) % 10;
			if (digit || div <= peak) {
				buf[i++] = '0' + digit;
			}
			div /= 10;
		}

		if (i < buf_size - 3) {
			buf[i++] = 'K';
			buf[i++] = 'B';
			buf[i++] = '\n';
		}
	}

	/* Error summary */
	if (i < buf_size - 30 && prof->error_count > 0) {
		const u8 *esec = (const u8*)"Errors: ";
		u32 j = 0;
		while (esec[j] && i < buf_size - 1) {
			buf[i++] = esec[j++];
		}

		u32 cnt = prof->error_count;
		buf[i++] = '0' + (cnt % 10);
		if (i < buf_size - 1) buf[i++] = '\n';
	}

	buf[i] = '\0';
	return i;
}

/* ============================================================ */
/* TELEMETRY */
/* ============================================================ */

/* Telemetry payload for sending to backend */
struct TelemetryPayload {
	u32 total_functions;       /* Total functions profiled */
	u32 total_errors;          /* Total errors reported */
	u64 peak_memory;           /* Peak memory usage */
	u64 total_runtime_us;      /* Total runtime in microseconds */
	u32 payload_size;          /* Payload size in bytes */
};

/* Create telemetry payload from profile */
static inline struct TelemetryPayload profiling_create_telemetry(
	struct ProfilingContext *prof) {

	struct TelemetryPayload tp = {0};

	if (prof) {
		tp.total_functions = prof->function_count;
		tp.total_errors = prof->error_count;
		tp.peak_memory = prof->memory.peak_bytes;
		tp.total_runtime_us = (prof->end_time > prof->start_time) ?
			(prof->end_time - prof->start_time) : 0;
		tp.payload_size = sizeof(struct TelemetryPayload);
	}

	return tp;
}

#endif /* APKC_DEPLOY_PROFILING_H */
