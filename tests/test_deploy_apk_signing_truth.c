#include "../Apkc/deploy_apk_signing.h"

static int eq32(const u8 *a, const u8 *b) {
    u32 i;
    for (i = 0; i < 32; i++) if (a[i] != b[i]) return 0;
    return 1;
}

int main(void) {
    static const u8 apk[] = {0x50,0x4b,0x03,0x04,1,2,3,4,5,6,7,8};
    static const u8 key_bytes[] = {1,2,3,4};
    static const u8 section[] = {9,8,7,6};
    struct SigningKey key;
    struct ApkSignature sig;
    struct IntegrityReport report;
    u8 expected[32], sections_a[32], sections_b[32];
    u8 tampered[sizeof(apk)];
    u32 i;

    if (signing_key_init(&key, key_bytes, sizeof(key_bytes), SIGN_SHA256_RSA) != 0) return 10;
    integrity_hash32_substitute(apk, sizeof(apk), expected);

    /* Crypto must fail closed, but owned integrity bytes remain inspectable. */
    if (signing_sign_apk(&key, apk, sizeof(apk), &sig) != 2) return 11;
    if (sig.signature_len != 0) return 12;
    if (sig.capability != SIGN_CAPABILITY_INTEGRITY_SUBSTITUTE_ONLY) return 13;
    if (!eq32(sig.content_hash, expected)) return 14;

    /* Verification can report hash equality but never cryptographic validity. */
    if (signing_verify_apk(apk, sizeof(apk), &sig, &report) != 2) return 15;
    if (!report.content_match) return 16;
    if (report.signature_valid || report.is_signed || report.not_tampered) return 17;

    /* Tampering must break the integrity substitute match. */
    for (i = 0; i < sizeof(apk); i++) tampered[i] = apk[i];
    tampered[5] ^= 0x80;
    if (signing_verify_apk(tampered, sizeof(tampered), &sig, &report) != 2) return 18;
    if (report.content_match) return 19;

    /* Missing sections are deterministic zeros, not uninitialized stack data. */
    if (signing_hash_sections(section, sizeof(section), 0, 0, 0, 0, sections_a) != 0) return 20;
    if (signing_hash_sections(section, sizeof(section), 0, 0, 0, 0, sections_b) != 0) return 21;
    if (!eq32(sections_a, sections_b)) return 22;

    /* NULL with nonzero length is invalid and must fail closed. */
    if (signing_hash_sections(0, 1, 0, 0, 0, 0, sections_a) == 0) return 23;

    return 0;
}
