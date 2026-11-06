#include "rafdb.h"

static int same(const rf_u8 *a, const rf_u8 *b, unsigned n) {
    unsigned i;
    for (i=0; i<n; ++i) if (a[i]!=b[i]) return 0;
    return 1;
}
static int locate(const struct rf_db *db, const rf_u8 *key, unsigned n) {
    unsigned i;
    for (i=0; i<RF_MAX_ROWS; ++i)
        if (db->rows[i].used && db->rows[i].key_len==n &&
            same(db->rows[i].key,key,n)) return (int)i;
    return -1;
}
void rf_init(struct rf_db *db) {
    unsigned i,j;
    if (!db) return;
    for (i=0; i<RF_MAX_ROWS; ++i) {
        db->rows[i].used=0; db->rows[i].key_len=0;
        db->rows[i].value_len=0; db->rows[i].generation=0;
        for (j=0;j<RF_MAX_KEY;++j) db->rows[i].key[j]=0;
        for (j=0;j<RF_MAX_VALUE;++j) db->rows[i].value[j]=0;
    }
    db->generation=0; db->busy=0; db->trigger=0; db->append=0;
    db->trigger_context=0; db->append_context=0;
}
void rf_hooks(struct rf_db *db, rf_trigger t, void *tc, rf_append a, void *ac) {
    if (!db || db->busy) return;
    db->trigger=t; db->trigger_context=tc;
    db->append=a; db->append_context=ac;
}
static int write_record(struct rf_db *db, unsigned op, const rf_u8 *k,
                        unsigned kn, const rf_u8 *v, unsigned vn, rf_u64 gen) {
    rf_u8 b[RF_RECORD_MAX];
    rf_u32 h=2166136261u;
    unsigned i,n=0;
    if (!db->append) return RF_OK;
    b[n++]='R'; b[n++]='D'; b[n++]='B'; b[n++]='1';
    b[n++]=(rf_u8)op; b[n++]=(rf_u8)kn; b[n++]=(rf_u8)vn;
    for (i=0;i<8;++i) b[n++]=(rf_u8)(gen>>(8u*i));
    for (i=0;i<kn;++i) b[n++]=k[i];
    for (i=0;i<vn;++i) b[n++]=v[i];
    for (i=0;i<n;++i) h=(h^(rf_u32)b[i])*16777619u;
    for (i=0;i<4;++i) b[n++]=(rf_u8)(h>>(8u*i));
    return db->append(db->append_context,b,n)==0 ? RF_OK : RF_EMIT_FAILED;
}
static int change(struct rf_db *db, unsigned mode, const rf_u8 *k,
                  unsigned kn, const rf_u8 *v, unsigned vn) {
    int pos,rc;
    unsigned i,op;
    struct rf_row *r;
    if (!db || !k || kn==0 || kn>RF_MAX_KEY || vn>RF_MAX_VALUE ||
        (mode!=RF_DELETE && (!v || vn==0)) || mode<RF_INSERT || mode>RF_DELETE)
        return RF_INVALID;
    if (db->busy) return RF_REENTRANT;
    if (db->generation==(rf_u64)(~(rf_u64)0)) return RF_FULL;
    pos=locate(db,k,kn);
    if (mode==RF_UPDATE && pos<0) return RF_NOT_FOUND;
    if (mode==RF_DELETE && pos<0) return RF_NOT_FOUND;
    if (mode==RF_INSERT && pos<0) {
        for (i=0;i<RF_MAX_ROWS;++i)
            if (!db->rows[i].used) { pos=(int)i; break; }
        if (pos<0) return RF_FULL;
    }
    op=(mode==RF_DELETE) ? RF_DELETE :
       (mode==RF_UPDATE || db->rows[pos].used) ? RF_UPDATE : RF_INSERT;
    db->busy=1;
    if (db->trigger && db->trigger(db->trigger_context, RF_BEFORE, op,
                                   k,kn,v,vn,db->generation+1)!=0) {
        db->busy=0; return RF_VETO;
    }
    rc=write_record(db,op,k,kn,v,vn,db->generation+1);
    if (rc!=RF_OK) { db->busy=0; return rc; }
    r=&db->rows[pos];
    if (mode==RF_DELETE) {
        r->used=0; r->key_len=0; r->value_len=0;
        for (i=0;i<RF_MAX_KEY;++i) r->key[i]=0;
        for (i=0;i<RF_MAX_VALUE;++i) r->value[i]=0;
    } else {
        r->used=1; r->key_len=(rf_u8)kn; r->value_len=(rf_u8)vn;
        for (i=0;i<kn;++i) r->key[i]=k[i];
        for (i=kn;i<RF_MAX_KEY;++i) r->key[i]=0;
        for (i=0;i<vn;++i) r->value[i]=v[i];
        for (i=vn;i<RF_MAX_VALUE;++i) r->value[i]=0;
    }
    db->generation++; r->generation=db->generation;
    /* AFTER is notification only; its result cannot undo the mutation. */
    if (db->trigger) (void)db->trigger(db->trigger_context,RF_AFTER,op,
                                     k,kn,v,vn,db->generation);
    db->busy=0;
    return RF_OK;
}
int rf_put(struct rf_db *d, const rf_u8 *k, unsigned kn,
           const rf_u8 *v, unsigned vn) { return change(d,RF_INSERT,k,kn,v,vn); }
int rf_update(struct rf_db *d, const rf_u8 *k, unsigned kn,
              const rf_u8 *v, unsigned vn) { return change(d,RF_UPDATE,k,kn,v,vn); }
int rf_delete(struct rf_db *d, const rf_u8 *k, unsigned kn) {
    return change(d,RF_DELETE,k,kn,0,0);
}
int rf_get(const struct rf_db *db, const rf_u8 *k, unsigned kn,
           rf_u8 *out, unsigned *len) {
    int i;
    unsigned j,need;
    if (!db || !k || !len || !kn || kn>RF_MAX_KEY) return RF_INVALID;
    i=locate(db,k,kn);
    if (i<0) return RF_NOT_FOUND;
    need=db->rows[i].value_len;
    if (*len<need || !out) { *len=need; return RF_FULL; }
    for (j=0;j<need;++j) out[j]=db->rows[i].value[j];
    *len=need;
    return RF_OK;
}
