/* RAFAELIA Ω Field Lab L0 v1. Original source: independent arithmetic and
 * bounded container construction, references SHA-3 NIST FIPS 202 and ZIP APPNOTE.
 * Runtime: no libc, heap, OS, syscalls, JNI, malloc or device I/O.
 * Inputs and buffers belong to caller. This code does not launch an Android UI.
 */
typedef unsigned char raf_b8;
typedef unsigned int raf_u32;
typedef unsigned long long raf_u64;
typedef signed short raf_i16;
typedef signed long long raf_i64;
_Static_assert(sizeof(raf_b8)==1 && sizeof(raf_u32)==4 &&
               sizeof(raf_u64)==8 && sizeof(raf_i16)==2,"layout");

static void raf16(raf_b8 *b,raf_u32 n){b[0]=(raf_b8)n;b[1]=(raf_b8)(n>>8);}
static void raf32(raf_b8 *b,raf_u32 n){raf16(b,n);raf16(b+2,n>>16);}
static void raf4(raf_b8 *b,raf_b8 a,raf_b8 c,raf_b8 d,raf_b8 e)
{b[0]=a;b[1]=c;b[2]=d;b[3]=e;}
static raf_u64 rot(raf_u64 v,raf_u32 n){return n?((v<<n)|(v>>(64u-n))):v;}

static const raf_u64 rounds[24]={
0x0000000000000001ULL,0x0000000000008082ULL,
0x800000000000808aULL,0x8000000080008000ULL,
0x000000000000808bULL,0x0000000080000001ULL,
0x8000000080008081ULL,0x8000000000008009ULL,
0x000000000000008aULL,0x0000000000000088ULL,
0x0000000080008009ULL,0x000000008000000aULL,
0x000000008000808bULL,0x800000000000008bULL,
0x8000000000008089ULL,0x8000000000008003ULL,
0x8000000000008002ULL,0x8000000000000080ULL,
0x000000000000800aULL,0x800000008000000aULL,
0x8000000080008081ULL,0x8000000000008080ULL,
0x0000000080000001ULL,0x8000000080008008ULL
};
static const raf_b8 shifts[25]={
0,1,62,28,27,36,44,6,55,20,3,10,43,25,39,
41,45,15,21,8,18,2,61,56,14
};
static void permutation(raf_u64 a[25]){
  raf_u64 b[25],c[5],d[5];
  raf_u32 r,x,y;
  for(r=0;r<24;r++){
    for(x=0;x<5;x++)c[x]=a[x]^a[x+5]^a[x+10]^a[x+15]^a[x+20];
    for(x=0;x<5;x++)d[x]=c[(x+4)%5]^rot(c[(x+1)%5],1);
    for(y=0;y<5;y++)for(x=0;x<5;x++)a[x+5*y]^=d[x];
    for(y=0;y<5;y++)for(x=0;x<5;x++)
      b[y+5*((2*x+3*y)%5)]=rot(a[x+5*y],shifts[x+5*y]);
    for(y=0;y<5;y++)for(x=0;x<5;x++)
      a[x+5*y]=b[x+5*y]^((~b[(x+1)%5+5*y])&b[(x+2)%5+5*y]);
    a[0]^=rounds[r];
  }
}
/* NIST SHA3-256: rate 136, pad 0x06..0x80; no copied provider code. */
int raf_lab_sha3_256(const raf_b8 *input,raf_u32 len,raf_b8 out[32]){
  raf_u64 a[25]={0};
  raf_u32 i,pos=0,remain=len;
  if(!out || (len&&!input) || len>1048576u)return 0;
  while(remain>=136u){
    for(i=0;i<136u;i++)a[i>>3]^=(raf_u64)input[pos+i]<<((i&7u)*8u);
    permutation(a);pos+=136u;remain-=136u;
  }
  for(i=0;i<remain;i++)a[i>>3]^=(raf_u64)input[pos+i]<<((i&7u)*8u);
  a[remain>>3]^=(raf_u64)0x06u<<((remain&7u)*8u);
  a[16]^=0x8000000000000000ULL;
  permutation(a);
  for(i=0;i<32;i++)out[i]=(raf_b8)(a[i>>3]>>((i&7u)*8u));
  return 1;
}
/* Generates caller-owned mono PCM16 samples at exactly 48000 Hz; no sound IO. */
int raf_lab_pcm16_48k_square(raf_i16 *out,raf_u32 count,raf_u32 *phase){
  raf_u32 i,p;
  if(!out||!phase||count>48000u||*phase>=48000u)return 0;
  p=*phase;
  for(i=0;i<count;i++){
    out[i]=(raf_i16)(p<24000u?12000:-12000);
    p+=440u;
    p-=(p>=48000u)?48000u:0u;
  }
  *phase=p;
  return 1;
}
/* Encodes exactly a 44-byte RIFF/WAVE header for mono 48kHz signed PCM16. */
int raf_lab_wav48k_header(raf_b8 *out,raf_u32 cap,raf_u32 frames){
  if(!out||cap<44u||frames>1000000u)return 0;
  raf4(out,'R','I','F','F');raf32(out+4,36u+frames*2u);
  raf4(out+8,'W','A','V','E');
  raf4(out+12,'f','m','t',' ');raf32(out+16,16);raf16(out+20,1);
  raf16(out+22,1);raf32(out+24,48000);raf32(out+28,96000);
  raf16(out+32,2);raf16(out+34,16);
  raf4(out+36,'d','a','t','a');raf32(out+40,frames*2u);
  return 1;
}
/* 8-bin short-window DFT magnitude L1 from 16 signed PCM samples.
 * Spectral PROBE, not calibrated physical spectrometry or audio playback.
 */
static const raf_i16 cosine[16]={
16384,15137,11585,6270,0,-6270,-11585,-15137,
-16384,-15137,-11585,-6270,0,6270,11585,15137
};
static const raf_i16 sine[16]={
0,6270,11585,15137,16384,15137,11585,6270,
0,-6270,-11585,-15137,-16384,-15137,-11585,-6270
};
int raf_lab_spectrum8_dft16(const raf_i16 samples[16],raf_u32 out[8]){
  raf_u32 k,n;
  if(!samples||!out)return 0;
  for(k=0;k<8;k++){
    raf_i64 re=0,im=0;
    for(n=0;n<16;n++){
      raf_u32 idx=(n*k)&15u;
      re+=(raf_i64)((int)samples[n]*(int)cosine[idx]);
      im-=(raf_i64)((int)samples[n]*(int)sine[idx]);
    }
    re=(re<0)?-re:re;im=(im<0)?-im:im;
    out[k]=(raf_u32)((re+im)>>14);
  }
  return 1;
}
static raf_u32 crc32(const raf_b8 *p,raf_u32 n){
  raf_u32 c=~0u,i,j;
  for(i=0;i<n;i++){
    c^=p[i];
    for(j=0;j<8;j++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);
  }
  return ~c;
}
/* One noncompressed ZIP32 entry; strict filename bounds and no traversal. */
int raf_lab_zip32_one(const raf_b8 *name,raf_u32 name_len,
   const raf_b8 *data,raf_u32 len,raf_b8 *out,raf_u32 cap,raf_u32 *written){
  raf_u32 i,p,central,crc,final_size;
  if(!written||!out||!name||!name_len||name_len>64u||
     (!data&&len)||len>65535u)return 0;
  for(i=0;i<name_len;i++){
    raf_b8 b=name[i];
    if(!((b>='a'&&b<='z')||(b>='A'&&b<='Z')||
       (b>='0'&&b<='9')||b=='-'||b=='_'||b=='.'))return 0;
  }
  if(name[0]=='.')return 0;
  final_size=98u+2u*name_len+len;
  if(final_size>cap)return 0;
  crc=crc32(data,len);p=0;
  for(i=0;i<30;i++)out[p+i]=0;
  raf32(out+p,0x04034b50u);raf16(out+p+4,20);raf32(out+p+14,crc);
  raf32(out+p+18,len);raf32(out+p+22,len);raf16(out+p+26,name_len);
  p+=30;
  for(i=0;i<name_len;i++)out[p++]=name[i];
  for(i=0;i<len;i++)out[p++]=data[i];
  central=p;
  for(i=0;i<46;i++)out[p+i]=0;
  raf32(out+p,0x02014b50u);raf16(out+p+4,20);raf16(out+p+6,20);
  raf32(out+p+16,crc);raf32(out+p+20,len);raf32(out+p+24,len);
  raf16(out+p+28,name_len);p+=46;
  for(i=0;i<name_len;i++)out[p++]=name[i];
  for(i=0;i<22;i++)out[p+i]=0;
  raf32(out+p,0x06054b50u);raf16(out+p+8,1);raf16(out+p+10,1);
  raf32(out+p+12,p-central);raf32(out+p+16,central);p+=22;
  *written=p;
  return 1;
}
