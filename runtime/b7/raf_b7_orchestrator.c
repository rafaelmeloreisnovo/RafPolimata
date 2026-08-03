// SPDX-License-Identifier: GPL-2.0-or-later
#include "raf_b7_orchestrator.h"
#if defined(__ARM_NEON)||defined(__ARM_NEON__)
#include <arm_neon.h>
#define RAF_B7_HAVE_NEON 1
#else
#define RAF_B7_HAVE_NEON 0
#endif
#if defined(__ARM_FEATURE_CRC32)
#include <arm_acle.h>
#define RAF_B7_HAVE_ARM_CRC 1
#else
#define RAF_B7_HAVE_ARM_CRC 0
#endif
#define PHI32 0x9E3779B9u
#define CRC_POLY 0x82F63B78u
static const uint32_t salt[16]={0x243F6A88u,0x85A308D3u,0x13198A2Eu,0x03707344u,0xA4093822u,0x299F31D0u,0x082EFA98u,0xEC4E6C89u,0x452821E6u,0x38D01377u,0xBE5466CFu,0x34E90C6Cu,0xC0AC29B7u,0xC97C50DDu,0x3F84D5B5u,0xB5470917u};
static uintptr_t au(uintptr_t v,uintptr_t a){return(v+a-1u)&~(a-1u);} static uint32_t ad(uint32_t v,uint32_t a){return v&~(a-1u);} static uint32_t rol(uint32_t x,uint32_t n){return(x<<n)|(x>>(32u-n));}
static uint32_t ld32(const uint8_t*p){return(uint32_t)p[0]|((uint32_t)p[1]<<8u)|((uint32_t)p[2]<<16u)|((uint32_t)p[3]<<24u);}
#if RAF_B7_HAVE_ARM_CRC&&defined(__aarch64__)
static uint64_t ld64(const uint8_t*p){return(uint64_t)ld32(p)|((uint64_t)ld32(p+4u)<<32u);}
#endif
uint32_t raf_b7_compile_capabilities(void){uint32_t c=RAF_B7_CAP_SCALAR;
#if RAF_B7_HAVE_NEON
c|=RAF_B7_CAP_NEON;
#endif
#if RAF_B7_HAVE_ARM_CRC
c|=RAF_B7_CAP_CRC32C_HW;
#endif
return c;}
uint32_t raf_b7_crc32c(uint32_t seed,const void*d,uint32_t n){const uint8_t*p=(const uint8_t*)d;uint32_t c=~seed,i=0;if(!p||!n)return~c;
#if RAF_B7_HAVE_ARM_CRC&&defined(__aarch64__)
for(;i+8u<=n;i+=8u)c=__crc32cd(c,ld64(p+i));for(;i+4u<=n;i+=4u)c=__crc32cw(c,ld32(p+i));for(;i<n;++i)c=__crc32cb(c,p[i]);
#elif RAF_B7_HAVE_ARM_CRC
for(;i+4u<=n;i+=4u)c=__crc32cw(c,ld32(p+i));for(;i<n;++i)c=__crc32cb(c,p[i]);
#else
for(;i<n;++i){uint32_t b;c^=p[i];for(b=0;b<8u;++b){uint32_t m=0u-(c&1u);c=(c>>1u)^(CRC_POLY&m);}}
#endif
return~c;}
void raf_b7_partition16(RafB7Lane l[16],uint32_t n){uint32_t q=n/16u,r=n%16u,o=0,i;if(!l)return;for(i=0;i<16u;++i){uint32_t z=q+(i<r);l[i].begin=o;l[i].end=o+z;l[i].crc32c=0;l[i].id=i;o+=z;}}
void raf_b7_matrix16_mix(void*dv,const void*sv,uint32_t n){uint8_t*d=dv;const uint8_t*s=sv;uint32_t w=n/4u,i=0;if(!d||!s)return;
#if RAF_B7_HAVE_NEON
for(;i+16u<=w;i+=16u){uint32x4_t a=vld1q_u32((const uint32_t*)(const void*)(s+(i+0u)*4u)),b=vld1q_u32((const uint32_t*)(const void*)(s+(i+4u)*4u)),c=vld1q_u32((const uint32_t*)(const void*)(s+(i+8u)*4u)),e=vld1q_u32((const uint32_t*)(const void*)(s+(i+12u)*4u));uint32x4_t r0,r1,r2,r3;a=vmulq_n_u32(veorq_u32(a,vld1q_u32(salt)),PHI32);b=vmulq_n_u32(veorq_u32(b,vld1q_u32(salt+4)),PHI32);c=vmulq_n_u32(veorq_u32(c,vld1q_u32(salt+8)),PHI32);e=vmulq_n_u32(veorq_u32(e,vld1q_u32(salt+12)),PHI32);r0=vorrq_u32(vshlq_n_u32(a,7),vshrq_n_u32(a,25));r1=vorrq_u32(vshlq_n_u32(b,7),vshrq_n_u32(b,25));r2=vorrq_u32(vshlq_n_u32(c,7),vshrq_n_u32(c,25));r3=vorrq_u32(vshlq_n_u32(e,7),vshrq_n_u32(e,25));a=veorq_u32(r0,vshrq_n_u32(r0,11));b=veorq_u32(r1,vshrq_n_u32(r1,11));c=veorq_u32(r2,vshrq_n_u32(r2,11));e=veorq_u32(r3,vshrq_n_u32(r3,11));vst1q_u32((uint32_t*)(void*)(d+(i+0u)*4u),a);vst1q_u32((uint32_t*)(void*)(d+(i+4u)*4u),b);vst1q_u32((uint32_t*)(void*)(d+(i+8u)*4u),c);vst1q_u32((uint32_t*)(void*)(d+(i+12u)*4u),e);}
#endif
for(;i<w;++i){uint32_t x=ld32(s+i*4u);x=(x^salt[i&15u])*PHI32;x=rol(x,7u);x^=x>>11u;d[i*4u]=(uint8_t)x;d[i*4u+1u]=(uint8_t)(x>>8u);d[i*4u+2u]=(uint8_t)(x>>16u);d[i*4u+3u]=(uint8_t)(x>>24u);}for(i=w*4u;i<n;++i)d[i]=s[i];}
static void rec(RafB7Plan*p,uint16_t st,uint16_t be,int32_t rc,const RafB7Bank*b,uint64_t off,uint32_t in,uint32_t out){RafB7Receipt*r;if(!p||!b)return;r=&p->receipt[p->receipt_head];r->epoch=p->epoch;r->offset=off;r->address=b->logical_address;r->bytes=b->used;r->input_crc32c=in;r->output_crc32c=out;r->stage=st;r->backend=be;r->status=rc;p->receipt_head=(p->receipt_head+1u)%64u;if(p->receipt_count<64u)++p->receipt_count;}
static uint16_t backend(RafB7Plan*p,uint32_t n){if(p&&(p->flags&RAF_B7_FLAG_ALLOW_GPU)&&n>=p->gpu_threshold&&p->gpu.available&&p->gpu.dispatch){if((p->capabilities&RAF_B7_CAP_GPU_VULKAN)&&p->gpu.available(p->gpu.ctx,RAF_B7_BACKEND_VULKAN)>0)return RAF_B7_BACKEND_VULKAN;if((p->capabilities&RAF_B7_CAP_GPU_OPENCL)&&p->gpu.available(p->gpu.ctx,RAF_B7_BACKEND_OPENCL)>0)return RAF_B7_BACKEND_OPENCL;}return p&&(p->capabilities&RAF_B7_CAP_NEON)?RAF_B7_BACKEND_NEON:RAF_B7_BACKEND_SCALAR;}
static int compute(RafB7Plan*p,RafB7Bank*b){uint16_t be;uint32_t in,out,i;if(!p||!b||b->state!=RAF_B7_BANK_COMPUTE)return RAF_B7_ESTATE;in=raf_b7_crc32c(0,b->base,b->used);raf_b7_partition16(p->lane,b->used);for(i=0;i<16u;++i){uint32_t n=p->lane[i].end-p->lane[i].begin;p->lane[i].crc32c=raf_b7_crc32c(0,b->base+p->lane[i].begin,n);}if(p->cache_bytes>=64u){uint32_t*q=(uint32_t*)(void*)p->cache_base;for(i=0;i<16u;++i)q[i]=p->lane[i].crc32c;}be=backend(p,b->used);if(be>=RAF_B7_BACKEND_VULKAN){int rc=p->gpu.dispatch(p->gpu.ctx,be,b->base,b->base,b->used,16u,p->cache_base,p->cache_bytes);if(!rc&&p->gpu.wait)rc=p->gpu.wait(p->gpu.ctx,be);if(rc){rec(p,RAF_B7_STAGE_COMPUTE,be,RAF_B7_EGPU,b,p->next_write_offset,in,in);return RAF_B7_EGPU;}}else raf_b7_matrix16_mix(b->base,b->base,b->used);out=raf_b7_crc32c(0,b->base,b->used);b->crc32c=out;b->state=RAF_B7_BANK_WRITE;rec(p,RAF_B7_STAGE_COMPUTE,be,0,b,p->next_write_offset,in,out);return 0;}
int raf_b7_init(RafB7Plan*p,void*r,uint32_t n,uint32_t c,uint32_t f,const RafB7DiskOps*d,const RafB7GpuOps*g){uintptr_t raw,a;uint32_t skip,u,z,i;if(!p||!r)return RAF_B7_EINVAL;if(c<256u)c=256u;c=ad(c,64u);raw=(uintptr_t)r;a=au(raw,64u);skip=(uint32_t)(a-raw);if(n<=skip+c+192u)return RAF_B7_ENOSPC;u=n-skip;z=ad((u-c)/3u,64u);if(z<64u)return RAF_B7_ENOSPC;for(i=0;i<sizeof(*p);++i)((uint8_t*)p)[i]=0;p->region_base=(uint8_t*)a;p->region_bytes=z*3u+c;for(i=0;i<3u;++i){p->bank[i].base=p->region_base+i*z;p->bank[i].capacity=z;p->bank[i].logical_address=(uintptr_t)p->bank[i].base;}p->cache_base=p->region_base+3u*z;p->cache_bytes=c;p->flags=f|RAF_B7_FLAG_MATRIX16;p->gpu_threshold=65536u;p->capabilities=raf_b7_compile_capabilities();if(d){p->disk=*d;if(d->read_at||d->write_at)p->capabilities|=RAF_B7_CAP_DISK;}if(g){p->gpu=*g;if(g->available&&g->dispatch){if(g->available(g->ctx,RAF_B7_BACKEND_VULKAN)>0)p->capabilities|=RAF_B7_CAP_GPU_VULKAN;if(g->available(g->ctx,RAF_B7_BACKEND_OPENCL)>0)p->capabilities|=RAF_B7_CAP_GPU_OPENCL;}}p->read_index=0;p->compute_index=1;p->write_index=2;return raf_b7_verify_layout(p);}
int raf_b7_verify_layout(const RafB7Plan*p){uintptr_t s,e;uint32_t i;if(!p||!p->region_base)return RAF_B7_EINVAL;s=(uintptr_t)p->region_base;e=s+p->region_bytes;if(s&63u)return RAF_B7_EVERIFY;for(i=0;i<3u;++i){uintptr_t b=(uintptr_t)p->bank[i].base,x=b+p->bank[i].capacity;if((b&63u)||b<s||x>e)return RAF_B7_EVERIFY;if(i&&b<(uintptr_t)p->bank[i-1u].base+p->bank[i-1u].capacity)return RAF_B7_EVERIFY;}if((uintptr_t)p->cache_base<(uintptr_t)p->bank[2].base+p->bank[2].capacity||(uintptr_t)p->cache_base+p->cache_bytes>e)return RAF_B7_EVERIFY;return 0;}
int raf_b7_pipeline_step(RafB7Plan*p){RafB7Bank*i,*c,*o;uint8_t a,b,d;int64_t x;int rc;if(!p)return RAF_B7_EINVAL;if(raf_b7_verify_layout(p))return RAF_B7_EVERIFY;i=&p->bank[p->read_index];c=&p->bank[p->compute_index];o=&p->bank[p->write_index];if(o->state==RAF_B7_BANK_WRITE){if(!p->disk.write_at)return RAF_B7_ESTATE;x=p->disk.write_at(p->disk.ctx,p->next_write_offset,o->base,o->used);if(x<0||(uint64_t)x!=o->used){rec(p,RAF_B7_STAGE_EGRESS,0,RAF_B7_EIO,o,p->next_write_offset,o->crc32c,o->crc32c);return RAF_B7_EIO;}rec(p,RAF_B7_STAGE_EGRESS,0,0,o,p->next_write_offset,o->crc32c,o->crc32c);p->next_write_offset+=o->used;o->used=o->crc32c=o->state=0;}if(c->state==RAF_B7_BANK_COMPUTE){rc=compute(p,c);if(rc)return rc;}if(!p->input_eof&&i->state==0){if(!p->disk.read_at)return RAF_B7_ESTATE;x=p->disk.read_at(p->disk.ctx,p->next_read_offset,i->base,i->capacity);if(x<0||(uint64_t)x>i->capacity)return RAF_B7_EIO;if(!x)p->input_eof=1;else{i->used=(uint32_t)x;i->crc32c=raf_b7_crc32c(0,i->base,i->used);i->state=RAF_B7_BANK_COMPUTE;rec(p,RAF_B7_STAGE_INGEST,0,0,i,p->next_read_offset,i->crc32c,i->crc32c);p->next_read_offset+=i->used;}}a=p->read_index;b=p->compute_index;d=p->write_index;p->read_index=d;p->compute_index=a;p->write_index=b;++p->epoch;return 0;}
int raf_b7_pipeline_done(const RafB7Plan*p){uint32_t i;if(!p||!p->input_eof)return 0;for(i=0;i<3u;++i)if(p->bank[i].state)return 0;return 1;}
int raf_b7_attest(RafB7Plan*p,uint32_t w,int ok){RafB7Bank b;if(!p||!w||!ok)return RAF_B7_EVERIFY;b.base=p->cache_base;b.capacity=p->cache_bytes;b.used=0;b.crc32c=w;b.state=0;b.logical_address=(uintptr_t)p->cache_base;p->attestation_crc32c=w;p->claim_allowed=1;rec(p,RAF_B7_STAGE_ATTEST,0,0,&b,0,w,w);return 0;}
const RafB7Receipt*raf_b7_last_receipt(const RafB7Plan*p){return(!p||!p->receipt_count)?0:&p->receipt[(p->receipt_head+63u)%64u];}
