#ifndef RAFDB_H
#define RAFDB_H
/*
 * RAFDB L0 volatile table and bounded record emitter.
 * Scope: C11, caller-owned RAM, no libc/heap/syscalls. No SQL wire, no durable
 * ZIPRAF custody or Oracle/MySQL equivalence is claimed.
 * License/provenance: repository rights policy; do not relicense third parties.
 */
typedef unsigned char rf_u8;
typedef unsigned int rf_u32;
typedef unsigned long long rf_u64;

#define RF_MAX_ROWS 64u
#define RF_MAX_KEY 31u
#define RF_MAX_VALUE 127u
#define RF_RECORD_MAX 177u

enum { RF_OK=0, RF_INVALID=1, RF_NOT_FOUND=2, RF_FULL=3,
       RF_VETO=4, RF_EMIT_FAILED=5, RF_REENTRANT=6 };
enum { RF_INSERT=1, RF_UPDATE=2, RF_DELETE=3 };
enum { RF_BEFORE=1, RF_AFTER=2 };

typedef int (*rf_trigger)(void *context, unsigned phase, unsigned operation,
                          const rf_u8 *key, unsigned key_len,
                          const rf_u8 *value, unsigned value_len, rf_u64 next_gen);
typedef int (*rf_append)(void *context, const rf_u8 *record, unsigned count);

struct rf_row {
    rf_u8 used, key_len, value_len;
    rf_u8 key[RF_MAX_KEY];
    rf_u8 value[RF_MAX_VALUE];
    rf_u64 generation;
};
struct rf_db {
    struct rf_row rows[RF_MAX_ROWS];
    rf_u64 generation;
    unsigned busy;
    rf_trigger trigger;
    void *trigger_context;
    rf_append append;
    void *append_context;
};

void rf_init(struct rf_db *db);
void rf_hooks(struct rf_db *db, rf_trigger trigger, void *trigger_context,
              rf_append append, void *append_context);
int rf_put(struct rf_db *db, const rf_u8 *key, unsigned key_len,
           const rf_u8 *value, unsigned value_len);
int rf_update(struct rf_db *db, const rf_u8 *key, unsigned key_len,
              const rf_u8 *value, unsigned value_len);
int rf_delete(struct rf_db *db, const rf_u8 *key, unsigned key_len);
int rf_get(const struct rf_db *db, const rf_u8 *key, unsigned key_len,
           rf_u8 *out, unsigned *inout_len);
/* Callback receives a versioned deterministic record prior to RAM mutation.
 * A callback returning zero MUST itself establish any required durability.
 * No callback => volatile-only mode. FNV tag detects accidental corruption,
 * NOT adversarial tampering; ZIPRAF cryptographic receipts remain external.
 */
#endif
