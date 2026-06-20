#ifndef RAFBBS_SHA256_H
#define RAFBBS_SHA256_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint32_t h[8]; uint64_t len; unsigned char buf[64]; uint32_t used; } RafSha256;
#define ROR32(x,n) (((x)>>(n))|((x)<<(32u-(n))))
static const uint32_t raf_k256[64] = {
0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
static void raf_sha256_init(RafSha256 *s){s->h[0]=0x6a09e667u;s->h[1]=0xbb67ae85u;s->h[2]=0x3c6ef372u;s->h[3]=0xa54ff53au;s->h[4]=0x510e527fu;s->h[5]=0x9b05688cu;s->h[6]=0x1f83d9abu;s->h[7]=0x5be0cd19u;s->len=0;s->used=0;}
static void raf_sha256_block(RafSha256 *s,const unsigned char *p){uint32_t w[64],a,b,c,d,e,f,g,h,t1,t2;int i;for(i=0;i<16;i++)w[i]=((uint32_t)p[i*4]<<24)|((uint32_t)p[i*4+1]<<16)|((uint32_t)p[i*4+2]<<8)|p[i*4+3];for(i=16;i<64;i++){uint32_t x=w[i-15],y=w[i-2];w[i]=w[i-16]+(ROR32(x,7)^ROR32(x,18)^(x>>3))+w[i-7]+(ROR32(y,17)^ROR32(y,19)^(y>>10));}a=s->h[0];b=s->h[1];c=s->h[2];d=s->h[3];e=s->h[4];f=s->h[5];g=s->h[6];h=s->h[7];for(i=0;i<64;i++){t1=h+(ROR32(e,6)^ROR32(e,11)^ROR32(e,25))+((e&f)^((~e)&g))+raf_k256[i]+w[i];t2=(ROR32(a,2)^ROR32(a,13)^ROR32(a,22))+((a&b)^(a&c)^(b&c));h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}s->h[0]+=a;s->h[1]+=b;s->h[2]+=c;s->h[3]+=d;s->h[4]+=e;s->h[5]+=f;s->h[6]+=g;s->h[7]+=h;}
static void raf_sha256_update(RafSha256 *s,const unsigned char *p,uint32_t n){uint32_t i;for(i=0;i<n;i++){s->buf[s->used++]=p[i];s->len+=8u;if(s->used==64u){raf_sha256_block(s,s->buf);s->used=0;}}}
static void raf_sha256_final(RafSha256 *s,unsigned char out[32]){uint64_t bits=s->len;uint32_t i;s->buf[s->used++]=0x80u;if(s->used>56u){while(s->used<64u)s->buf[s->used++]=0;raf_sha256_block(s,s->buf);s->used=0;}while(s->used<56u)s->buf[s->used++]=0;for(i=0;i<8;i++)s->buf[63u-i]=(unsigned char)(bits>>(i*8u));raf_sha256_block(s,s->buf);for(i=0;i<8;i++){out[i*4]=(unsigned char)(s->h[i]>>24);out[i*4+1]=(unsigned char)(s->h[i]>>16);out[i*4+2]=(unsigned char)(s->h[i]>>8);out[i*4+3]=(unsigned char)s->h[i];}}
static void raf_sha256_hex(const unsigned char in[32], char out[65]){static const char hex[]="0123456789abcdef";int i;for(i=0;i<32;i++){out[i*2]=hex[in[i]>>4];out[i*2+1]=hex[in[i]&15];}out[64]=0;}
static inline int raf_sha256_file(const char *path,char out[65]){unsigned char b[4096],d[32];RafSha256 s;FILE *f=fopen(path,"rb");if(!f)return-1;raf_sha256_init(&s);for(;;){size_t n=fread(b,1,sizeof(b),f);if(n)raf_sha256_update(&s,b,(uint32_t)n);if(n<sizeof(b))break;}if(ferror(f)){fclose(f);return-1;}fclose(f);raf_sha256_final(&s,d);raf_sha256_hex(d,out);return 0;}
#endif
