// SPDX-License-Identifier: GPL-2.0-or-later
#include "raf_b7_orchestrator.h"
#include <stdio.h>
#define N 8192u
#define R (256u*1024u)
static unsigned char arena[R+64u],input[N],output[N];
typedef struct Io{uint32_t wrote;}Io;
static int64_t rd(void*ctx,uint64_t off,void*dst,uint32_t cap){uint32_t i;(void)ctx;if(off>=N)return 0;if(cap>N-(uint32_t)off)cap=N-(uint32_t)off;for(i=0;i<cap;++i)((uint8_t*)dst)[i]=input[(uint32_t)off+i];return cap;}
static int64_t wr(void*ctx,uint64_t off,const void*src,uint32_t n){Io*io=ctx;uint32_t i;if(off+n>N)return-1;for(i=0;i<n;++i)output[(uint32_t)off+i]=((const uint8_t*)src)[i];io->wrote+=n;return n;}
int main(void){RafB7Plan p;RafB7DiskOps d;Io io={0};uint32_t i,a,b;int guard=0;for(i=0;i<N;++i)input[i]=(uint8_t)((i*29u+7u)&255u);d.ctx=&io;d.read_at=rd;d.write_at=wr;if(raf_b7_crc32c(0,"123456789",9)!=0xE3069283u)return 1;if(raf_b7_init(&p,arena,sizeof(arena),4096,RAF_B7_FLAG_REQUIRE_CRC|RAF_B7_FLAG_MATRIX16,&d,0))return 2;while(!raf_b7_pipeline_done(&p)&&guard++<16)if(raf_b7_pipeline_step(&p))return 3;if(io.wrote!=N)return 4;raf_b7_matrix16_mix(arena,input,N);a=raf_b7_crc32c(0,arena,N);b=raf_b7_crc32c(0,output,N);if(a!=b||p.claim_allowed)return 5;if(raf_b7_attest(&p,b,1)||!p.claim_allowed)return 6;printf("PASS bytes=%u crc32c=%08x caps=%08x receipts=%u\n",io.wrote,b,p.capabilities,p.receipt_count);return 0;}
