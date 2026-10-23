/* RAFAELIA Field Lab Android edge: native entry uses only primitive JNI
 * ABI parameters (opaque); it does not dereference JNIEnv or access Android.
 * The L0 source remains unchanged and compiled into the same shared ELF.
 * No malloc, no headers, no libc, no syscall or shared mutable state.
 */
typedef unsigned char u8;
typedef unsigned int u32;
typedef short i16;
extern int raf_lab_sha3_256(const u8*,u32,u8*);
extern int raf_lab_pcm16_48k_square(i16*,u32,u32*);
extern int raf_lab_wav48k_header(u8*,u32,u32);
extern int raf_lab_spectrum8_dft16(const i16*,u32*);
extern int raf_lab_zip32_one(const u8*,u32,const u8*,u32,u8*,u32,u32*);
#if defined(__GNUC__)
#define RAF_EXPORT __attribute__((visibility("default")))
#else
#define RAF_EXPORT
#endif
/* Returns bits for SHA3/PCM/WAV/DFT/ZIP independent checks, 31=all pass. */
RAF_EXPORT int Java_io_rafaelia_fieldlab_FieldLabActivity_nativeSelfTest(
    void *environment, void *instance) {
  u8 hash[32],wav[44],zip[180];
  i16 pcm[16];
  u32 bins[8],phase=0,written=0;
  const u8 filename[6]={'r','a','f','.','t','x'};
  const u8 abc[3]={'a','b','c'};
  /* SHA3-256("abc") known answer, NIST FIPS 202. */
  const u8 expected[32]={
    0x3a,0x98,0x5d,0xa7,0x4f,0xe2,0x25,0xb2,
    0x04,0x5c,0x17,0x2d,0x6b,0xd3,0x90,0xbd,
    0x85,0x5f,0x08,0x6e,0x3e,0x9d,0x52,0x5b,
    0x46,0xbf,0xe2,0x45,0x11,0x43,0x15,0x32
  };
  u32 k,eq=0xffffffffu,flags=0;
  (void)environment;
  (void)instance;
  if(raf_lab_sha3_256(abc,3,hash)) {
    for(k=0;k<32u;k++)eq &= (u32)(hash[k]==expected[k]);
    flags |= (eq&1u);
  }
  if(raf_lab_pcm16_48k_square(pcm,16,&phase) &&
     phase==7040u && pcm[0]==12000)flags|=2;
  if(raf_lab_wav48k_header(wav,44,16) &&
     wav[0]=='R' && wav[1]=='I' && wav[2]=='F' && wav[3]=='F' &&
     wav[24]==0x80 && wav[25]==0xbb && wav[26]==0x00)flags|=4;
  if(raf_lab_spectrum8_dft16(pcm,bins) && bins[0]>0)flags|=8;
  if(raf_lab_zip32_one(filename,6,abc,3,zip,180,&written) &&
     written==113u && zip[0]=='P' && zip[1]=='K')flags|=16;
  return (int)flags;
}
