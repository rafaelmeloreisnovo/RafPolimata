/* deploy_apk_signing.h — APK integrity/signing boundary (Stage 9.2 hardening)
 *
 * IMPORTANT EVIDENCE BOUNDARY:
 * - This freestanding module DOES NOT implement RSA, ECDSA, APK Signature Scheme,
 *   X.509 chain validation, or cryptographic SHA-256.
 * - integrity_hash32_substitute() is deterministic integrity instrumentation only.
 * - Any request for RSA/ECDSA signing fails closed until a real implementation is wired.
 * - claim_allowed remains false outside this header until cryptographic/provider/runtime proof exists.
 *
 * FREESTANDING: no malloc, no libc, stack-only/fixed-size storage.
 */
#ifndef APKC_DEPLOY_APK_SIGNING_H
#define APKC_DEPLOY_APK_SIGNING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

enum SigningAlgorithm {
    SIGN_SHA256_RSA = 0,
    SIGN_SHA256_EC = 1,
    SIGN_NONE = 2
};

enum SigningCapability {
    SIGN_CAPABILITY_UNIMPLEMENTED = 0,
    SIGN_CAPABILITY_INTEGRITY_SUBSTITUTE_ONLY = 1
};

struct Certificate {
    const u8 *subject;
    u32 subject_len;
    const u8 *issuer;
    u32 issuer_len;
    u64 serial;
    u32 not_before;
    u32 not_after;
    const u8 *fingerprint;
    u8 self_signed;
};

struct SigningKey {
    const u8 *key_data;
    u32 key_len;
    enum SigningAlgorithm algo;
    struct Certificate cert;
    u8 valid;
};

/* Own the bytes. Never retain pointers to temporary stack arrays. */
struct ApkSignature {
    u8 signature[32];
    u32 signature_len;
    u8 content_hash[32];
    struct Certificate cert;
    enum SigningAlgorithm algo;
    u32 signed_at;
    enum SigningCapability capability;
};

struct IntegrityReport {
    u8 is_signed;
    u8 signature_valid;
    u8 content_match;
    u8 not_tampered;
    enum SigningAlgorithm algo;
    const u8 *signature_issue;
    u32 issue_len;
    u64 verified_at;
    enum SigningCapability capability;
};

static inline void apkc_zero32(u8 *p) {
    u32 i;
    if (!p) return;
    for (i = 0; i < 32; i++) p[i] = 0;
}

static inline void apkc_copy32(u8 *dst, const u8 *src) {
    u32 i;
    if (!dst || !src) return;
    for (i = 0; i < 32; i++) dst[i] = src[i];
}

static inline u8 apkc_equal32(const u8 *a, const u8 *b) {
    u32 i;
    u8 diff = 0;
    if (!a || !b) return 0;
    for (i = 0; i < 32; i++) diff |= (u8)(a[i] ^ b[i]);
    return (u8)(diff == 0);
}

static inline u8 signing_key_init(
    struct SigningKey *key,
    const u8 *key_data, u32 key_len,
    enum SigningAlgorithm algo) {
    if (!key || !key_data || key_len == 0) return 1;
    key->key_data = key_data;
    key->key_len = key_len;
    key->algo = algo;
    /* A key blob being present is not proof that RSA/ECDSA is implemented. */
    key->valid = 1;
    key->cert.subject = (const u8*)"CN=APKC";
    key->cert.subject_len = 7;
    key->cert.issuer = (const u8*)"CN=APKC";
    key->cert.issuer_len = 7;
    key->cert.serial = 0x0123456789ABCDEFULL;
    key->cert.not_before = 0;
    key->cert.not_after = 0xFFFFFFFFU;
    key->cert.fingerprint = (const u8*)0;
    key->cert.self_signed = 1;
    return 0;
}

static inline u8 signing_key_load(
    struct SigningKey *key,
    const u8 *key_path, u32 path_len,
    enum SigningAlgorithm algo) {
    if (!key || !key_path || path_len == 0) return 1;
    key->key_data = (const u8*)0;
    key->key_len = 0;
    key->algo = algo;
    key->valid = 0;
    return 1;
}

static inline void signing_key_set_cert(
    struct SigningKey *key,
    const u8 *subject, u32 subject_len,
    const u8 *issuer, u32 issuer_len,
    u64 serial) {
    if (!key) return;
    key->cert.subject = subject;
    key->cert.subject_len = subject_len;
    key->cert.issuer = issuer;
    key->cert.issuer_len = issuer_len;
    key->cert.serial = serial;
}

/* Deterministic 32-byte integrity substitute. NOT SHA-256 and NOT cryptographic. */
static inline void integrity_hash32_substitute(
    const u8 *data, u32 len, u8 *hash_out) {
    u32 i;
    if (!hash_out) return;
    apkc_zero32(hash_out);
    if (!data && len != 0) return;
    for (i = 0; i < len; i++) {
        hash_out[i % 32] ^= data[i];
        hash_out[(i + 1U) % 32] ^= (u8)((data[i] << 1) | (data[i] >> 7));
        hash_out[(i + 2U) % 32] ^= (u8)((data[i] >> 1) | (data[i] << 7));
    }
    hash_out[0] ^= (u8)((len >> 0) & 0xFFU);
    hash_out[1] ^= (u8)((len >> 8) & 0xFFU);
    hash_out[2] ^= (u8)((len >> 16) & 0xFFU);
    hash_out[3] ^= (u8)((len >> 24) & 0xFFU);
}

/* Compatibility alias, intentionally documented as non-cryptographic. */
static inline void sha256_simple(const u8 *data, u32 len, u8 *hash_out) {
    integrity_hash32_substitute(data, len, hash_out);
}

static inline u8 signing_hash_content(
    const u8 *apk_data, u32 apk_size, u8 *hash_out) {
    if ((!apk_data && apk_size != 0) || !hash_out) return 1;
    integrity_hash32_substitute(apk_data, apk_size, hash_out);
    return 0;
}

static inline u8 signing_hash_sections(
    const u8 *code_section, u32 code_len,
    const u8 *resource_section, u32 resource_len,
    const u8 *manifest_section, u32 manifest_len,
    u8 *hash_out) {
    u8 code_hash[32], resource_hash[32], manifest_hash[32];
    u32 i;
    if (!hash_out) return 1;
    if ((!code_section && code_len) || (!resource_section && resource_len) ||
        (!manifest_section && manifest_len)) return 1;
    apkc_zero32(code_hash);
    apkc_zero32(resource_hash);
    apkc_zero32(manifest_hash);
    if (code_section) integrity_hash32_substitute(code_section, code_len, code_hash);
    if (resource_section) integrity_hash32_substitute(resource_section, resource_len, resource_hash);
    if (manifest_section) integrity_hash32_substitute(manifest_section, manifest_len, manifest_hash);
    for (i = 0; i < 32; i++)
        hash_out[i] = (u8)(code_hash[i] ^ resource_hash[i] ^ manifest_hash[i]);
    return 0;
}

/* Cryptographic signing is intentionally unavailable until implemented/proved. */
static inline u8 signing_sign_apk(
    struct SigningKey *key,
    const u8 *apk_data, u32 apk_size,
    struct ApkSignature *out_sig) {
    u8 h[32];
    if (!key || !apk_data || apk_size == 0 || !out_sig) return 1;
    apkc_zero32(out_sig->signature);
    apkc_zero32(out_sig->content_hash);
    out_sig->signature_len = 0;
    out_sig->algo = key->algo;
    out_sig->cert = key->cert;
    out_sig->signed_at = 0;
    out_sig->capability = SIGN_CAPABILITY_INTEGRITY_SUBSTITUTE_ONLY;
    if (signing_hash_content(apk_data, apk_size, h)) return 1;
    apkc_copy32(out_sig->content_hash, h);
    /* Never claim RSA/ECDSA signature from the substitute hash. */
    return 2;
}

static inline u8 signing_verify_apk(
    const u8 *apk_data, u32 apk_size,
    struct ApkSignature *sig,
    struct IntegrityReport *out_report) {
    u8 current_hash[32];
    u8 match;
    if (!apk_data || apk_size == 0 || !sig || !out_report) return 1;
    out_report->is_signed = 0;
    out_report->signature_valid = 0;
    out_report->content_match = 0;
    out_report->not_tampered = 0;
    out_report->algo = sig->algo;
    out_report->verified_at = 0;
    out_report->capability = SIGN_CAPABILITY_INTEGRITY_SUBSTITUTE_ONLY;
    out_report->signature_issue = (const u8*)"cryptographic signing unimplemented";
    out_report->issue_len = 35;
    if (signing_hash_content(apk_data, apk_size, current_hash)) return 1;
    match = apkc_equal32(current_hash, sig->content_hash);
    out_report->content_match = match;
    /* A matching substitute hash is integrity instrumentation, not signature validity. */
    return 2;
}

static inline struct Certificate signing_extract_cert(struct ApkSignature *sig) {
    struct Certificate cert = {0};
    if (sig) return sig->cert;
    return cert;
}

static inline u8 signing_cert_is_valid(struct Certificate *cert, u32 current_time) {
    if (!cert) return 0;
    if (current_time < cert->not_before || current_time > cert->not_after) return 0;
    return 1;
}

static inline u8 signing_cert_pin_check(
    struct Certificate *cert, const u8 *pinned_fingerprint) {
    if (!cert || !cert->fingerprint || !pinned_fingerprint) return 0;
    return apkc_equal32(cert->fingerprint, pinned_fingerprint);
}

static inline u8 signing_verify_integrity(
    const u8 *apk_data, u32 apk_size,
    struct ApkSignature *sig,
    struct IntegrityReport *out_report) {
    /* Integrity hash comparison can be observed, but cryptographic verification is absent. */
    return signing_verify_apk(apk_data, apk_size, sig, out_report);
}

static inline u32 signing_report_format(
    struct IntegrityReport *report, u8 *buf, u32 buf_size) {
    const u8 *prefix = (const u8*)"Integrity[";
    const u8 *state;
    u32 i = 0, j = 0;
    if (!report || !buf || buf_size < 24) return 0;
    while (prefix[j] && i < buf_size - 1) buf[i++] = prefix[j++];
    state = report->signature_valid ? (const u8*)"CRYPTO_OK" :
            (report->content_match ? (const u8*)"HASH_MATCH_ONLY" : (const u8*)"FAIL");
    j = 0;
    while (state[j] && i < buf_size - 1) buf[i++] = state[j++];
    if (i < buf_size - 1) buf[i++] = ']';
    buf[i] = 0;
    return i;
}

#endif /* APKC_DEPLOY_APK_SIGNING_H */
