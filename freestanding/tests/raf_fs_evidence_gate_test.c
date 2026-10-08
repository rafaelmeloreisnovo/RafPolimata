#include "../include/raf_fs_evidence_gate.h"

/* Hosted test entrypoint only. Tested policy remains freestanding/no-CRT. */
/* CLOSURE_L15: test marks are synthetic; missing P0/device evidence stays blocked. */
#define CHECK(cond, code) do { if (!(cond)) return (code); } while (0)

int main(void)
{
    raf_fs_gate_input a = {
        RAF_FS_EV_OBSERVED, RAF_FS_EV_OBSERVED, RAF_FS_EV_OBSERVED,
        RAF_FS_EV_OBSERVED, RAF_FS_EV_OBSERVED, RAF_FS_EV_OBSERVED,
        RAF_FS_OUTCOME_PASS, RAF_FS_EV_OBSERVED, RAF_FS_EV_OBSERVED,
        RAF_FS_EV_OBSERVED
    };
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_PASS_SCOPED, 1);
    CHECK(raf_fs_gate_evaluate((const raf_fs_gate_input *)0) == RAF_FS_GATE_BAD_INPUT, 2);

    a.source = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_SOURCE_BLOCKED, 3);
    a.source = RAF_FS_EV_OBSERVED;
    a.rights = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_RIGHTS_BLOCKED, 4);
    a.rights = RAF_FS_EV_OBSERVED;
    a.authority = RAF_FS_EV_REJECTED;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_AUTHORITY_BLOCKED, 5);
    a.authority = RAF_FS_EV_OBSERVED;
    a.target = RAF_FS_EV_NOT_APPLICABLE;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_TARGET_BLOCKED, 6);
    a.target = RAF_FS_EV_OBSERVED;
    a.evidence_rule = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_RULE_BLOCKED, 7);
    a.evidence_rule = RAF_FS_EV_OBSERVED;
    a.execution = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_NOT_EXECUTED, 8);
    a.execution = RAF_FS_EV_OBSERVED;
    a.outcome = RAF_FS_OUTCOME_FAIL;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_EXECUTION_FAILED, 9);
    a.outcome = RAF_FS_OUTCOME_NOT_RUN;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_OUTCOME_UNKNOWN, 10);
    a.outcome = RAF_FS_OUTCOME_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_OUTCOME_UNKNOWN, 11);
    a.outcome = RAF_FS_OUTCOME_PASS;
    a.artifact_identity = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_ARTIFACT_BLOCKED, 12);
    a.artifact_identity = RAF_FS_EV_OBSERVED;
    a.exact_head = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_HEAD_BLOCKED, 13);
    a.exact_head = RAF_FS_EV_OBSERVED;
    a.receipt = RAF_FS_EV_TOKEN_VAZIO;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_RECEIPT_BLOCKED, 14);
    a.receipt = RAF_FS_EV_OBSERVED;

    a.rights = (raf_fs_ev_mark)0;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_BAD_INPUT, 15);
    a.rights = RAF_FS_EV_OBSERVED;
    a.outcome = (raf_fs_ev_outcome)0;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_BAD_INPUT, 16);
    a.outcome = RAF_FS_OUTCOME_PASS;
    a.execution = RAF_FS_EV_REJECTED;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_NOT_EXECUTED, 17);
    a.execution = RAF_FS_EV_OBSERVED;
    CHECK(raf_fs_gate_evaluate(&a) == RAF_FS_GATE_PASS_SCOPED, 18);
    CHECK(RAF_FS_EV_TOKEN_VAZIO != 0, 19);
    CHECK(RAF_FS_EV_TOKEN_VAZIO != RAF_FS_EV_NOT_APPLICABLE, 20);
    CHECK(RAF_FS_OUTCOME_NOT_RUN != RAF_FS_OUTCOME_FAIL, 21);
    return 0;
}
