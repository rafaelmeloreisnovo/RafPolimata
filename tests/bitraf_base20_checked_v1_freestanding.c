/* Freestanding object probe. No physical ARM execution claim. */
#include "../rafaelia/bitraf_base20_checked_v1.h"
bitraf_u32 raf_b20c_probe(bitraf_u32 index){
    char chars[4];bitraf_u16 back;
    if(raf_b20c_encode(index,chars)!=RAF_B20C_OK)return 1u;
    if(raf_b20c_decode(chars,3u,&back)!=RAF_B20C_OK)return 2u;
    return (bitraf_u32)(back!=index);
}
