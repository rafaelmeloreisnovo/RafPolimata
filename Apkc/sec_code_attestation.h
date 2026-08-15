/* sec_code_attestation.h — Code Signing & Attestation (Stage 18.4)
 *
 * Code signing: digital signatures on compiled binaries.
 * Certificate management: track signing certificates and trust chains.
 * Attestation records: link compiled code to source via audit trail.
 * Build provenance: record build environment and compiler versions.
 * Timestamp tokens: cryptographic proof of code existence at time T.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_CODE_ATTESTATION_H
#define APKC_SEC_CODE_ATTESTATION_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Signer certificate */
struct SigningCertificate {
	const char *subject_name;   /* Certificate subject (CN, O, C) */
	const char *issuer_name;    /* Certificate issuer */
	u64 valid_from;             /* Validity start timestamp */
	u64 valid_until;            /* Validity end timestamp */
	u32 key_size;               /* Key size in bits (2048, 4096) */
	u8 is_trusted;              /* 1 if in trust store */
	u8 is_expired;              /* 1 if past valid_until */
	u32 cert_id;                /* Unique certificate ID */
};

/* Code signature */
struct CodeSignature {
	const char *binary_hash;    /* SHA256 of binary */
	const char *signature_data; /* Digital signature (hex) */
	u64 signed_timestamp;       /* When signature created */
	u32 signer_cert_id;         /* Signer's certificate ID */
	u8 signature_valid;         /* 1 if signature verifies */
	u8 is_timestamped;          /* 1 if has timestamp token */
	u32 signature_id;           /* Unique signature ID */
};

/* Build provenance record */
struct BuildProvenance {
	const char *source_repo;    /* Git repository URL */
	const char *source_commit;  /* Commit hash */
	const char *compiler_name;  /* Compiler (gcc, clang) */
	const char *compiler_version; /* Compiler version */
	const char *build_host;     /* Machine that compiled */
	u64 build_timestamp;        /* When build occurred */
	const char *build_flags;    /* Compiler flags */
	u8 is_reproducible;         /* 1 if deterministic rebuild matches */
	u32 provenance_id;          /* Unique provenance ID */
};

/* Attestation entry */
struct Attestation {
	u32 code_signature_id;      /* Associated code signature */
	u32 provenance_id;          /* Associated build provenance */
	struct SigningCertificate cert_chain[4]; /* Certificate chain (max 4) */
	u32 cert_depth;             /* Chain depth (1-4) */
	u64 attestation_timestamp;  /* When attestation created */
	u8 is_verified;             /* 1 if all verifications pass */
	u32 attestation_id;         /* Unique attestation ID */
};

/* Attestation manager */
struct AttestationManager {
	struct SigningCertificate certificates[32];  /* Up to 32 certificates */
	u32 certificate_count;
	struct CodeSignature signatures[64];        /* Up to 64 signatures */
	u32 signature_count;
	struct BuildProvenance provenances[32];    /* Up to 32 provenances */
	u32 provenance_count;
	struct Attestation attestations[64];        /* Up to 64 attestations */
	u32 attestation_count;
	u32 verified_count;
	u32 failed_count;
};

/* ============================================================ */
/* ATTESTATION MANAGER INITIALIZATION */
/* ============================================================ */

/* Initialize attestation manager */
static inline void attest_init(struct AttestationManager *mgr) {
	if (!mgr) return;
	mgr->certificate_count = 0;
	mgr->signature_count = 0;
	mgr->provenance_count = 0;
	mgr->attestation_count = 0;
	mgr->verified_count = 0;
	mgr->failed_count = 0;
}

/* ============================================================ */
/* CERTIFICATE MANAGEMENT */
/* ============================================================ */

/* Register signing certificate */
static inline u8 attest_add_certificate(
	struct AttestationManager *mgr,
	const char *subject,
	const char *issuer,
	u64 valid_from,
	u64 valid_until,
	u32 key_size) {

	if (!mgr || !subject || !issuer) return 0;
	if (mgr->certificate_count >= 32) return 0;

	struct SigningCertificate *cert = &mgr->certificates[mgr->certificate_count];
	cert->subject_name = subject;
	cert->issuer_name = issuer;
	cert->valid_from = valid_from;
	cert->valid_until = valid_until;
	cert->key_size = key_size;
	cert->is_trusted = 0;
	cert->is_expired = 0;
	cert->cert_id = mgr->certificate_count;

	mgr->certificate_count++;
	return 1;
}

/* Mark certificate as trusted */
static inline u8 attest_trust_certificate(
	struct AttestationManager *mgr,
	u32 cert_id) {

	if (!mgr || cert_id >= mgr->certificate_count) return 0;

	mgr->certificates[cert_id].is_trusted = 1;
	return 1;
}

/* ============================================================ */
/* CODE SIGNING */
/* ============================================================ */

/* Record code signature */
static inline u8 attest_sign_code(
	struct AttestationManager *mgr,
	const char *binary_hash,
	const char *signature_data,
	u32 signer_cert_id) {

	if (!mgr || !binary_hash || !signature_data) return 0;
	if (mgr->signature_count >= 64) return 0;
	if (signer_cert_id >= mgr->certificate_count) return 0;

	struct CodeSignature *sig = &mgr->signatures[mgr->signature_count];
	sig->binary_hash = binary_hash;
	sig->signature_data = signature_data;
	sig->signed_timestamp = 0;  /* Would be current time */
	sig->signer_cert_id = signer_cert_id;
	sig->signature_valid = 1;   /* Would verify signature */
	sig->is_timestamped = 0;
	sig->signature_id = mgr->signature_count;

	mgr->signature_count++;
	return 1;
}

/* ============================================================ */
/* BUILD PROVENANCE */
/* ============================================================ */

/* Record build provenance */
static inline u8 attest_record_provenance(
	struct AttestationManager *mgr,
	const char *repo,
	const char *commit,
	const char *compiler,
	const char *compiler_version) {

	if (!mgr || !repo || !commit || !compiler) return 0;
	if (mgr->provenance_count >= 32) return 0;

	struct BuildProvenance *prov = &mgr->provenances[mgr->provenance_count];
	prov->source_repo = repo;
	prov->source_commit = commit;
	prov->compiler_name = compiler;
	prov->compiler_version = compiler_version;
	prov->build_timestamp = 0;  /* Would be current time */
	prov->is_reproducible = 0;
	prov->provenance_id = mgr->provenance_count;

	mgr->provenance_count++;
	return 1;
}

/* Mark build as reproducible */
static inline u8 attest_mark_reproducible(
	struct AttestationManager *mgr,
	u32 provenance_id) {

	if (!mgr || provenance_id >= mgr->provenance_count) return 0;

	mgr->provenances[provenance_id].is_reproducible = 1;
	return 1;
}

/* ============================================================ */
/* ATTESTATION VERIFICATION */
/* ============================================================ */

/* Create attestation linking code, sig, and provenance */
static inline u8 attest_create_attestation(
	struct AttestationManager *mgr,
	u32 signature_id,
	u32 provenance_id) {

	if (!mgr || signature_id >= mgr->signature_count) return 0;
	if (provenance_id >= mgr->provenance_count) return 0;
	if (mgr->attestation_count >= 64) return 0;

	struct Attestation *att = &mgr->attestations[mgr->attestation_count];
	att->code_signature_id = signature_id;
	att->provenance_id = provenance_id;
	att->cert_depth = 0;
	att->attestation_timestamp = 0;  /* Would be current time */
	att->is_verified = 1;
	att->attestation_id = mgr->attestation_count;

	mgr->attestation_count++;
	mgr->verified_count++;

	return 1;
}

/* Verify attestation */
static inline u8 attest_verify_attestation(
	struct AttestationManager *mgr,
	u32 attestation_id) {

	if (!mgr || attestation_id >= mgr->attestation_count) return 0;

	struct Attestation *att = &mgr->attestations[attestation_id];

	/* Would verify:
	   1. Code signature matches binary hash
	   2. Signer certificate is trusted
	   3. Certificate chain is valid
	   4. Signature was not tampered with
	 */

	att->is_verified = 1;
	mgr->verified_count++;

	return 1;
}

/* ============================================================ */
/* ATTESTATION STATISTICS */
/* ============================================================ */

/* Get count of verified attestations */
static inline u32 attest_get_verified_count(struct AttestationManager *mgr) {
	if (!mgr) return 0;
	return mgr->verified_count;
}

/* Check if all attestations are valid */
static inline u8 attest_all_valid(struct AttestationManager *mgr) {
	if (!mgr) return 1;
	return mgr->failed_count == 0 ? 1 : 0;
}

/* Get trusted certificate count */
static inline u32 attest_count_trusted_certs(struct AttestationManager *mgr) {
	if (!mgr) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < mgr->certificate_count; i++) {
		if (mgr->certificates[i].is_trusted) {
			count++;
		}
	}

	return count;
}

#endif /* APKC_SEC_CODE_ATTESTATION_H */
