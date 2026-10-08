#ifndef RAF_FS_EVIDENCE_GATE_H
#define RAF_FS_EVIDENCE_GATE_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Fail-closed, typed promotion of an observed, exact-head execution to a scoped gate outcome.
 * SCOPE: Pure deterministic evidence policy; no provider, filesystem, hash algorithm, clock, libc, OS or syscall.
 * PRECONDITIONS: Caller supplies authentic, independently checked marks; this policy cannot verify their truth.
 * REGISTER_OWNERSHIP: Compiler-allocated scalar registers; input memory is caller-owned.
 * CLOBBERS: NONE; input is read-only.
 * MEMORY_ORDER: NONE; no concurrency or atomic publication semantics.
 * TAIL_SHADOW: No tail processing, heap, history, retry, mutable cache or hidden state.
 * EVIDENCE: Source-only until an exact-head gate runs; build PASS cannot assert device/science/provider PASS.
 * CLOSURE_L15: unknown marks and P0 license/physical gaps remain explicitly unresolved.
 * AUTHORSHIP: New authorial implementation for RafPolimata; no upstream code imported.
 * RIGHTS: Repository-wide redistribution license remains a separate P0 owner decision.
 */

#include "raf_fs_types.h"

typedef enum raf_fs_ev_mark {
    RAF_FS_EV_TOKEN_VAZIO = 0x31,
    RAF_FS_EV_OBSERVED = 0x32,
    RAF_FS_EV_REJECTED = 0x33,
    RAF_FS_EV_NOT_APPLICABLE = 0x34
} raf_fs_ev_mark;

typedef enum raf_fs_ev_outcome {
    RAF_FS_OUTCOME_TOKEN_VAZIO = 0x41,
    RAF_FS_OUTCOME_NOT_RUN = 0x42,
    RAF_FS_OUTCOME_PASS = 0x43,
    RAF_FS_OUTCOME_FAIL = 0x44
} raf_fs_ev_outcome;

typedef enum raf_fs_gate_state {
    RAF_FS_GATE_BAD_INPUT = 0x50,
    RAF_FS_GATE_SOURCE_BLOCKED = 0x51,
    RAF_FS_GATE_RIGHTS_BLOCKED = 0x52,
    RAF_FS_GATE_AUTHORITY_BLOCKED = 0x53,
    RAF_FS_GATE_TARGET_BLOCKED = 0x54,
    RAF_FS_GATE_RULE_BLOCKED = 0x55,
    RAF_FS_GATE_NOT_EXECUTED = 0x56,
    RAF_FS_GATE_EXECUTION_FAILED = 0x57,
    RAF_FS_GATE_OUTCOME_UNKNOWN = 0x58,
    RAF_FS_GATE_ARTIFACT_BLOCKED = 0x59,
    RAF_FS_GATE_HEAD_BLOCKED = 0x5a,
    RAF_FS_GATE_RECEIPT_BLOCKED = 0x5b,
    RAF_FS_GATE_PASS_SCOPED = 0x5c
} raf_fs_gate_state;

typedef struct raf_fs_gate_input {
    raf_fs_ev_mark source;
    raf_fs_ev_mark rights;
    raf_fs_ev_mark authority;
    raf_fs_ev_mark target;
    raf_fs_ev_mark evidence_rule;
    raf_fs_ev_mark execution;
    raf_fs_ev_outcome outcome;
    raf_fs_ev_mark artifact_identity;
    raf_fs_ev_mark exact_head;
    raf_fs_ev_mark receipt;
} raf_fs_gate_input;

#define RAF_FS_EV_MARK_VALID(m) ( \
    (m) == RAF_FS_EV_TOKEN_VAZIO || (m) == RAF_FS_EV_OBSERVED || \
    (m) == RAF_FS_EV_REJECTED || (m) == RAF_FS_EV_NOT_APPLICABLE)

static inline __attribute__((always_inline)) raf_fs_gate_state
raf_fs_gate_evaluate(const raf_fs_gate_input *input)
{
    if (input == (const raf_fs_gate_input *)0) {
        return RAF_FS_GATE_BAD_INPUT;
    }
    if (!RAF_FS_EV_MARK_VALID(input->source) ||
        !RAF_FS_EV_MARK_VALID(input->rights) ||
        !RAF_FS_EV_MARK_VALID(input->authority) ||
        !RAF_FS_EV_MARK_VALID(input->target) ||
        !RAF_FS_EV_MARK_VALID(input->evidence_rule) ||
        !RAF_FS_EV_MARK_VALID(input->execution) ||
        !RAF_FS_EV_MARK_VALID(input->artifact_identity) ||
        !RAF_FS_EV_MARK_VALID(input->exact_head) ||
        !RAF_FS_EV_MARK_VALID(input->receipt) ||
        (input->outcome != RAF_FS_OUTCOME_TOKEN_VAZIO &&
         input->outcome != RAF_FS_OUTCOME_NOT_RUN &&
         input->outcome != RAF_FS_OUTCOME_PASS &&
         input->outcome != RAF_FS_OUTCOME_FAIL)) {
        return RAF_FS_GATE_BAD_INPUT;
    }
    if (input->source != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_SOURCE_BLOCKED;
    if (input->rights != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_RIGHTS_BLOCKED;
    if (input->authority != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_AUTHORITY_BLOCKED;
    if (input->target != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_TARGET_BLOCKED;
    if (input->evidence_rule != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_RULE_BLOCKED;
    if (input->execution != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_NOT_EXECUTED;
    if (input->outcome == RAF_FS_OUTCOME_FAIL) return RAF_FS_GATE_EXECUTION_FAILED;
    if (input->outcome != RAF_FS_OUTCOME_PASS) return RAF_FS_GATE_OUTCOME_UNKNOWN;
    if (input->artifact_identity != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_ARTIFACT_BLOCKED;
    if (input->exact_head != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_HEAD_BLOCKED;
    if (input->receipt != RAF_FS_EV_OBSERVED) return RAF_FS_GATE_RECEIPT_BLOCKED;
    return RAF_FS_GATE_PASS_SCOPED;
}

#endif
