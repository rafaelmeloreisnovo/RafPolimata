#ifndef RAFCI_WIRE_V1_H
#define RAFCI_WIRE_V1_H

/*
RAFCI-FILE-CONTRACT
PURPOSE=Fixed binary vocabulary for RafCI jobs, receipts, capability bits and gate bits.
SCOPE=Freestanding-compatible metadata only; no execution engine, libc, heap, syscall or hosted runtime.
PRECONDITIONS=8-bit bytes and C11 compiler for layout assertions.
REGISTER_OWNERSHIP=NONE
CLOBBERS=NONE
MEMORY_ORDER=NONE
TAIL_SHADOW=NONE
EVIDENCE=Source/layout existence only until an executed compile/link gate produces a receipt.
*/

/*
RAFCI-BIT
ID=rafci.bit.contract.wire.v1
KIND=contract
ROUTE=stage.source>stage.artifact>stage.execution>stage.evidence
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Defines stable wire names and layouts; does not prove execution.
*/

#if !defined(__CHAR_BIT__) || (__CHAR_BIT__ != 8)
#error "RafCI wire v1 requires 8-bit bytes"
#endif

typedef unsigned char rafci_u8;

/* Capability bit positions: semantic owner is rafci/graph.v1.json. */
#define RAFCI_CAP_AUTHORITY_BIT 0u
#define RAFCI_CAP_OPERATOR_BIT 1u
#define RAFCI_CAP_ACTION_PROVIDER_BIT 2u
#define RAFCI_CAP_HOST_PROVIDER_BIT 3u
#define RAFCI_CAP_TOOLCHAIN_PROVIDER_BIT 4u
#define RAFCI_CAP_FREESTANDING_TARGET_BIT 5u
#define RAFCI_CAP_PHYSICAL_TARGET_BIT 6u
#define RAFCI_CAP_RECEIPT_SINK_BIT 7u

/* Gate bit positions: PASS/FAIL/TOKEN_VAZIO are carried in separate masks. */
#define RAFCI_GATE_SOURCE_IDENTITY_BIT 0u
#define RAFCI_GATE_ARTIFACT_IDENTITY_BIT 1u
#define RAFCI_GATE_NO_NEEDED_BIT 2u
#define RAFCI_GATE_NO_INTERP_BIT 3u
#define RAFCI_GATE_NO_UNDEFINED_BIT 4u
#define RAFCI_GATE_LINKER_ANCHORS_BIT 5u
#define RAFCI_GATE_HOSTED_EXECUTION_BIT 6u
#define RAFCI_GATE_PHYSICAL_EXECUTION_BIT 7u
#define RAFCI_GATE_PROVIDER_ENFORCEMENT_BIT 8u
#define RAFCI_GATE_PROVENANCE_BIT 9u

#define RAFCI_BIT_MASK(bit_) (1ULL << (bit_))

#define RAFCI_STATUS_IMPLEMENTED 1u
#define RAFCI_STATUS_PASS 2u
#define RAFCI_STATUS_FAIL 3u
#define RAFCI_STATUS_TOKEN_VAZIO 4u

/*
RAFCI-BIT
ID=rafci.bit.contract.job.v1
KIND=contract
ROUTE=stage.source>stage.artifact
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Carries source identity, target and required gates; it is not an execution receipt.
*/
typedef struct rafci_job_v1 {
    rafci_u8 magic[4];               /* "RFCI" */
    rafci_u8 version_le[2];          /* 1 */
    rafci_u8 record_bytes_le[2];     /* 88 */
    rafci_u8 job_id[16];             /* opaque stable job identity */
    rafci_u8 source_sha256[32];      /* exact source/input identity */
    rafci_u8 target_id[16];          /* bounded target/profile identifier */
    rafci_u8 capability_mask_le[8];  /* requested/declared capabilities */
    rafci_u8 required_gate_mask_le[8];
} rafci_job_v1;

/*
RAFCI-BIT
ID=rafci.bit.contract.receipt.v1
KIND=contract
ROUTE=stage.execution>stage.evidence>stage.claim
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Separates gate PASS/FAIL/TOKEN_VAZIO; claim promotion remains external and scoped.
*/
typedef struct rafci_receipt_v1 {
    rafci_u8 magic[4];                /* "RFCR" */
    rafci_u8 version_le[2];           /* 1 */
    rafci_u8 record_bytes_le[2];      /* 152 */
    rafci_u8 job_id[16];
    rafci_u8 source_sha256[32];
    rafci_u8 artifact_sha256[32];
    rafci_u8 evidence_sha256[32];
    rafci_u8 gate_pass_mask_le[8];
    rafci_u8 gate_fail_mask_le[8];
    rafci_u8 gate_token_vazio_mask_le[8];
    rafci_u8 status;                  /* RAFCI_STATUS_* */
    rafci_u8 reserved[7];             /* MUST be zero in v1 */
} rafci_receipt_v1;

_Static_assert(sizeof(rafci_job_v1) == 88u, "rafci_job_v1 layout drift");
_Static_assert(sizeof(rafci_receipt_v1) == 152u, "rafci_receipt_v1 layout drift");

#endif /* RAFCI_WIRE_V1_H */
