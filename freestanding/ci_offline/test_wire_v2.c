/* Producer selftest without consumer project: all data caller-owned. */
#include "raf_ci_wire_v2.h"
#define CHECK(x) if(!(x)) return __LINE__
static unsigned char wire[40];
static int test(void){
 const unsigned char allow[]="on: push\npermissions: read-all\n- uses: owner/action@0123456789abcdef0123456789abcdef01234567\n";
 const unsigned char risk[]="on: [pull_request_target]\npermissions: write-all\n  - uses: actions/checkout@v5\n";
 const unsigned char absent[]="";
 raf_ci_wire_v2_encode(allow,sizeof(allow)-1u,wire);
 CHECK(wire[0]=='R'&&wire[3]=='2');
 CHECK(raf_ci_crc32c(wire,36u)==((unsigned int)wire[36]|((unsigned int)wire[37]<<8u)|((unsigned int)wire[38]<<16u)|((unsigned int)wire[39]<<24u)));
 CHECK(wire[24]==1u && wire[28]==15u);
 CHECK((wire[8]&2u)==0u);
 raf_ci_wire_v2_encode(risk,sizeof(risk)-1u,wire);
 CHECK(wire[24]==2u && (wire[8]&2u));
 CHECK((wire[8] & 2u)!=0u && (wire[9] & 12u)==12u); /* bits 10/11 */
 raf_ci_wire_v2_encode(absent,0u,wire);
 CHECK(wire[24]==2u && wire[28]==15u && (wire[8]&128u));
 CHECK(raf_ci_crc32c((const unsigned char*)"123456789",9u)==0xe3069283u);
 return 0;
}
int main(void){return test();}
