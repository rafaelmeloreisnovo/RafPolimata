#include "rafdb_sql.h"
static int word(const char *s, unsigned n, unsigned *p, const char *lit) {
    unsigned i=0,at=*p;
    while (lit[i]) {
        if (at>=n || s[at]!=lit[i]) return 0;
        ++at; ++i;
    }
    *p=at; return 1;
}
static int field(const char *s, unsigned n, unsigned *p,
                 const rf_u8 **v, unsigned *len, unsigned max) {
    unsigned first;
    if (*p>=n || s[*p]!='\'') return 0;
    ++*p; first=*p;
    while (*p<n && s[*p]!='\'') {
        unsigned char c=(unsigned char)s[*p];
        if (c<32u || c>126u || c=='\\') return 0;
        ++*p;
    }
    if (*p>=n || *p==first || *p-first>max) return 0;
    *v=(const rf_u8*)(s+first); *len=*p-first;
    ++*p; return 1;
}
int rf_sql(struct rf_db *d, const char *s, unsigned n, rf_u8 *out,
           unsigned *outlen) {
    const rf_u8 *k=0,*v=0;
    unsigned p=0,kn=0,vn=0,mode=0;
    if (!d || !s || n>512u || n==0) return RF_INVALID;
    if (word(s,n,&p,"INSERT INTO rafdb VALUES (")) {
        mode=1;
        if (!field(s,n,&p,&k,&kn,RF_MAX_KEY) ||
            !word(s,n,&p,",") ||
            !field(s,n,&p,&v,&vn,RF_MAX_VALUE) ||
            !word(s,n,&p,");")) return RF_INVALID;
    } else if (word(s,n,&p,"UPDATE rafdb SET value=")) {
        mode=2;
        if (!field(s,n,&p,&v,&vn,RF_MAX_VALUE) ||
            !word(s,n,&p," WHERE key=") ||
            !field(s,n,&p,&k,&kn,RF_MAX_KEY) ||
            !word(s,n,&p,";")) return RF_INVALID;
    } else if (word(s,n,&p,"SELECT value FROM rafdb WHERE key=")) {
        mode=3;
        if (!field(s,n,&p,&k,&kn,RF_MAX_KEY) ||
            !word(s,n,&p,";")) return RF_INVALID;
    } else if (word(s,n,&p,"DELETE FROM rafdb WHERE key=")) {
        mode=4;
        if (!field(s,n,&p,&k,&kn,RF_MAX_KEY) ||
            !word(s,n,&p,";")) return RF_INVALID;
    } else return RF_INVALID;
    if (p!=n) return RF_INVALID;
    if (mode==1) return rf_put(d,k,kn,v,vn);
    if (mode==2) return rf_update(d,k,kn,v,vn);
    if (mode==3) return rf_get(d,k,kn,out,outlen);
    return rf_delete(d,k,kn);
}
