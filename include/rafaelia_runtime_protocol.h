#ifndef RAFAELIA_RUNTIME_PROTOCOL_H
#define RAFAELIA_RUNTIME_PROTOCOL_H

typedef unsigned char      rrp_u8;
typedef unsigned short     rrp_u16;
typedef unsigned int       rrp_u32;
typedef unsigned long long rrp_u64;
typedef signed int         rrp_s32;

#define RRP_SCHEMA_VERSION 0x00010000u
#define RRP_MAX_NAME       192u
#define RRP_MAX_LOCATOR    512u
#define RRP_MAX_HASH       128u
#define RRP_MAX_OUTPUTS    16u

#define RRP_OK              0
#define RRP_E_NULL         -1
#define RRP_E_RANGE        -2
#define RRP_E_VERSION      -3
#define RRP_E_POLICY       -4
#define RRP_E_INVARIANT    -5

typedef enum rrp_provider {
    RRP_PROVIDER_UNKNOWN = 0,
    RRP_PROVIDER_GOOGLE_DRIVE = 1,
    RRP_PROVIDER_FILESYSTEM = 2,
    RRP_PROVIDER_GITHUB = 3,
    RRP_PROVIDER_ZIP_ENTRY = 4
} rrp_provider;

typedef enum rrp_operation {
    RRP_OP_UNKNOWN = 0,
    RRP_OP_INVENTORY_SOURCE = 1,
    RRP_OP_INDEX_CONVERSATIONS = 2,
    RRP_OP_INDEX_TEXT_LOGS = 3,
    RRP_OP_INSPECT_SQLITE = 4,
    RRP_OP_INDEX_MATRIX_JSON = 5,
    RRP_OP_BUILD_TIMELINE = 6,
    RRP_OP_SEMANTIC_ANCHOR = 7
} rrp_operation;

typedef enum rrp_state {
    RRP_STATE_VERIFIED = 0,
    RRP_STATE_DECLARED_BY_AUTHOR = 1,
    RRP_STATE_TOKEN_VAZIO = 2,
    RRP_STATE_CONTRADICTION = 3
} rrp_state;

typedef enum rrp_output_kind {
    RRP_OUT_SOURCE_MANIFEST = 1,
    RRP_OUT_OBJECTS_SEGMENT = 2,
    RRP_OUT_CONVERSATIONS_SEGMENT = 3,
    RRP_OUT_MESSAGES_SEGMENT = 4,
    RRP_OUT_RELATIONS_SEGMENT = 5,
    RRP_OUT_TIMELINE_SEGMENT = 6,
    RRP_OUT_AUDIT_JSONL = 7,
    RRP_OUT_COVERAGE_REPORT = 8,
    RRP_OUT_CHECKPOINT_STATE = 9
} rrp_output_kind;

typedef struct rrp_source {
    rrp_u32 provider;
    rrp_u64 size_bytes;
    rrp_u8 locator[RRP_MAX_LOCATOR];
    rrp_u8 name[RRP_MAX_NAME];
    rrp_u8 content_hash[RRP_MAX_HASH];
} rrp_source;

typedef struct rrp_policy {
    rrp_u64 max_memory_bytes;
    rrp_u64 max_expanded_bytes;
    rrp_u32 read_only;
    rrp_u32 exclude_private_media;
    rrp_u32 allow_network;
    rrp_u32 allow_model_inference;
} rrp_policy;

typedef struct rrp_job {
    rrp_u32 schema_version;
    rrp_u32 operation;
    rrp_u8 job_id[RRP_MAX_HASH];
    rrp_source source;
    rrp_policy policy;
    rrp_u32 outputs[RRP_MAX_OUTPUTS];
    rrp_u32 output_count;
    rrp_u32 reserved[8];
} rrp_job;

typedef struct rrp_event {
    rrp_u8 job_id[RRP_MAX_HASH];
    rrp_u32 stage;
    rrp_u32 state;
    rrp_u64 bytes_read;
    rrp_u64 records_processed;
    rrp_u64 checkpoint;
    rrp_s32 error_code;
} rrp_event;

static int rrp_validate_job(const rrp_job *job) {
    if (!job) return RRP_E_NULL;
    if (job->schema_version != RRP_SCHEMA_VERSION) return RRP_E_VERSION;
    if (job->policy.read_only != 1u) return RRP_E_POLICY;
    if (job->output_count == 0u || job->output_count > RRP_MAX_OUTPUTS) return RRP_E_RANGE;
    if (job->source.provider == RRP_PROVIDER_UNKNOWN) return RRP_E_INVARIANT;
    if (job->operation == RRP_OP_UNKNOWN) return RRP_E_INVARIANT;
    return RRP_OK;
}

#endif
