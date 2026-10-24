/* Hosted test harness only; NEVER part of L0 artifact. */
#include <stdio.h>
#include <string.h>
#include "../raf_fs_probe.h"
static int errors;
static void check(const unsigned char *p, unsigned n, unsigned format, unsigned state) {
    unsigned matrix[8];
    for (unsigned i=0;i<8;i++) matrix[i]=0xcafef00du;
    raf_fs_probe(p,n,matrix);
    if(matrix[0]!=state || matrix[1]!=format || matrix[5]!=0 || matrix[6]!=0 || matrix[7]!=1) {
        fprintf(stderr,"FAIL format=%u state=%u got %u %u\n",format,state,matrix[1],matrix[0]);errors++;
    }
}
int main(void) {
    static const unsigned char headers[][8]={
      {0x7f,'E','L','F',2,1,1,0}, {'d','e','x','\n','0','3','9',0},
      {'c','d','e','x','0','0','1',0}, {'o','a','t','\n','0','0','0',0},
      {'v','d','e','x','0','2','7',0}, {'a','r','t','\n','0','0','0',0},
      {'P','K',3,4,0,0,0,0}, {'!','<','a','r','c','h','>','\n'},
      {0,'a','s','m',1,0,0,0}, {0x89,'P','N','G',13,10,26,10},
      {0x1f,0x8b,0x08,0,0,0,0,0}, {'M','Z',0,0,0,0,0,0},
      {0xce,0xfa,0xed,0xfe,0,0,0,0}
    };
    for(unsigned i=0;i<13;i++) check(headers[i],8,i+1,RAF_ST_HEADER_CANDIDATE);
    check(headers[1],4,0,RAF_ST_NEED_BYTES);
    check(headers[7],7,0,RAF_ST_NEED_BYTES);
    check(headers[0],3,0,RAF_ST_NEED_BYTES);
    check((const unsigned char *)"TEST",4,0,RAF_ST_UNRECOGNIZED);
    check(NULL,32,0,RAF_ST_INVALID_POINTER);
    unsigned m[8];raf_fs_probe(headers[0],8,NULL);
    raf_fs_probe(headers[0],8,m);
    if(m[2]!=8 || m[4]!=1 || m[5]!=0)errors++;
    puts(errors?"FAIL hosted fixture probes":"PASS hosted fixture probes (13 prefixes + 5 guards)");
    return errors?1:0;
}
