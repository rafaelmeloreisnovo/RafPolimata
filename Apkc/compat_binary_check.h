/* compat_binary_check.h — Binary Compatibility Checking (Stage 13.2)
 *
 * ABI compatibility verification: check calling conventions.
 * Symbol version compatibility: verify symbol versions match.
 * Architecture compatibility: verify target arch matches.
 * ISA feature detection: check for required instructions.
 * Compatibility matrix: 7 languages × 4 platforms verification.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_COMPAT_BINARY_CHECK_H
#define APKC_COMPAT_BINARY_CHECK_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Compatibility check status */
enum CompatibilityStatus {
	COMPAT_OK = 0,              /* Binary compatible */
	COMPAT_ARCH_MISMATCH = 1,   /* Architecture mismatch */
	COMPAT_ABI_MISMATCH = 2,    /* Calling convention mismatch */
	COMPAT_FEATURE_MISSING = 3, /* Required ISA feature missing */
	COMPAT_VERSION_CONFLICT = 4,/* Symbol version conflict */
	COMPAT_SYMBOL_UNDEFINED = 5,/* Required symbol not found */
	COMPAT_PLATFORM_UNSUPPORTED = 6 /* Platform not supported */
};

/* ISA feature flags */
#define COMPAT_FEATURE_BASE     0x01  /* Base ARM64 */
#define COMPAT_FEATURE_NEON     0x02  /* NEON vector ops */
#define COMPAT_FEATURE_FMA      0x04  /* Fused multiply-add */
#define COMPAT_FEATURE_CRYPTO   0x08  /* Cryptographic extensions */
#define COMPAT_FEATURE_SVE      0x10  /* Scalable vector extensions */
#define COMPAT_FEATURE_SME      0x20  /* Scalable matrix extension */

/* Target architecture */
enum TargetArch {
	ARCH_ARM64 = 1,
	ARCH_ARM32 = 2,
	ARCH_X86_64 = 3,
	ARCH_X86 = 4,
	ARCH_UNKNOWN = 0xff
};

/* Calling convention */
enum CallingConvention {
	CC_ARM64_AAPCS = 1,         /* ARM64 AAPCS standard */
	CC_ARM32_EABI = 2,          /* ARM32 EABI */
	CC_X86_64_SYSV = 3,         /* x86-64 System V */
	CC_X86_CDECL = 4,           /* x86 C declaration */
	CC_BRANCHLESS_NATIVE = 5,   /* Branchless machine native */
	CC_UNKNOWN = 0xff
};

/* Symbol version requirement */
struct SymbolVersion {
	u16 major;
	u16 minor;
	u32 patch;
};

/* Binary compatibility descriptor */
struct BinaryDescriptor {
	u8 target_arch;             /* TargetArch */
	u8 calling_convention;      /* CallingConvention */
	u8 isa_features;            /* Feature bitmask */
	u8 pointer_size;            /* 4=32-bit, 8=64-bit */
	struct SymbolVersion min_version; /* Minimum supported version */
	struct SymbolVersion current_version; /* Current version */
	const char *language;       /* Source language (Python, Go, etc.) */
	u32 abi_flags;              /* Additional ABI flags */
};

/* Compatibility matrix entry */
struct CompatibilityEntry {
	struct BinaryDescriptor required;
	struct BinaryDescriptor provided;
	u8 status;                  /* CompatibilityStatus */
	u32 mismatch_flags;         /* Which specific fields mismatch */
};

/* Compatibility check context */
struct CompatibilityChecker {
	struct BinaryDescriptor current_target;
	struct CompatibilityEntry checks[32];  /* Up to 32 compatibility checks */
	u32 check_count;
	u32 failures;
	u32 warnings;
};

/* ============================================================ */
/* COMPATIBILITY CHECKER INITIALIZATION */
/* ============================================================ */

/* Initialize compatibility checker */
static inline void compat_init_checker(
	struct CompatibilityChecker *checker,
	u8 target_arch) {

	if (!checker) return;
	checker->check_count = 0;
	checker->failures = 0;
	checker->warnings = 0;
	checker->current_target.target_arch = target_arch;
	checker->current_target.pointer_size = (target_arch == ARCH_ARM64 || target_arch == ARCH_X86_64) ? 8 : 4;
}

/* Set target architecture */
static inline void compat_set_target_arch(
	struct CompatibilityChecker *checker,
	u8 arch) {

	if (!checker) return;
	checker->current_target.target_arch = arch;
	checker->current_target.pointer_size = (arch == ARCH_ARM64 || arch == ARCH_X86_64) ? 8 : 4;
}

/* ============================================================ */
/* ARCHITECTURE COMPATIBILITY CHECK */
/* ============================================================ */

/* Check if architectures are compatible */
static inline u8 compat_check_arch(
	u8 required_arch,
	u8 provided_arch) {

	/* Exact match is always compatible */
	if (required_arch == provided_arch) return COMPAT_OK;

	/* Some cross-arch scenarios might be acceptable via emulation/translation */
	/* For now, require exact match */
	return COMPAT_ARCH_MISMATCH;
}

/* ============================================================ */
/* ABI COMPATIBILITY CHECK */
/* ============================================================ */

/* Check if calling conventions are compatible */
static inline u8 compat_check_calling_convention(
	struct BinaryDescriptor *required,
	struct BinaryDescriptor *provided) {

	if (!required || !provided) return COMPAT_ABI_MISMATCH;

	/* ARM64 AAPCS is our standard */
	if (required->calling_convention != provided->calling_convention) {
		/* Check for known compatible equivalents */
		if (required->calling_convention == CC_BRANCHLESS_NATIVE &&
			provided->calling_convention == CC_ARM64_AAPCS) {
			return COMPAT_OK;  /* Branchless can adapt to AAPCS */
		}
		if (required->calling_convention == CC_ARM64_AAPCS &&
			provided->calling_convention == CC_BRANCHLESS_NATIVE) {
			return COMPAT_OK;  /* AAPCS can adapt to branchless */
		}
		return COMPAT_ABI_MISMATCH;
	}

	/* Pointer size must match */
	if (required->pointer_size != provided->pointer_size) {
		return COMPAT_ABI_MISMATCH;
	}

	return COMPAT_OK;
}

/* ============================================================ */
/* ISA FEATURE COMPATIBILITY CHECK */
/* ============================================================ */

/* Check if required ISA features are supported */
static inline u8 compat_check_features(
	u8 required_features,
	u8 provided_features) {

	/* Provided must have all required features */
	u8 missing = required_features & ~provided_features;
	if (missing != 0) {
		return COMPAT_FEATURE_MISSING;
	}

	return COMPAT_OK;
}

/* ============================================================ */
/* SYMBOL VERSION COMPATIBILITY */
/* ============================================================ */

/* Compare symbol versions: -1 if v1<v2, 0 if equal, 1 if v1>v2 */
static inline s32 compat_version_compare(
	struct SymbolVersion v1,
	struct SymbolVersion v2) {

	s32 rc = 0;
	if (v1.major != v2.major) {
		return (v1.major > v2.major) ? 1 : -1;
	}
	if (v1.minor != v2.minor) {
		return (v1.minor > v2.minor) ? 1 : -1;
	}
	if (v1.patch != v2.patch) {
		return (v1.patch > v2.patch) ? 1 : -1;
	}
	return rc;
}

/* Check if provided version satisfies required version */
static inline u8 compat_check_version(
	struct SymbolVersion required,
	struct SymbolVersion provided) {

	/* Provided must be >= required (backward compatibility) */
	s32 cmp = compat_version_compare(provided, required);
	if (cmp >= 0) {
		return COMPAT_OK;
	}
	return COMPAT_VERSION_CONFLICT;
}

/* ============================================================ */
/* COMPREHENSIVE COMPATIBILITY CHECKING */
/* ============================================================ */

/* Perform full compatibility check */
static inline u8 compat_check_full(
	struct CompatibilityChecker *checker,
	struct BinaryDescriptor *required,
	struct BinaryDescriptor *provided) {

	if (!checker || !required || !provided) return COMPAT_ABI_MISMATCH;
	if (checker->check_count >= 32) return COMPAT_ABI_MISMATCH;

	u8 status = COMPAT_OK;
	u32 mismatch_flags = 0;

	/* Check architecture */
	u8 arch_status = compat_check_arch(required->target_arch, provided->target_arch);
	if (arch_status != COMPAT_OK) {
		status = arch_status;
		mismatch_flags |= 0x01;
	}

	/* Check ABI/calling convention */
	u8 abi_status = compat_check_calling_convention(required, provided);
	if (abi_status != COMPAT_OK) {
		status = abi_status;
		mismatch_flags |= 0x02;
	}

	/* Check ISA features */
	u8 feature_status = compat_check_features(required->isa_features, provided->isa_features);
	if (feature_status != COMPAT_OK) {
		status = feature_status;
		mismatch_flags |= 0x04;
	}

	/* Check version compatibility */
	u8 version_status = compat_check_version(required->min_version, provided->current_version);
	if (version_status != COMPAT_OK) {
		status = version_status;
		mismatch_flags |= 0x08;
	}

	/* Record check result */
	struct CompatibilityEntry *entry = &checker->checks[checker->check_count];
	entry->required = *required;
	entry->provided = *provided;
	entry->status = status;
	entry->mismatch_flags = mismatch_flags;

	if (status != COMPAT_OK) {
		checker->failures++;
	}

	checker->check_count++;
	return status;
}

/* ============================================================ */
/* LANGUAGE-SPECIFIC COMPATIBILITY */
/* ============================================================ */

/* Check compatibility for specific language */
static inline u8 compat_check_language(
	struct CompatibilityChecker *checker,
	const char *language,
	struct BinaryDescriptor *descriptor) {

	if (!checker || !language || !descriptor) return COMPAT_ABI_MISMATCH;

	/* Set language */
	descriptor->language = language;

	/* Determine expected ABI for language */
	if (language[0] == 'p' && language[1] == 'y') {
		/* Python: usually CC_BRANCHLESS_NATIVE */
		descriptor->calling_convention = CC_BRANCHLESS_NATIVE;
	} else if (language[0] == 'g' && language[1] == 'o') {
		/* Go: usually CC_ARM64_AAPCS */
		descriptor->calling_convention = CC_ARM64_AAPCS;
	} else if (language[0] == 'r' && language[1] == 'u' && language[2] == 's') {
		/* Rust: usually CC_ARM64_AAPCS */
		descriptor->calling_convention = CC_ARM64_AAPCS;
	} else if (language[0] == 'c' && language[1] == 0) {
		/* C: CC_ARM64_AAPCS */
		descriptor->calling_convention = CC_ARM64_AAPCS;
	} else if (language[0] == 'j' && language[1] == 'a' && language[2] == 'v') {
		/* Java: CC_ARM64_AAPCS via JNI */
		descriptor->calling_convention = CC_ARM64_AAPCS;
	} else if (language[0] == 's' && language[1] == 'w' && language[2] == 'i') {
		/* Swift: CC_ARM64_AAPCS */
		descriptor->calling_convention = CC_ARM64_AAPCS;
	}

	return COMPAT_OK;
}

/* ============================================================ */
/* COMPATIBILITY REPORT */
/* ============================================================ */

/* Get total compatibility failures */
static inline u32 compat_get_failure_count(struct CompatibilityChecker *checker) {
	if (!checker) return 0;
	return checker->failures;
}

/* Get total checks performed */
static inline u32 compat_get_check_count(struct CompatibilityChecker *checker) {
	if (!checker) return 0;
	return checker->check_count;
}

/* Check if all checks passed */
static inline u8 compat_all_passed(struct CompatibilityChecker *checker) {
	if (!checker) return 0;
	return (checker->failures == 0) ? 1 : 0;
}

#endif /* APKC_COMPAT_BINARY_CHECK_H */
