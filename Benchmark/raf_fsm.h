/* raf_fsm.h — FSM 10 estados (domínios RAFAELIA) + estimador Lyapunov Q16
 * Lyapunov discreto: lambda = (1/N)*Sigma log2(|delta[k]|/|delta[0]|)
 * Classificação: SOURCE(>0) LIMIT(≈0) SPIRAL(-0.05) TORUS(-0.14) STRANGE
 * Transition table is direct O(1) lookup; branchlessness is a codegen fact,
 * not asserted from source text alone.                                      */
#pragma once
#include "raf_types.h"
#include "raf_q16.h"

/* 10 estados — índices de domínio RAFAELIA                                  */
/* 0=MATH 1=COSMO 2=QUANTUM 3=BIO 4=ENG 5=CONSCIOUS 6=AI 7=SYMBOLIC
   8=THERMO 9=FIELD                                                           */

/* Tabela de transição: fsm_trans[estado][input] → novo_estado
 * Branch-free: compilador emite lea + movsxd, zero branch                   */
static const u8 FSM_TRANS[FSM_N][FSM_IN] = {
    /* MATH     */ {1, 2, 0, 9},
    /* COSMO    */ {0, 3, 1, 8},
    /* QUANTUM  */ {3, 0, 4, 7},
    /* BIO      */ {2, 4, 3, 6},
    /* ENG      */ {3, 5, 4, 5},
    /* CONSCIOUS*/ {4, 6, 5, 4},
    /* AI       */ {5, 7, 6, 3},
    /* SYMBOLIC */ {6, 8, 7, 2},
    /* THERMO   */ {7, 9, 8, 1},
    /* FIELD    */ {8, 0, 9, 0},
};

/* Output por estado — peso ético em Q16 [0, Q16_ONE]                        */
static const q16_t FSM_WEIGHT[FSM_N] = {
    Q16_SQRT3_2, Q16_PHI>>2, Q16_HALF,     Q16_SQRT3_2,  Q16_ONE,
    Q16_PHI>>2,  Q16_SQRT3_2, Q16_HALF,   Q16_ONE,       Q16_PHI>>2,
};

typedef struct {
    u8   state;          /* estado atual [0..9]                              */
    u32  step;           /* contador de transições                            */
    q16_t acc_weight;    /* acumulador de peso ético — Phi_ethica             */
    /* Lyapunov window — 16 amostras de divergência                          */
    s32  div_log[16];    /* log2(|delta|) por passo — Q16 estimado           */
    u32  div_idx;        /* índice circular                                   */
    q16_t lambda;        /* expoente de Lyapunov estimado — atualizado a/16  */
} FSMState;

/* Classificação de atrator baseada em lambda — Eq. análoga a A14            */
/* Thresholds em Q16 (dividir por 65536 para valor real):
   SOURCE:   lambda > 819   (>0.0125/step)
   LIMIT:    |lambda| < 819
   SPIRAL:   -3277 < lambda < -819  (-0.05 a -0.0125)
   TORUS:    -9830 < lambda < -3277 (-0.15 a -0.05) ← KAM-estável
   STRANGE:  -32768 < lambda < -9830
   HOMOCLINIC: lambda < -32768                                               */
#define LYAP_SOURCE_THR     819
#define LYAP_LIMIT_THR      819
#define LYAP_SPIRAL_THR    3277
#define LYAP_TORUS_THR     9830
#define LYAP_STRANGE_THR  32768

typedef enum {
    ATT_SOURCE=0, ATT_LIMIT, ATT_SPIRAL, ATT_TORUS, ATT_STRANGE, ATT_HOMO
} AttractorClass;

static inline AttractorClass fsm_classify(q16_t lambda) {
    /* Branch-free via cascata de comparações — sem if-else no hot path       */
    u32 gt_src  = (u32)(lambda >  LYAP_SOURCE_THR);
    u32 lt_hom  = (u32)(lambda < -LYAP_STRANGE_THR);
    u32 lt_str  = (u32)(lambda < -LYAP_TORUS_THR);
    u32 lt_tor  = (u32)(lambda < -LYAP_SPIRAL_THR);
    u32 lt_spi  = (u32)(lambda < -LYAP_LIMIT_THR);
    /* Comparison results are 0/1. Keep selection arithmetic explicit and
     * avoid mixing logical-not with bitwise operators. */
    return (AttractorClass)(lt_hom*5 +
            ((1u-lt_hom) & lt_str)*4 +
            ((1u-lt_str) & lt_tor)*3 +
            ((1u-lt_tor) & lt_spi)*2 +
            ((1u-lt_spi) & (1u-gt_src))*1);
}

static void fsm_init(FSMState *f) {
    f->state = 0;
    f->step = 0;
    f->acc_weight = 0;
    for (u32 i = 0; i < 16; i++) f->div_log[i] = 0;
    f->div_idx = 0;
    f->lambda = Q16_NEG_LN_S; /* explicit prior/reference value */
}

static void fsm_step(FSMState *f, u32 input) {
    /* Transição O(1) lookup — input clampado em [0,FSM_IN) sem branch       */
    u32 inp = input & (FSM_IN - 1);
    u8  ns  = FSM_TRANS[f->state][inp];
    /* Divergência estimada como diferença de pesos entre estados             */
    q16_t w_old = FSM_WEIGHT[f->state];
    q16_t w_new = FSM_WEIGHT[ns];
    q16_t delta = w_new - w_old;
    q16_t adelta = q16_abs(delta) | 1; /* OR 1: evita log(0)                */
    /* log2 inteiro do delta — estimativa Lyapunov discreta                  */
    f->div_log[f->div_idx & 15] = q16_log2i((u32)adelta);
    f->div_idx++;
    /* Atualiza lambda a cada 16 steps — média da janela                     */
    if ((f->div_idx & 15) == 0) {
        s32 sum = 0;
        for (u32 i = 0; i < 16; i++) sum += f->div_log[i];
        /* Converte para Q16: (sum/16) << 10 — scaling para range útil      */
        f->lambda = (q16_t)((sum << 10) / 16);
    }
    f->acc_weight = q16_iir(f->acc_weight, FSM_WEIGHT[ns]);
    f->state      = ns;
    f->step++;
}

static const char* fsm_domain_name(u8 s) {
    static const char* names[FSM_N] = {
        "MATH","COSMO","QUANTUM","BIO","ENG",
        "CONSCIOUS","AI","SYMBOLIC","THERMO","FIELD"
    };
    if (s >= FSM_N) return "INVALID_STATE";
    return names[s];
}
static const char* attractor_name(AttractorClass a) {
    static const char* names[] = {
        "SOURCE","LIMIT","SPIRAL","TORUS","STRANGE","HOMOCLINIC"
    };
    u32 idx = (u32)a;
    if (idx >= 6U) return "INVALID_CLASS";
    return names[idx];
}
