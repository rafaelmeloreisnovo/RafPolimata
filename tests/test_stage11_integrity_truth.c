#include <stdio.h>
#include "Apkc/sec_hardening_gates.h"

static int fail = 0;
#define CHECK(name, expr) do { if (expr) { printf("PASS %s\n", name); } else { printf("FAIL %s\n", name); fail++; } } while (0)

int main(void) {
    struct HardeningContext hc = {0};
    u8 state[] = {1,2,3,4};
    u8 changed[] = {9,2,3,4};

    harden_init(&hc);
    CHECK("noncrypto_capability", HARDEN_INTEGRITY_CRYPTOGRAPHIC == 0);
    CHECK("unsealed_verify_rejects", harden_verify_seal(&hc, state, 4) == GATE_REJECT);
    CHECK("bootstrap_checkpoint_passes", harden_checkpoint(&hc, "boot", state, 4, 1, 1, 1) == GATE_PASS);
    CHECK("counter_exact_after_bootstrap", hc.seal.seal_counter == 1);
    CHECK("sealed_verify_passes", harden_verify_seal(&hc, state, 4) == GATE_PASS);
    CHECK("tamper_rejects", harden_verify_seal(&hc, changed, 4) == GATE_REJECT);

    harden_init(&hc);
    hc.seal.seal_counter = 7;
    CHECK("nonpristine_zero_seal_rejects", harden_checkpoint(&hc, "forged", state, 4, 1, 1, 8) == GATE_REJECT);

    harden_init(&hc);
    CHECK("bootstrap_counter_replay_rejects", harden_checkpoint(&hc, "zero", state, 4, 1, 1, 0) == GATE_REJECT);

    printf("RESULT pass=%d fail=%d claim_allowed=false\n", 8 - fail, fail);
    return fail ? 1 : 0;
}
