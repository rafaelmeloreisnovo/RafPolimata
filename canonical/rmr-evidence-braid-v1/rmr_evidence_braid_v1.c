/*
 * RMR Evidence Braid V1
 * Governance anchor: CLOSURE_L1 (provenance/reproducibility gaps).
 * Freestanding/no-libc/no-heap/no-syscall core.
 *
 * Scope:
 *  - streaming SHA-256 + CRC32C over caller-fed bytes;
 *  - 10 typed evidence channels with event-preserving hash chains;
 *  - generation root bound to predecessor, sequence, channel presence and roots;
 *  - challenge derived from a pre-root, then bindable into the final root.
 *
 * Boundary:
 *  HASH != TRUTH; TIME_SOURCE != IDENTITY; CONTEXT != CUSTODY.
 *  Missing channel = unset mask bit, never an all-zero digest interpreted as evidence.
 */

typedef unsigned char rmr_u8;
typedef unsigned int rmr_u32;
typedef unsigned long long rmr_u64;
typedef unsigned long rmr_size;

#define RMR_EVB1_CHANNELS 10u
#define RMR_EVB1_DIGEST_BYTES 32u

#define RMR_EVB1_CH_CONTENT 0u
#define RMR_EVB1_CH_BUILD   1u
#define RMR_EVB1_CH_TIME    2u
#define RMR_EVB1_CH_NETWORK 3u
#define RMR_EVB1_CH_PUBLIC  4u
#define RMR_EVB1_CH_CHAIN   5u
#define RMR_EVB1_CH_DEVICE  6u
#define RMR_EVB1_CH_RADIO   7u
#define RMR_EVB1_CH_ENV     8u
#define RMR_EVB1_CH_HUMAN   9u

typedef struct {
    rmr_u32 h[8];
    rmr_u64 bit_len;
    rmr_u8 block[64];
    rmr_u32 used;
} RmrSha256;

typedef struct {
    RmrSha256 sha256;
    rmr_u32 crc32c;
    rmr_u64 total_bytes;
} RmrDigestStream;

typedef struct {
    rmr_u8 previous_root[32];
    rmr_u8 channel_root[RMR_EVB1_CHANNELS][32];
    rmr_u32 channel_mask;
    rmr_u64 sequence;
} RmrEvidenceBraid;

static rmr_u32 rmr_ror32(rmr_u32 x, rmr_u32 n) { return (x >> n) | (x << (32u - n)); }

static const rmr_u32 rmr_sha256_k[64] = {
0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};

static void rmr_memzero(void *p, rmr_size n) { rmr_u8 *d=(rmr_u8*)p; while(n--) *d++=0u; }
static void rmr_memcpy(void *dst,const void *src,rmr_size n){rmr_u8*d=(rmr_u8*)dst;const rmr_u8*s=(const rmr_u8*)src;while(n--)*d++=*s++;}

static void rmr_sha256_init(RmrSha256 *s){
    s->h[0]=0x6a09e667u;s->h[1]=0xbb67ae85u;s->h[2]=0x3c6ef372u;s->h[3]=0xa54ff53au;
    s->h[4]=0x510e527fu;s->h[5]=0x9b05688cu;s->h[6]=0x1f83d9abu;s->h[7]=0x5be0cd19u;
    s->bit_len=0u;s->used=0u;
}

static void rmr_sha256_block(RmrSha256*s,const rmr_u8*p){
    rmr_u32 w[64],a,b,c,d,e,f,g,h,t1,t2,i,x,y;
    for(i=0;i<16u;i++)w[i]=((rmr_u32)p[i*4u]<<24)|((rmr_u32)p[i*4u+1u]<<16)|((rmr_u32)p[i*4u+2u]<<8)|(rmr_u32)p[i*4u+3u];
    for(i=16u;i<64u;i++){x=w[i-15u];y=w[i-2u];w[i]=w[i-16u]+(rmr_ror32(x,7u)^rmr_ror32(x,18u)^(x>>3u))+w[i-7u]+(rmr_ror32(y,17u)^rmr_ror32(y,19u)^(y>>10u));}
    a=s->h[0];b=s->h[1];c=s->h[2];d=s->h[3];e=s->h[4];f=s->h[5];g=s->h[6];h=s->h[7];
    for(i=0;i<64u;i++){
        t1=h+(rmr_ror32(e,6u)^rmr_ror32(e,11u)^rmr_ror32(e,25u))+((e&f)^((~e)&g))+rmr_sha256_k[i]+w[i];
        t2=(rmr_ror32(a,2u)^rmr_ror32(a,13u)^rmr_ror32(a,22u))+((a&b)^(a&c)^(b&c));
        h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
    s->h[0]+=a;s->h[1]+=b;s->h[2]+=c;s->h[3]+=d;s->h[4]+=e;s->h[5]+=f;s->h[6]+=g;s->h[7]+=h;
}

static void rmr_sha256_update(RmrSha256*s,const void*data,rmr_size n){
    const rmr_u8*p=(const rmr_u8*)data;rmr_size i;
    for(i=0;i<n;i++){s->block[s->used++]=p[i];s->bit_len+=8u;if(s->used==64u){rmr_sha256_block(s,s->block);s->used=0u;}}
}

static void rmr_sha256_final(RmrSha256*s,rmr_u8 out[32]){
    rmr_u64 bits=s->bit_len;rmr_u32 i;
    s->block[s->used++]=0x80u;
    if(s->used>56u){while(s->used<64u)s->block[s->used++]=0u;rmr_sha256_block(s,s->block);s->used=0u;}
    while(s->used<56u)s->block[s->used++]=0u;
    for(i=0;i<8u;i++)s->block[63u-i]=(rmr_u8)(bits>>(i*8u));
    rmr_sha256_block(s,s->block);
    for(i=0;i<8u;i++){out[i*4u]=(rmr_u8)(s->h[i]>>24);out[i*4u+1u]=(rmr_u8)(s->h[i]>>16);out[i*4u+2u]=(rmr_u8)(s->h[i]>>8);out[i*4u+3u]=(rmr_u8)s->h[i];}
}

static rmr_u32 rmr_crc32c_update(rmr_u32 crc,const void*data,rmr_size n){
    const rmr_u8*p=(const rmr_u8*)data;rmr_size i;rmr_u32 j,x,mask;
    for(i=0;i<n;i++){x=crc^(rmr_u32)p[i];for(j=0;j<8u;j++){mask=(rmr_u32)(0u-(x&1u));x=(x>>1)^(0x82f63b78u&mask);}crc=x;}
    return crc;
}

void rmr_digest_stream_init(RmrDigestStream*s){rmr_sha256_init(&s->sha256);s->crc32c=0xffffffffu;s->total_bytes=0u;}
void rmr_digest_stream_update(RmrDigestStream*s,const void*data,rmr_size n){rmr_sha256_update(&s->sha256,data,n);s->crc32c=rmr_crc32c_update(s->crc32c,data,n);s->total_bytes+=(rmr_u64)n;}
void rmr_digest_stream_final(RmrDigestStream*s,rmr_u8 sha256_out[32],rmr_u32*crc32c_out){rmr_sha256_final(&s->sha256,sha256_out);if(crc32c_out)*crc32c_out=s->crc32c^0xffffffffu;}

static void rmr_u32be(rmr_u32 v,rmr_u8 out[4]){out[0]=(rmr_u8)(v>>24);out[1]=(rmr_u8)(v>>16);out[2]=(rmr_u8)(v>>8);out[3]=(rmr_u8)v;}
static void rmr_u64be(rmr_u64 v,rmr_u8 out[8]){rmr_u32 i;for(i=0;i<8u;i++)out[7u-i]=(rmr_u8)(v>>(i*8u));}

void rmr_evidence_braid_init(RmrEvidenceBraid*s,const rmr_u8 previous_root[32],rmr_u64 sequence){
    rmr_memzero(s,(rmr_size)sizeof(*s));if(previous_root)rmr_memcpy(s->previous_root,previous_root,32u);s->sequence=sequence;
}

int rmr_evidence_braid_absorb(RmrEvidenceBraid*s,rmr_u32 channel,const void*event,rmr_size event_len){
    static const rmr_u8 domain[]={'R','M','R','-','E','V','B','1','-','C','H','A','N'};
    RmrSha256 h;rmr_u8 next[32],idx[4],lenbe[8];
    if(channel>=RMR_EVB1_CHANNELS || (!event && event_len))return -1;
    rmr_u32be(channel,idx);rmr_u64be((rmr_u64)event_len,lenbe);
    rmr_sha256_init(&h);rmr_sha256_update(&h,domain,sizeof(domain));rmr_sha256_update(&h,idx,4u);
    rmr_sha256_update(&h,s->channel_root[channel],32u);rmr_sha256_update(&h,lenbe,8u);if(event_len)rmr_sha256_update(&h,event,event_len);rmr_sha256_final(&h,next);
    rmr_memcpy(s->channel_root[channel],next,32u);s->channel_mask|=(1u<<channel);return 0;
}

void rmr_evidence_braid_root(const RmrEvidenceBraid*s,const rmr_u8 challenge_or_null[32],rmr_u8 out[32]){
    static const rmr_u8 domain[]={'R','M','R','-','E','V','B','1','-','R','O','O','T'};
    RmrSha256 h;rmr_u8 seq[8],mask[4],i4[4],present;rmr_u32 i;
    rmr_u64be(s->sequence,seq);rmr_u32be(s->channel_mask,mask);
    rmr_sha256_init(&h);rmr_sha256_update(&h,domain,sizeof(domain));rmr_sha256_update(&h,s->previous_root,32u);rmr_sha256_update(&h,seq,8u);rmr_sha256_update(&h,mask,4u);
    for(i=0;i<RMR_EVB1_CHANNELS;i++){rmr_u32be(i,i4);present=(rmr_u8)((s->channel_mask>>i)&1u);rmr_sha256_update(&h,i4,4u);rmr_sha256_update(&h,&present,1u);rmr_sha256_update(&h,s->channel_root[i],32u);}
    present=(rmr_u8)(challenge_or_null?1u:0u);rmr_sha256_update(&h,&present,1u);if(challenge_or_null)rmr_sha256_update(&h,challenge_or_null,32u);rmr_sha256_final(&h,out);
}

void rmr_evidence_braid_challenge(const RmrEvidenceBraid*s,const void*nonce,rmr_size nonce_len,rmr_u8 out[32]){
    static const rmr_u8 domain[]={'R','M','R','-','E','V','B','1','-','C','H','A','L'};
    RmrSha256 h;rmr_u8 pre[32],lenbe[8];rmr_evidence_braid_root(s,(const rmr_u8*)0,pre);rmr_u64be((rmr_u64)nonce_len,lenbe);
    rmr_sha256_init(&h);rmr_sha256_update(&h,domain,sizeof(domain));rmr_sha256_update(&h,pre,32u);rmr_sha256_update(&h,lenbe,8u);if(nonce_len)rmr_sha256_update(&h,nonce,nonce_len);rmr_sha256_final(&h,out);
}

#ifdef RMR_EVB1_SELFTEST
#include <stdio.h>
static int rmr_memeq(const void*a,const void*b,rmr_size n){const rmr_u8*x=(const rmr_u8*)a,*y=(const rmr_u8*)b;rmr_u8 d=0;while(n--)d|=(rmr_u8)(*x++^*y++);return d==0u;}
static int hex32(const rmr_u8*d,const char*h){static const char*x="0123456789abcdef";rmr_u32 i;for(i=0;i<32u;i++)if(x[d[i]>>4]!=h[i*2u]||x[d[i]&15u]!=h[i*2u+1u])return 0;return 1;}
static int neq32(const rmr_u8*a,const rmr_u8*b){return !rmr_memeq(a,b,32u);}
int main(void){
    int fail=0;rmr_u8 d[32],r1[32],r2[32],ch[32],prev[32];rmr_u32 crc;RmrDigestStream ds;RmrEvidenceBraid a,b;
    rmr_digest_stream_init(&ds);rmr_digest_stream_update(&ds,"abc",3u);rmr_digest_stream_final(&ds,d,&crc);
    if(!hex32(d,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")){puts("FAIL sha256 abc");fail++;}
    if(crc!=0x364b3fb7u){printf("FAIL crc32c abc got=%08x\n",crc);fail++;}
    rmr_memzero(prev,32u);rmr_evidence_braid_init(&a,prev,1u);rmr_evidence_braid_absorb(&a,RMR_EVB1_CH_CONTENT,"same",4u);rmr_evidence_braid_root(&a,0,r1);
    rmr_evidence_braid_init(&b,prev,1u);rmr_evidence_braid_absorb(&b,RMR_EVB1_CH_BUILD,"same",4u);rmr_evidence_braid_root(&b,0,r2);if(!neq32(r1,r2)){puts("FAIL channel separation");fail++;}
    prev[0]=1u;rmr_evidence_braid_init(&b,prev,1u);rmr_evidence_braid_absorb(&b,RMR_EVB1_CH_CONTENT,"same",4u);rmr_evidence_braid_root(&b,0,r2);if(!neq32(r1,r2)){puts("FAIL predecessor binding");fail++;}
    rmr_memzero(prev,32u);rmr_evidence_braid_init(&b,prev,1u);rmr_evidence_braid_absorb(&b,RMR_EVB1_CH_CONTENT,"same",4u);rmr_evidence_braid_challenge(&b,"nonce",5u,ch);rmr_evidence_braid_root(&b,ch,r2);if(!neq32(r1,r2)){puts("FAIL challenge binding");fail++;}
    if((b.channel_mask&(1u<<RMR_EVB1_CH_TIME))!=0u){puts("FAIL TOKEN_VAZIO channel mask");fail++;}
    if(fail){printf("RMR_EVIDENCE_BRAID_V1 FAIL=%d\n",fail);return 1;}puts("RMR_EVIDENCE_BRAID_V1 PASS");return 0;
}
#endif
