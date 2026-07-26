#include "RAF_rafaelia_common.h"

/*
 * Método M062: Âncora quaternária — 4 ciclos íntegros, 8 gates,
 * quinto estado derivado.
 *
 * Alvo: MCU / bare-metal / Linux / Android
 * Domínio: integridade, scheduler, tolerância a ruído
 * Estado: implementação C estática, sem heap, selftest host-runnable.
 *
 * Invariante:
 *   quatro órgãos independentes, cada um com gate próprio e gate do par;
 *   o quinto estado não é uma quinta task nem um supervisor;
 *   ele só é promovido quando os oito gates fecham, as quatro assinaturas
 *   convergem e todos os órgãos avançaram desde o commit anterior.
 */

#define M062_ORGANS       4u
#define M062_GATES        8u
#define M062_STALE_LIMIT  3u

typedef enum {
    M062_PULSAR = 0,
    M062_SENTINEL = 1,
    M062_MEMORY = 2,
    M062_ACTION = 3
} rafaelia_m062_role_t;

typedef enum {
    M062_WARMUP_OR_NOISE = 0,
    M062_NOMINAL = 1,
    M062_ACCELERATION_OBSERVED = 2,
    M062_LATENCY_OR_NOISE = 3,
    M062_INPUT_TRANSITION_OR_NOISE = 4,
    M062_RESTART_OR_NOISE = 5,
    M062_FAULT_CANDIDATE_STALLED = 6,
    M062_FAULT_CANDIDATE_ABSENT = 7,
    M062_FAULT_CANDIDATE_REGRESSION = 8
} rafaelia_m062_class_t;

typedef struct {
    uint32_t sequence;
    uint32_t last_peer_sequence;
    uint64_t core_signature;
    uint8_t stale_cycles;
    uint8_t self_gate;
    uint8_t peer_gate;
    uint8_t classification;
} rafaelia_m062_organ_t;

typedef struct {
    rafaelia_m062_organ_t organ[M062_ORGANS];
    uint32_t committed_sequence[M062_ORGANS];
    uint32_t fifth_cycle;
    uint64_t fifth_signature;
    uint8_t fifth_valid;
    uint8_t gates_passed;
} rafaelia_m062_anchor_t;

static uint8_t rafaelia_m062_peer(uint8_t role) {
    static const uint8_t peer[M062_ORGANS] = {
        M062_SENTINEL,
        M062_MEMORY,
        M062_ACTION,
        M062_PULSAR
    };
    return peer[role & 3u];
}

void rafaelia_m062_init(rafaelia_m062_anchor_t *anchor) {
    uint8_t i;
    if (anchor == (rafaelia_m062_anchor_t *)0) {
        return;
    }
    for (i = 0u; i < M062_ORGANS; i++) {
        anchor->organ[i].sequence = 0u;
        anchor->organ[i].last_peer_sequence = 0u;
        anchor->organ[i].core_signature = 0u;
        anchor->organ[i].stale_cycles = 0u;
        anchor->organ[i].self_gate = 0u;
        anchor->organ[i].peer_gate = 0u;
        anchor->organ[i].classification = (uint8_t)M062_WARMUP_OR_NOISE;
        anchor->committed_sequence[i] = 0u;
    }
    anchor->fifth_cycle = 0u;
    anchor->fifth_signature = 0u;
    anchor->fifth_valid = 0u;
    anchor->gates_passed = 0u;
}

static uint8_t rafaelia_m062_try_fifth(rafaelia_m062_anchor_t *anchor) {
    uint8_t i;
    uint8_t gates = 0u;
    uint8_t advanced = 1u;
    uint64_t signature;

    signature = anchor->organ[0].core_signature;
    for (i = 0u; i < M062_ORGANS; i++) {
        gates = (uint8_t)(gates + anchor->organ[i].self_gate
                         + anchor->organ[i].peer_gate);
        if (anchor->organ[i].sequence <= anchor->committed_sequence[i]) {
            advanced = 0u;
        }
        if (anchor->organ[i].core_signature != signature) {
            advanced = 0u;
        }
    }

    anchor->gates_passed = gates;
    if ((gates != M062_GATES) || (advanced == 0u)) {
        return 0u;
    }

    for (i = 0u; i < M062_ORGANS; i++) {
        anchor->committed_sequence[i] = anchor->organ[i].sequence;
    }
    anchor->fifth_cycle++;
    anchor->fifth_signature = signature;
    anchor->fifth_valid = 1u;
    return 1u;
}

uint8_t rafaelia_m062_publish(
    rafaelia_m062_anchor_t *anchor,
    rafaelia_m062_role_t role,
    uint64_t core_signature,
    uint8_t self_integrity
) {
    uint8_t r;
    uint8_t p;
    uint32_t peer_sequence;
    uint32_t previous_peer_sequence;
    rafaelia_m062_organ_t *own;
    const rafaelia_m062_organ_t *peer;

    if (anchor == (rafaelia_m062_anchor_t *)0) {
        return 0u;
    }
    r = (uint8_t)role;
    if (r >= M062_ORGANS) {
        return 0u;
    }

    p = rafaelia_m062_peer(r);
    own = &anchor->organ[r];
    peer = &anchor->organ[p];
    peer_sequence = peer->sequence;
    previous_peer_sequence = own->last_peer_sequence;

    own->sequence++;
    own->core_signature = core_signature;
    own->self_gate = (uint8_t)(self_integrity != 0u);
    own->peer_gate = 0u;

    if (peer_sequence == 0u) {
        if (own->stale_cycles < 255u) {
            own->stale_cycles++;
        }
        own->classification = (own->stale_cycles >= M062_STALE_LIMIT)
            ? (uint8_t)M062_FAULT_CANDIDATE_ABSENT
            : (uint8_t)M062_WARMUP_OR_NOISE;
    } else if (peer_sequence > previous_peer_sequence) {
        uint32_t delta = peer_sequence - previous_peer_sequence;
        own->last_peer_sequence = peer_sequence;
        own->stale_cycles = 0u;
        if (peer->core_signature == core_signature) {
            own->peer_gate = 1u;
            own->classification = (delta == 1u)
                ? (uint8_t)M062_NOMINAL
                : (uint8_t)M062_ACCELERATION_OBSERVED;
        } else {
            own->classification = (uint8_t)M062_INPUT_TRANSITION_OR_NOISE;
        }
    } else if (peer_sequence == previous_peer_sequence) {
        if (own->stale_cycles < 255u) {
            own->stale_cycles++;
        }
        if ((peer->core_signature == core_signature)
            && (own->stale_cycles < M062_STALE_LIMIT)) {
            own->peer_gate = 1u;
            own->classification = (uint8_t)M062_LATENCY_OR_NOISE;
        } else if (own->stale_cycles < M062_STALE_LIMIT) {
            own->classification = (uint8_t)M062_INPUT_TRANSITION_OR_NOISE;
        } else {
            own->classification = (uint8_t)M062_FAULT_CANDIDATE_STALLED;
        }
    } else {
        if (own->stale_cycles < 255u) {
            own->stale_cycles++;
        }
        own->classification = (own->stale_cycles >= M062_STALE_LIMIT)
            ? (uint8_t)M062_FAULT_CANDIDATE_REGRESSION
            : (uint8_t)M062_RESTART_OR_NOISE;
    }

    return rafaelia_m062_try_fifth(anchor);
}

int rafaelia_m062_selftest(void) {
    rafaelia_m062_anchor_t anchor;
    uint8_t round;
    uint8_t role;
    uint32_t first_fifth;

    rafaelia_m062_init(&anchor);

    for (round = 0u; round < 2u; round++) {
        for (role = 0u; role < M062_ORGANS; role++) {
            (void)rafaelia_m062_publish(
                &anchor,
                (rafaelia_m062_role_t)role,
                0xA11CE5EEDULL,
                1u
            );
        }
    }
    if ((anchor.fifth_cycle == 0u)
        || (anchor.fifth_valid == 0u)
        || (anchor.gates_passed != M062_GATES)) {
        return -1;
    }
    first_fifth = anchor.fifth_cycle;

    (void)rafaelia_m062_publish(
        &anchor,
        M062_PULSAR,
        0xB22CE5EEDULL,
        1u
    );
    if (anchor.fifth_cycle != first_fifth) {
        return -1;
    }
    if (anchor.organ[M062_PULSAR].classification
        != (uint8_t)M062_INPUT_TRANSITION_OR_NOISE) {
        return -1;
    }

    for (round = 0u; round < 2u; round++) {
        for (role = 0u; role < M062_ORGANS; role++) {
            (void)rafaelia_m062_publish(
                &anchor,
                (rafaelia_m062_role_t)role,
                0xB22CE5EEDULL,
                1u
            );
        }
    }
    if (anchor.fifth_cycle <= first_fifth) {
        return -1;
    }

    for (round = 0u; round <= M062_STALE_LIMIT; round++) {
        (void)rafaelia_m062_publish(
            &anchor,
            M062_PULSAR,
            0xB22CE5EEDULL,
            1u
        );
    }
    if (anchor.organ[M062_PULSAR].classification
        != (uint8_t)M062_FAULT_CANDIDATE_STALLED) {
        return -1;
    }

    return 0;
}

#ifdef RAFAELIA_M062_SELFTEST_MAIN
int main(void) {
    return rafaelia_m062_selftest();
}
#endif
