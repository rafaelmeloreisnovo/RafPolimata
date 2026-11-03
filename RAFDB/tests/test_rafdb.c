#include "../banco/rafdb.h"
#include "../gateway/rafdb_sql.h"
#include <stdio.h>
static struct rf_db d;
static int pre=0,post=0,fail_emit=0,veto=0,record_n=0;
static int hook(void *ctx,unsigned phase,unsigned op,const rf_u8 *k,
                unsigned kl,const rf_u8 *v,unsigned vl,rf_u64 next) {
    (void)ctx;(void)op;(void)k;(void)kl;(void)v;(void)vl;(void)next;
    if (phase==RF_BEFORE) { ++pre; return veto; }
    ++post; return 0;
}
static int emit(void *ctx,const rf_u8 *r,unsigned n) {
    (void)ctx;
    if (n<19 || r[0]!='R' || r[1]!='D' || r[2]!='B' || r[3]!='1')
        return 1;
    ++record_n; return fail_emit;
}
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d\n",__LINE__);return 1; } } while (0)
int main(void) {
    rf_u8 out[128];
    unsigned n=sizeof out;
    rf_u64 g;
    const rf_u8 key[]="a",val[]="first",val2[]="next";
    rf_init(&d);
    rf_hooks(&d,hook,0,emit,0);
    CHECK(rf_put(&d,key,1,val,5)==RF_OK);
    CHECK(d.generation==1 && pre==1 && post==1 && record_n==1);
    CHECK(rf_get(&d,key,1,out,&n)==RF_OK && n==5 && out[0]=='f');
    n=2; CHECK(rf_get(&d,key,1,out,&n)==RF_FULL && n==5);
    veto=1;g=d.generation;
    CHECK(rf_update(&d,key,1,val2,4)==RF_VETO && d.generation==g);
    veto=0;fail_emit=1;
    CHECK(rf_delete(&d,key,1)==RF_EMIT_FAILED && d.generation==g);
    n=sizeof out;CHECK(rf_get(&d,key,1,out,&n)==RF_OK && n==5);
    fail_emit=0;
    CHECK(rf_update(&d,key,1,val2,4)==RF_OK);
    n=sizeof out;CHECK(rf_get(&d,key,1,out,&n)==RF_OK && n==4 && out[0]=='n');
    CHECK(rf_delete(&d,key,1)==RF_OK);
    CHECK(rf_get(&d,key,1,out,&n)==RF_NOT_FOUND);
    CHECK(rf_delete(&d,key,1)==RF_NOT_FOUND);
    rf_hooks(&d,0,0,0,0);
    CHECK(rf_sql(&d,"INSERT INTO rafdb VALUES ('foo','bar');",39,out,&n)==RF_OK);
    n=sizeof out;
    CHECK(rf_sql(&d,"SELECT value FROM rafdb WHERE key='foo';",40,out,&n)==RF_OK &&
          n==3 && out[2]=='r');
    CHECK(rf_sql(&d,"UPDATE rafdb SET value='baz' WHERE key='foo';",45,out,&n)==RF_OK);
    n=sizeof out;
    CHECK(rf_sql(&d,"SELECT value FROM rafdb WHERE key='foo';",40,out,&n)==RF_OK &&
          n==3 && out[2]=='z');
    CHECK(rf_sql(&d,"DELETE FROM rafdb WHERE key='foo';",33,out,&n)==RF_OK);
    CHECK(rf_sql(&d,"INSERT INTO rafdb VALUES ('foo','bar');DROP",43,out,&n)==RF_INVALID);
    printf("RAFDB bounded core PASS; volatile+callback records; no persistence/Oracle/MySQL wire claim\n");
    return 0;
}
