/* sec_input_validation.h — Input Validation & Fuzzing (Stage 11.1)
 *
 * Deterministic input validation for all runtime surfaces.
 * Fuzz testing with bounded input generation.
 * Type safety and bounds checking without abstractions.
 * Constraint validation: size limits, format compliance, encoding.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_INPUT_VALIDATION_H
#define APKC_SEC_INPUT_VALIDATION_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Input validation result */
enum ValidationStatus {
	VAL_OK = 0,           /* Valid input, safe to use */
	VAL_SIZE_EXCEEDED = 1, /* Buffer/size constraint violated */
	VAL_FORMAT_INVALID = 2, /* Format constraint violated */
	VAL_ENCODING_BAD = 3,  /* Encoding/charset constraint violated */
	VAL_OVERFLOW = 4,     /* Arithmetic overflow detected */
	VAL_NULL_PTR = 5,     /* NULL pointer where required */
	VAL_ALIGNMENT = 6,    /* Memory alignment constraint violated */
	VAL_RANGE = 7         /* Value outside valid range */
};

/* Validation context for bounded fuzz testing */
struct FuzzContext {
	u8 seed[32];          /* Deterministic seed (SHA256 hash) */
	u32 iteration;        /* Fuzz iteration counter */
	u32 max_iterations;   /* Bounded fuzz limit */
	u8 test_buffer[4096]; /* Buffer for fuzzing */
	u32 buffer_size;      /* Current buffer size */
};

/* ============================================================ */
/* BASIC INPUT VALIDATION */
/* ============================================================ */

/* Validate pointer is non-null */
static inline u8 val_ptr_nonnull(const void *ptr) {
	return (ptr == 0) ? VAL_NULL_PTR : VAL_OK;
}

/* Validate size is within bounds */
static inline u8 val_size_bounds(u32 size, u32 max_size) {
	if (size > max_size) return VAL_SIZE_EXCEEDED;
	return VAL_OK;
}

/* Validate alignment (power of 2) */
static inline u8 val_alignment(u64 addr, u32 alignment) {
	if ((addr & (alignment - 1)) != 0) return VAL_ALIGNMENT;
	return VAL_OK;
}

/* Validate range [min, max] */
static inline u8 val_range(u64 value, u64 min_val, u64 max_val) {
	if (value < min_val || value > max_val) return VAL_RANGE;
	return VAL_OK;
}

/* Validate no arithmetic overflow in addition */
static inline u8 val_add_overflow(u64 a, u64 b, u64 *result) {
	if (a > (0xffffffffffffffff - b)) return VAL_OVERFLOW;
	*result = a + b;
	return VAL_OK;
}

/* Validate no arithmetic overflow in multiplication */
static inline u8 val_mul_overflow(u64 a, u64 b, u64 *result) {
	if (b > 0 && a > (0xffffffffffffffff / b)) return VAL_OVERFLOW;
	*result = a * b;
	return VAL_OK;
}

/* ============================================================ */
/* BUFFER & FORMAT VALIDATION */
/* ============================================================ */

/* Validate buffer contains only ASCII printable */
static inline u8 val_ascii_printable(const u8 *buf, u32 len) {
	u32 i;
	for (i = 0; i < len; i++) {
		u8 c = buf[i];
		if (c < 32 || c > 126) {
			if (c != 9 && c != 10 && c != 13) { /* Allow tab, LF, CR */
				return VAL_ENCODING_BAD;
			}
		}
	}
	return VAL_OK;
}

/* Validate UTF-8 encoding */
static inline u8 val_utf8(const u8 *buf, u32 len) {
	u32 i = 0;
	while (i < len) {
		u8 c = buf[i];
		if (c < 0x80) {
			i++;
		} else if ((c & 0xe0) == 0xc0) {
			if (i + 1 >= len) return VAL_ENCODING_BAD;
			if ((buf[i+1] & 0xc0) != 0x80) return VAL_ENCODING_BAD;
			i += 2;
		} else if ((c & 0xf0) == 0xe0) {
			if (i + 2 >= len) return VAL_ENCODING_BAD;
			if ((buf[i+1] & 0xc0) != 0x80 || (buf[i+2] & 0xc0) != 0x80) {
				return VAL_ENCODING_BAD;
			}
			i += 3;
		} else if ((c & 0xf8) == 0xf0) {
			if (i + 3 >= len) return VAL_ENCODING_BAD;
			if ((buf[i+1] & 0xc0) != 0x80 || (buf[i+2] & 0xc0) != 0x80 ||
				(buf[i+3] & 0xc0) != 0x80) {
				return VAL_ENCODING_BAD;
			}
			i += 4;
		} else {
			return VAL_ENCODING_BAD;
		}
	}
	return VAL_OK;
}

/* Validate hex string format (0-9, a-f, A-F) */
static inline u8 val_hex_string(const u8 *buf, u32 len) {
	u32 i;
	for (i = 0; i < len; i++) {
		u8 c = buf[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
			  (c >= 'A' && c <= 'F'))) {
			return VAL_FORMAT_INVALID;
		}
	}
	return VAL_OK;
}

/* ============================================================ */
/* DETERMINISTIC FUZZ TESTING */
/* ============================================================ */

/* Initialize fuzz context with seed */
static inline void fuzz_init(struct FuzzContext *fc, const u8 *seed, u32 seed_len) {
	if (!fc) return;
	fc->iteration = 0;
	fc->max_iterations = 256;  /* Bounded: max 256 fuzz iterations */
	fc->buffer_size = 0;

	u32 i;
	for (i = 0; i < 32 && i < seed_len; i++) {
		fc->seed[i] = seed[i];
	}
	for (; i < 32; i++) {
		fc->seed[i] = 0;
	}
}

/* Generate next fuzz input (deterministic) */
static inline u32 fuzz_generate(struct FuzzContext *fc, u32 target_size) {
	if (!fc || fc->iteration >= fc->max_iterations) return 0;

	/* Deterministic PRNG from seed: murmur3-like mixing */
	u64 h = fc->iteration;
	u32 i;

	for (i = 0; i < 32; i++) {
		h ^= fc->seed[i];
		h *= 0x85ebca6b;
		h ^= h >> 32;
	}

	fc->buffer_size = (h % (target_size + 1));

	/* Fill buffer with deterministic pattern */
	for (i = 0; i < fc->buffer_size && i < 4096; i++) {
		fc->test_buffer[i] = (u8)((h >> (i % 8)) & 0xff);
		h = h * 1103515245 + 12345;  /* Linear congruential */
	}

	fc->iteration++;
	return fc->buffer_size;
}

/* Check if fuzz testing complete */
static inline u8 fuzz_done(struct FuzzContext *fc) {
	if (!fc) return 1;
	return (fc->iteration >= fc->max_iterations) ? 1 : 0;
}

/* ============================================================ */
/* CONSTRAINT VALIDATION GATE */
/* ============================================================ */

/* Composite validation: all constraints at once */
static inline u8 val_comprehensive(
	const void *ptr,
	u32 size,
	u32 max_size,
	u32 alignment) {

	if (val_ptr_nonnull(ptr) != VAL_OK) return VAL_NULL_PTR;
	if (val_size_bounds(size, max_size) != VAL_OK) return VAL_SIZE_EXCEEDED;
	if (val_alignment((u64)ptr, alignment) != VAL_OK) return VAL_ALIGNMENT;
	return VAL_OK;
}

/* Validation report for logging */
struct ValidationReport {
	u8 status;           /* ValidationStatus code */
	u32 iteration;       /* Which test caught failure (if fuzz) */
	u64 failed_value;    /* Value that failed validation */
	const char *message; /* Error description */
};

#endif /* APKC_SEC_INPUT_VALIDATION_H */
